#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CLIENT_ADAPTER_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CLIENT_ADAPTER_H_

#include <memory>

#include "quiche/quic/tools/quic_default_client.h"
#include "samples/web_transport_crypto_config.h"

namespace google_quiche::samples {

class WebTransportClientAdapter : public quic::QuicDefaultClient {
 public:
  WebTransportClientAdapter(
      quic::QuicSocketAddress server_address,
      const quic::QuicServerId& server_id,
      const quic::ParsedQuicVersionVector& supported_versions,
      quic::QuicEventLoop* event_loop,
      std::unique_ptr<quic::ProofVerifier> proof_verifier,
      WebTransportCryptoConfig demo_crypto_config);

  std::unique_ptr<quic::QuicSession> CreateQuicClientSession(
      const quic::ParsedQuicVersionVector& supported_versions,
      quic::QuicConnection* connection) override;

 private:
  WebTransportCryptoConfig demo_crypto_config_;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CLIENT_ADAPTER_H_
