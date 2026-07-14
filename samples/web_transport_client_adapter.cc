#include "samples/web_transport_client_adapter.h"

#include <memory>
#include <utility>

#include "openssl/ssl.h"
#include "samples/web_transport_client_session.h"

namespace google_quiche::samples {

WebTransportClientAdapter::WebTransportClientAdapter(
    quic::QuicSocketAddress server_address,
    const quic::QuicServerId& server_id,
    const quic::ParsedQuicVersionVector& supported_versions,
    quic::QuicEventLoop* event_loop,
    std::unique_ptr<quic::ProofVerifier> proof_verifier,
    WebTransportCryptoConfig demo_crypto_config)
    : quic::QuicDefaultClient(server_address, server_id, supported_versions,
                              event_loop, std::move(proof_verifier)),
      demo_crypto_config_(demo_crypto_config) {
  if (demo_crypto_config_.use_null_one_rtt_crypter) {
    SSL_CTX_set_early_data_enabled(crypto_config()->ssl_ctx(), 0);
  }
}

std::unique_ptr<quic::QuicSession>
WebTransportClientAdapter::CreateQuicClientSession(
    const quic::ParsedQuicVersionVector& supported_versions,
    quic::QuicConnection* connection) {
  return std::make_unique<WebTransportClientSession>(
      *config(), supported_versions, connection, network_helper(), server_id(),
      crypto_config(), drop_response_body(), enable_web_transport(),
      demo_crypto_config_);
}

}  // namespace google_quiche::samples
