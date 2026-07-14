#include "samples/web_transport_client_session.h"

#include <memory>
#include <utility>

#include "samples/web_transport_null_decrypter.h"
#include "samples/web_transport_null_encrypter.h"

namespace google_quiche::samples {

WebTransportClientSession::WebTransportClientSession(
    const quic::QuicConfig& config,
    const quic::ParsedQuicVersionVector& supported_versions,
    quic::QuicConnection* connection,
    quic::QuicClientBase::NetworkHelper* network_helper,
    const quic::QuicServerId& server_id,
    quic::QuicCryptoClientConfig* crypto_config, bool drop_response_body,
    bool enable_web_transport, WebTransportCryptoConfig demo_crypto_config)
    : quic::QuicSimpleClientSession(
          config, supported_versions, connection, network_helper, server_id,
          crypto_config, drop_response_body, enable_web_transport),
      demo_crypto_config_(demo_crypto_config) {}

void WebTransportClientSession::OnNewEncryptionKeyAvailable(
    quic::EncryptionLevel level,
    std::unique_ptr<quic::QuicEncrypter> encrypter) {
  if (demo_crypto_config_.use_null_one_rtt_crypter &&
      level == quic::ENCRYPTION_FORWARD_SECURE) {
    encrypter = std::make_unique<WebTransportNullEncrypter>(
        quic::Perspective::IS_CLIENT);
  }
  quic::QuicSimpleClientSession::OnNewEncryptionKeyAvailable(
      level, std::move(encrypter));
}

bool WebTransportClientSession::OnNewDecryptionKeyAvailable(
    quic::EncryptionLevel level,
    std::unique_ptr<quic::QuicDecrypter> decrypter,
    bool set_alternative_decrypter, bool latch_once_used) {
  if (demo_crypto_config_.use_null_one_rtt_crypter &&
      level == quic::ENCRYPTION_FORWARD_SECURE) {
    decrypter = std::make_unique<WebTransportNullDecrypter>(
        quic::Perspective::IS_CLIENT);
  }
  return quic::QuicSimpleClientSession::OnNewDecryptionKeyAvailable(
      level, std::move(decrypter), set_alternative_decrypter,
      latch_once_used);
}

std::unique_ptr<quic::QuicDecrypter>
WebTransportClientSession::AdvanceKeysAndCreateCurrentOneRttDecrypter() {
  if (demo_crypto_config_.use_null_one_rtt_crypter) {
    return std::make_unique<WebTransportNullDecrypter>(
        quic::Perspective::IS_CLIENT);
  }
  return quic::QuicSimpleClientSession::
      AdvanceKeysAndCreateCurrentOneRttDecrypter();
}

std::unique_ptr<quic::QuicEncrypter>
WebTransportClientSession::CreateCurrentOneRttEncrypter() {
  if (demo_crypto_config_.use_null_one_rtt_crypter) {
    return std::make_unique<WebTransportNullEncrypter>(
        quic::Perspective::IS_CLIENT);
  }
  return quic::QuicSimpleClientSession::CreateCurrentOneRttEncrypter();
}

}  // namespace google_quiche::samples
