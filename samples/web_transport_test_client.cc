// Copyright 2026 The google_quiche Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "samples/network_initializer.h"

#include "quiche/common/http/http_header_block.h"
#include "quiche/common/platform/api/quiche_command_line_flags.h"
#include "quiche/common/platform/api/quiche_default_proof_providers.h"
#include "quiche/common/platform/api/quiche_logging.h"
#include "quiche/quic/core/crypto/proof_verifier.h"
#include "quiche/quic/core/http/quic_spdy_client_stream.h"
#include "quiche/quic/core/http/web_transport_http3.h"
#include "quiche/quic/core/io/quic_default_event_loop.h"
#include "quiche/quic/core/io/quic_event_loop.h"
#include "quiche/quic/core/quic_default_clock.h"
#include "quiche/quic/core/quic_server_id.h"
#include "quiche/quic/core/quic_versions.h"
#include "quiche/quic/tools/fake_proof_verifier.h"
#include "quiche/quic/tools/quic_default_client.h"
#include "quiche/quic/tools/quic_event_loop_tools.h"
#include "quiche/quic/tools/quic_name_lookup.h"
#include "quiche/quic/tools/quic_url.h"
#include "quiche/web_transport/complete_buffer_visitor.h"
#include "quiche/web_transport/web_transport.h"
#include "samples/web_transport_client_adapter.h"
#include "samples/web_transport_crypto_config.h"
#include "samples/web_transport_echo_visitor.h"

DEFINE_QUICHE_COMMAND_LINE_FLAG(
    bool,
    disable_certificate_verification,
    true,
    "Accept any server certificate. Intended only for local testing.");
DEFINE_QUICHE_COMMAND_LINE_FLAG(
    int32_t,
    timeout_ms,
    5000,
    "Maximum time to wait for each connection or echo operation.");
DEFINE_QUICHE_COMMAND_LINE_FLAG(int32_t,
                                interval_ms,
                                1000,
                                "Delay between consecutive echo operations.");
DEFINE_QUICHE_COMMAND_LINE_FLAG(int32_t,
                                count,
                                10,
                                "Number of echo operations to perform.");
DEFINE_QUICHE_COMMAND_LINE_FLAG(
    bool,
    null_one_rtt_crypter,
    false,
    "Use NullEncrypter/NullDecrypter for 1-RTT packets. Both endpoints must "
    "enable this unsafe local-debug option.");

namespace {

class DiscardStreamVisitor : public webtransport::StreamVisitor {
 public:
  explicit DiscardStreamVisitor(webtransport::Stream* stream)
      : stream_(stream) {}

  void OnCanRead() override {
    std::string data;
    while (true) {
      webtransport::Stream::ReadResult result = stream_->Read(&data);
      if (result.fin || result.bytes_read == 0) {
        return;
      }
      data.clear();
    }
  }
  void OnCanWrite() override {}
  void OnResetStreamReceived(webtransport::StreamErrorCode) override {}
  void OnStopSendingReceived(webtransport::StreamErrorCode) override {}
  void OnWriteSideInDataRecvdState() override {}

 private:
  webtransport::Stream* stream_;
};

class EchoClientVisitor : public webtransport::SessionVisitor {
 public:
  struct State {
    std::string error;
    bool ready = false;
    bool failed = false;
  };

  EchoClientVisitor(webtransport::Session* session,
                    std::shared_ptr<State> state)
      : session_(session), state_(std::move(state)) {}

  void OnSessionReady() override { state_->ready = true; }

  void OnSessionClosed(webtransport::SessionErrorCode error_code,
                       const std::string& error_message) override {
    if (!state_->failed) {
      state_->failed = true;
      state_->error = "session closed with error " +
                      std::to_string(error_code) + ": " + error_message;
    }
  }

  void OnIncomingBidirectionalStreamAvailable() override {
    AcceptAndDiscard(/*bidirectional=*/true);
  }

  void OnIncomingUnidirectionalStreamAvailable() override {
    AcceptAndDiscard(/*bidirectional=*/false);
  }

  void OnDatagramReceived(absl::string_view) override {}

  void OnCanCreateNewOutgoingBidirectionalStream() override {}
  void OnCanCreateNewOutgoingUnidirectionalStream() override {}

 private:
  void AcceptAndDiscard(bool bidirectional) {
    while (true) {
      webtransport::Stream* stream =
          bidirectional ? session_->AcceptIncomingBidirectionalStream()
                        : session_->AcceptIncomingUnidirectionalStream();
      if (stream == nullptr) {
        return;
      }
      stream->SetVisitor(std::make_unique<DiscardStreamVisitor>(stream));
      stream->visitor()->OnCanRead();
    }
  }

  webtransport::Session* session_;
  std::shared_ptr<State> state_;
};

struct EchoOperation {
  std::string response;
  bool complete = false;
};

void RunEventLoopFor(quic::QuicEventLoop* event_loop,
                     quic::QuicTimeDelta duration) {
  const quic::QuicClock* clock = event_loop->GetClock();
  const quic::QuicTime deadline = clock->Now() + duration;
  while (clock->Now() < deadline) {
    event_loop->RunEventLoopOnce(deadline - clock->Now());
  }
}

int RunClient(const quic::QuicUrl& url, const std::string& message) {
  quic::QuicServerId server_id(url.host(), url.port());
  quic::QuicSocketAddress peer_address = quic::tools::LookupAddress(server_id);
  if (!peer_address.IsInitialized()) {
    std::cerr << "Failed to resolve " << server_id.host() << "\n";
    return 1;
  }

  std::unique_ptr<quic::ProofVerifier> proof_verifier;
  if (quiche::GetQuicheCommandLineFlag(
          FLAGS_disable_certificate_verification)) {
    proof_verifier = std::make_unique<quic::FakeProofVerifier>();
  } else {
    proof_verifier = quiche::CreateDefaultProofVerifier(server_id.host());
    if (proof_verifier == nullptr) {
      std::cerr << "The default certificate verifier is unavailable; use "
                   "--disable_certificate_verification=true for local tests\n";
      return 1;
    }
  }

  quic::QuicDefaultClock* clock = quic::QuicDefaultClock::Get();
  std::unique_ptr<quic::QuicEventLoop> event_loop =
      quic::GetDefaultEventLoop()->Create(clock);
  google_quiche::samples::WebTransportCryptoConfig crypto_config;
  crypto_config.use_null_one_rtt_crypter =
      quiche::GetQuicheCommandLineFlag(FLAGS_null_one_rtt_crypter);
  if (crypto_config.use_null_one_rtt_crypter) {
    std::cerr << "WARNING: 1-RTT Null Crypter is enabled. Application data "
                 "is not confidential. 0-RTT is disabled.\n";
  }
  google_quiche::samples::WebTransportClientAdapter client(
      peer_address, server_id, quic::AllSupportedVersions(), event_loop.get(),
      std::move(proof_verifier), crypto_config);
  client.set_enable_web_transport(true);

  if (!client.Initialize()) {
    std::cerr << "Failed to initialize the QUIC client\n";
    return 1;
  }
  if (!client.Connect()) {
    std::cerr << "Failed to establish the QUIC connection: "
              << quic::QuicErrorCodeToString(client.session()->error()) << " "
              << client.session()->error_details() << "\n";
    return 1;
  }

  const quic::QuicTimeDelta timeout = quic::QuicTimeDelta::FromMilliseconds(
      quiche::GetQuicheCommandLineFlag(FLAGS_timeout_ms));
  if (!quic::ProcessEventsUntil(
          event_loop.get(),
          [&] { return client.client_session()->settings_received(); },
          timeout)) {
    std::cerr << "Timed out waiting for HTTP/3 SETTINGS\n";
    return 1;
  }
  if (!client.client_session()->SupportsWebTransport()) {
    std::cerr << "The server did not negotiate WebTransport\n";
    return 1;
  }

  auto* connect_stream = static_cast<quic::QuicSpdyClientStream*>(
      client.client_session()->CreateOutgoingBidirectionalStream());
  if (connect_stream == nullptr) {
    std::cerr << "Failed to create the WebTransport CONNECT stream\n";
    return 1;
  }

  quiche::HttpHeaderBlock headers;
  headers[":scheme"] = "https";
  headers[":authority"] = url.HostPort();
  headers[":path"] = url.PathParamsQuery();
  headers[":method"] = "CONNECT";
  headers[":protocol"] = "webtransport";
  client.set_store_response(true);
  connect_stream->SendRequest(std::move(headers), "", false);

  quic::WebTransportHttp3* session = connect_stream->web_transport();
  if (session == nullptr) {
    std::cerr << "Failed to initialize the WebTransport session\n";
    return 1;
  }

  auto state = std::make_shared<EchoClientVisitor::State>();
  auto visitor = std::make_unique<EchoClientVisitor>(session, state);
  session->SetVisitor(std::move(visitor));

  if (!quic::ProcessEventsUntil(
          event_loop.get(), [&] { return state->ready || state->failed; },
          timeout)) {
    std::cerr << "Timed out waiting for the WebTransport session\n";
    return 1;
  }
  if (state->failed) {
    std::cerr << "WebTransport session failed: " << state->error << "\n";
    return 1;
  }

  const int32_t count = quiche::GetQuicheCommandLineFlag(FLAGS_count);
  const quic::QuicTimeDelta interval = quic::QuicTimeDelta::FromMilliseconds(
      quiche::GetQuicheCommandLineFlag(FLAGS_interval_ms));
  for (int32_t i = 0; i < count; ++i) {
    if (!quic::ProcessEventsUntil(
            event_loop.get(),
            [&] {
              return state->failed ||
                     session->CanOpenNextOutgoingBidirectionalStream();
            },
            timeout) ||
        state->failed) {
      std::cerr << "Unable to open echo stream: " << state->error << "\n";
      return 1;
    }

    webtransport::Stream* stream = session->OpenOutgoingBidirectionalStream();
    if (stream == nullptr) {
      std::cerr << "Failed to create echo stream\n";
      return 1;
    }
    auto operation = std::make_shared<EchoOperation>();
    const webtransport::StreamId stream_id = stream->GetStreamId();
    const quic::QuicTime start_time = clock->Now();
    stream->SetVisitor(std::make_unique<webtransport::CompleteBufferVisitor>(
        stream, message, [operation, stream_id, i, start_time, clock](
                             std::string response) {
          const double rtt_ms =
              (clock->Now() - start_time).ToMicroseconds() / 1000.0;
          std::cout << "[echo_receive] side=client index=" << (i + 1)
                    << " stream_id=" << stream_id
                    << " length=" << response.size() << " payload="
                    << google_quiche::samples::FormatEchoPayload(response)
                    << " rtt_ms=" << rtt_ms << "\n";
          operation->response = std::move(response);
          operation->complete = true;
        }));
    stream->visitor()->OnCanWrite();
    std::cout << "[echo_send] side=client index=" << (i + 1)
              << " stream_id=" << stream_id << " length=" << message.size()
              << " payload="
              << google_quiche::samples::FormatEchoPayload(message) << "\n";

    if (!quic::ProcessEventsUntil(
            event_loop.get(),
            [&] { return operation->complete || state->failed; }, timeout)) {
      std::cerr << "Timed out waiting for echo " << (i + 1) << "\n";
      return 1;
    }
    if (state->failed || operation->response != message) {
      std::cerr << "[echo_verify] side=client index=" << (i + 1)
                << " stream_id=" << stream_id << " verify=failed expected="
                << google_quiche::samples::FormatEchoPayload(message)
                << " actual=" << google_quiche::samples::FormatEchoPayload(
                                      operation->response)
                << "\nEcho " << (i + 1) << " failed"
                << (state->error.empty() ? "" : ": " + state->error) << "\n";
      return 1;
    }

    std::cout << "[echo_verify] side=client index=" << (i + 1)
              << " stream_id=" << stream_id << " verify=ok\n"
              << "Echo " << (i + 1) << "/" << count
              << " succeeded: " << operation->response << "\n";
    if (i + 1 < count && interval > quic::QuicTimeDelta::Zero()) {
      RunEventLoopFor(event_loop.get(), interval);
      if (state->failed) {
        std::cerr << "WebTransport session failed: " << state->error << "\n";
        return 1;
      }
    }
  }

  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  google_quiche::samples::NetworkInitializer network;
  if (!network.ok()) {
    std::cerr << "Failed to initialize networking, error "
              << network.error_code() << "\n";
    return 1;
  }

  const char* usage =
      "Usage: web_transport_test_client [options] <url> <message>";
  std::vector<std::string> args =
      quiche::QuicheParseCommandLineFlags(usage, argc, argv);
  if (args.size() != 2) {
    quiche::QuichePrintCommandLineFlagHelp(usage);
    return 1;
  }

  if (quiche::GetQuicheCommandLineFlag(FLAGS_count) <= 0 ||
      quiche::GetQuicheCommandLineFlag(FLAGS_interval_ms) < 0 ||
      quiche::GetQuicheCommandLineFlag(FLAGS_timeout_ms) <= 0) {
    std::cerr << "count and timeout_ms must be positive; interval_ms must be "
                 "non-negative\n";
    return 1;
  }

  quic::QuicUrl url(args[0], "https");
  if (!url.IsValid() || url.scheme() != "https" || url.host().empty() ||
      url.port() == 0) {
    std::cerr << "Invalid WebTransport URL: " << args[0] << "\n";
    return 1;
  }
  if (url.path() != "/webtransport/echo") {
    std::cerr << "This client currently supports only /webtransport/echo\n";
    return 1;
  }

  return RunClient(url, args[1]);
}
