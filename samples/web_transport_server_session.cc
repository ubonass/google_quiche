#include "samples/web_transport_server_session.h"

#include <memory>
#include <utility>

#include "samples/web_transport_null_decrypter.h"
#include "samples/web_transport_null_encrypter.h"

namespace google_quiche::samples {

WebTransportServerSession::WebTransportServerSession(
    const quic::QuicConfig& config,
    const quic::ParsedQuicVersionVector& supported_versions,
    quic::QuicConnection* connection, quic::QuicSession::Visitor* visitor,
    quic::QuicCryptoServerStreamBase::Helper* helper,
    const quic::QuicCryptoServerConfig* crypto_config,
    quic::QuicCompressedCertsCache* compressed_certs_cache,
    quic::QuicSimpleServerBackend* backend,
    WebTransportCryptoConfig demo_crypto_config)
    : quic::QuicSimpleServerSession(
          config, supported_versions, connection, visitor, helper,
          crypto_config, compressed_certs_cache, backend),
      demo_crypto_config_(demo_crypto_config) {}

void WebTransportServerSession::OnNewEncryptionKeyAvailable(
    quic::EncryptionLevel level,
    std::unique_ptr<quic::QuicEncrypter> encrypter) {
  if (demo_crypto_config_.use_null_one_rtt_crypter &&
      level == quic::ENCRYPTION_FORWARD_SECURE) {
    encrypter = std::make_unique<WebTransportNullEncrypter>(
        quic::Perspective::IS_SERVER);
  }
  quic::QuicSimpleServerSession::OnNewEncryptionKeyAvailable(
      level, std::move(encrypter));
}

bool WebTransportServerSession::OnNewDecryptionKeyAvailable(
    quic::EncryptionLevel level,
    std::unique_ptr<quic::QuicDecrypter> decrypter,
    bool set_alternative_decrypter, bool latch_once_used) {
  if (demo_crypto_config_.use_null_one_rtt_crypter &&
      level == quic::ENCRYPTION_FORWARD_SECURE) {
    decrypter = std::make_unique<WebTransportNullDecrypter>(
        quic::Perspective::IS_SERVER);
  }
  return quic::QuicSimpleServerSession::OnNewDecryptionKeyAvailable(
      level, std::move(decrypter), set_alternative_decrypter,
      latch_once_used);
}

std::unique_ptr<quic::QuicDecrypter>
WebTransportServerSession::AdvanceKeysAndCreateCurrentOneRttDecrypter() {
  if (demo_crypto_config_.use_null_one_rtt_crypter) {
    return std::make_unique<WebTransportNullDecrypter>(
        quic::Perspective::IS_SERVER);
  }
  return quic::QuicSimpleServerSession::
      AdvanceKeysAndCreateCurrentOneRttDecrypter();
}

std::unique_ptr<quic::QuicEncrypter>
WebTransportServerSession::CreateCurrentOneRttEncrypter() {
  if (demo_crypto_config_.use_null_one_rtt_crypter) {
    return std::make_unique<WebTransportNullEncrypter>(
        quic::Perspective::IS_SERVER);
  }
  return quic::QuicSimpleServerSession::CreateCurrentOneRttEncrypter();
}

}  // namespace google_quiche::samples
