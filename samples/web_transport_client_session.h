#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CLIENT_SESSION_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CLIENT_SESSION_H_

#include <memory>

#include "quiche/quic/tools/quic_simple_client_session.h"
#include "samples/web_transport_crypto_config.h"

namespace google_quiche::samples {

class WebTransportClientSession : public quic::QuicSimpleClientSession {
 public:
  WebTransportClientSession(
      const quic::QuicConfig& config,
      const quic::ParsedQuicVersionVector& supported_versions,
      quic::QuicConnection* connection,
      quic::QuicClientBase::NetworkHelper* network_helper,
      const quic::QuicServerId& server_id,
      quic::QuicCryptoClientConfig* crypto_config, bool drop_response_body,
      bool enable_web_transport, WebTransportCryptoConfig demo_crypto_config);

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

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CLIENT_SESSION_H_
