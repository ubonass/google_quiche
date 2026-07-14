#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_SERVER_SESSION_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_SERVER_SESSION_H_

#include <memory>

#include "quiche/quic/tools/quic_simple_server_session.h"
#include "samples/web_transport_crypto_config.h"

namespace google_quiche::samples {

class WebTransportServerSession : public quic::QuicSimpleServerSession {
 public:
  WebTransportServerSession(
      const quic::QuicConfig& config,
      const quic::ParsedQuicVersionVector& supported_versions,
      quic::QuicConnection* connection, quic::QuicSession::Visitor* visitor,
      quic::QuicCryptoServerStreamBase::Helper* helper,
      const quic::QuicCryptoServerConfig* crypto_config,
      quic::QuicCompressedCertsCache* compressed_certs_cache,
      quic::QuicSimpleServerBackend* backend,
      WebTransportCryptoConfig demo_crypto_config);

  void OnNewEncryptionKeyAvailable(
      quic::EncryptionLevel level,
      std::unique_ptr<quic::QuicEncrypter> encrypter) override;
  bool OnNewDecryptionKeyAvailable(
      quic::EncryptionLevel level,
      std::unique_ptr<quic::QuicDecrypter> decrypter,
      bool set_alternative_decrypter, bool latch_once_used) override;
  std::unique_ptr<quic::QuicDecrypter>
  AdvanceKeysAndCreateCurrentOneRttDecrypter() override;
  std::unique_ptr<quic::QuicEncrypter> CreateCurrentOneRttEncrypter() override;

 private:
  WebTransportCryptoConfig demo_crypto_config_;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_SERVER_SESSION_H_
