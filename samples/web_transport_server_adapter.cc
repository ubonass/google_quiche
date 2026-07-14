#include "samples/web_transport_server_adapter.h"

#include <memory>
#include <utility>

#include "openssl/ssl.h"
#include "quiche/quic/core/quic_default_connection_helper.h"
#include "quiche/quic/tools/quic_simple_crypto_server_stream_helper.h"
#include "samples/web_transport_dispatcher.h"

namespace google_quiche::samples {

WebTransportServerAdapter::WebTransportServerAdapter(
    std::unique_ptr<quic::ProofSource> proof_source,
    quic::QuicSimpleServerBackend* backend,
    WebTransportCryptoConfig demo_crypto_config)
    : quic::QuicServer(std::move(proof_source), backend),
      demo_crypto_config_(demo_crypto_config) {
  if (demo_crypto_config_.use_null_one_rtt_crypter) {
    SSL_CTX_set_early_data_enabled(crypto_config().ssl_ctx(), 0);
  }
}

quic::QuicDispatcher* WebTransportServerAdapter::CreateQuicDispatcher() {
  return new WebTransportDispatcher(
      &config(), &crypto_config(), version_manager(),
      std::make_unique<quic::QuicDefaultConnectionHelper>(),
      std::make_unique<quic::QuicSimpleCryptoServerStreamHelper>(),
      event_loop()->CreateAlarmFactory(), server_backend(),
      expected_server_connection_id_length(), connection_id_generator(),
      demo_crypto_config_);
}

}  // namespace google_quiche::samples
