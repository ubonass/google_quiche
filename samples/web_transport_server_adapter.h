#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_SERVER_ADAPTER_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_SERVER_ADAPTER_H_

#include <memory>

#include "quiche/quic/tools/quic_server.h"
#include "samples/web_transport_crypto_config.h"

namespace google_quiche::samples {

class WebTransportServerAdapter : public quic::QuicServer {
 public:
  WebTransportServerAdapter(
      std::unique_ptr<quic::ProofSource> proof_source,
      quic::QuicSimpleServerBackend* backend,
      WebTransportCryptoConfig demo_crypto_config);

 protected:
  quic::QuicDispatcher* CreateQuicDispatcher() override;

 private:
  WebTransportCryptoConfig demo_crypto_config_;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_SERVER_ADAPTER_H_
