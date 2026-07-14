#include "samples/web_transport_echo_visitor.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "quiche/common/quiche_stream.h"
#include "quiche/web_transport/complete_buffer_visitor.h"

namespace google_quiche::samples {
namespace {

class BidirectionalEchoVisitor : public webtransport::StreamVisitor {
 public:
  explicit BidirectionalEchoVisitor(webtransport::Stream* stream)
      : stream_(stream) {}

  void OnCanRead() override {
    const webtransport::Stream::ReadResult result = stream_->Read(&buffer_);
    if (result.bytes_read > 0) {
      std::cout << "[echo_receive] side=server transport=stream stream_id="
                << stream_->GetStreamId() << " length=" << result.bytes_read
                << " payload=" << FormatEchoPayload(buffer_) << "\n";
    }
    if (result.fin) {
      send_fin_ = true;
    }
    OnCanWrite();
  }

  void OnCanWrite() override {
    if (stop_sending_received_) {
      return;
    }
    if (!buffer_.empty()) {
      const std::string data = buffer_;
      const absl::Status status = quiche::WriteIntoStream(*stream_, data);
      if (!status.ok()) {
        return;
      }
      std::cout << "[echo_send] side=server transport=stream stream_id="
                << stream_->GetStreamId() << " length=" << data.size()
                << " payload=" << FormatEchoPayload(data) << "\n";
      buffer_.clear();
    }
    if (send_fin_ && !fin_sent_ &&
        quiche::SendFinOnStream(*stream_).ok()) {
      fin_sent_ = true;
    }
  }

  void OnResetStreamReceived(webtransport::StreamErrorCode) override {
    send_fin_ = true;
    OnCanWrite();
  }
  void OnStopSendingReceived(webtransport::StreamErrorCode) override {
    stop_sending_received_ = true;
  }
  void OnWriteSideInDataRecvdState() override {}

 private:
  webtransport::Stream* stream_;
  std::string buffer_;
  bool send_fin_ = false;
  bool fin_sent_ = false;
  bool stop_sending_received_ = false;
};

}  // namespace

std::string FormatEchoPayload(absl::string_view payload, size_t maximum_bytes) {
  std::ostringstream output;
  output << '"';
  const size_t display_size = std::min(payload.size(), maximum_bytes);
  for (size_t i = 0; i < display_size; ++i) {
    const unsigned char value = static_cast<unsigned char>(payload[i]);
    switch (value) {
      case '\\': output << "\\\\"; break;
      case '"': output << "\\\""; break;
      case '\r': output << "\\r"; break;
      case '\n': output << "\\n"; break;
      case '\t': output << "\\t"; break;
      default:
        if (std::isprint(value)) {
          output << static_cast<char>(value);
        } else {
          output << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                 << static_cast<int>(value) << std::dec;
        }
    }
  }
  output << '"';
  if (display_size != payload.size()) {
    output << " truncated=true";
  }
  return output.str();
}

WebTransportEchoVisitor::WebTransportEchoVisitor(
    webtransport::Session* session, bool open_server_initiated_echo_stream)
    : session_(session),
      echo_stream_opened_(!open_server_initiated_echo_stream) {}

void WebTransportEchoVisitor::OnSessionReady() {
  if (session_->CanOpenNextOutgoingBidirectionalStream()) {
    OnCanCreateNewOutgoingBidirectionalStream();
  }
}

void WebTransportEchoVisitor::OnSessionClosed(
    webtransport::SessionErrorCode, const std::string&) {}

void WebTransportEchoVisitor::OnIncomingBidirectionalStreamAvailable() {
  while (webtransport::Stream* stream =
             session_->AcceptIncomingBidirectionalStream()) {
    stream->SetVisitor(std::make_unique<BidirectionalEchoVisitor>(stream));
    stream->visitor()->OnCanRead();
  }
}

void WebTransportEchoVisitor::OnIncomingUnidirectionalStreamAvailable() {
  while (webtransport::Stream* stream =
             session_->AcceptIncomingUnidirectionalStream()) {
    const webtransport::StreamId stream_id = stream->GetStreamId();
    stream->SetVisitor(std::make_unique<webtransport::CompleteBufferVisitor>(
        stream, [this, stream_id](std::string data) {
          std::cout << "[echo_receive] side=server transport=stream stream_id="
                    << stream_id << " length=" << data.size()
                    << " payload=" << FormatEchoPayload(data) << "\n";
          streams_to_echo_back_.push_back(std::move(data));
          TrySendingUnidirectionalStreams();
        }));
    stream->visitor()->OnCanRead();
  }
}

void WebTransportEchoVisitor::OnDatagramReceived(absl::string_view datagram) {
  std::cout << "[echo_receive] side=server transport=datagram length="
            << datagram.size() << " payload=" << FormatEchoPayload(datagram)
            << "\n";
  session_->SendOrQueueDatagram(datagram);
  std::cout << "[echo_send] side=server transport=datagram length="
            << datagram.size() << " payload=" << FormatEchoPayload(datagram)
            << "\n";
}

void WebTransportEchoVisitor::OnCanCreateNewOutgoingBidirectionalStream() {
  if (!echo_stream_opened_) {
    webtransport::Stream* stream = session_->OpenOutgoingBidirectionalStream();
    if (stream != nullptr) {
      stream->SetVisitor(std::make_unique<BidirectionalEchoVisitor>(stream));
      echo_stream_opened_ = true;
    }
  }
}

void WebTransportEchoVisitor::OnCanCreateNewOutgoingUnidirectionalStream() {
  TrySendingUnidirectionalStreams();
}

void WebTransportEchoVisitor::TrySendingUnidirectionalStreams() {
  while (!streams_to_echo_back_.empty() &&
         session_->CanOpenNextOutgoingUnidirectionalStream()) {
    webtransport::Stream* stream = session_->OpenOutgoingUnidirectionalStream();
    if (stream == nullptr) {
      return;
    }
    std::string data = std::move(streams_to_echo_back_.front());
    streams_to_echo_back_.pop_front();
    std::cout << "[echo_send] side=server transport=stream stream_id="
              << stream->GetStreamId() << " length=" << data.size()
              << " payload=" << FormatEchoPayload(data) << "\n";
    stream->SetVisitor(std::make_unique<webtransport::CompleteBufferVisitor>(
        stream, std::move(data)));
    stream->visitor()->OnCanWrite();
  }
}

}  // namespace google_quiche::samples
