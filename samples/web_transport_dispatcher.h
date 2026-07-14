#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_DISPATCHER_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_DISPATCHER_H_

#include <memory>

#include "quiche/quic/tools/quic_simple_dispatcher.h"
#include "samples/web_transport_crypto_config.h"

namespace google_quiche::samples {

class WebTransportDispatcher : public quic::QuicSimpleDispatcher {
 public:
  WebTransportDispatcher(
      const quic::QuicConfig* config,
      const quic::QuicCryptoServerConfig* crypto_config,
      quic::QuicVersionManager* version_manager,
      std::unique_ptr<quic::QuicConnectionHelperInterface> helper,
      std::unique_ptr<quic::QuicCryptoServerStreamBase::Helper> session_helper,
      std::unique_ptr<quic::QuicAlarmFactory> alarm_factory,
      quic::QuicSimpleServerBackend* backend,
      uint8_t expected_server_connection_id_length,
      quic::ConnectionIdGeneratorInterface& generator,
      WebTransportCryptoConfig demo_crypto_config);

 protected:
  std::unique_ptr<quic::QuicSession> CreateQuicSession(
      quic::QuicConnectionId connection_id,
      const quic::QuicSocketAddress& self_address,
      const quic::QuicSocketAddress& peer_address, absl::string_view alpn,
      const quic::ParsedQuicVersion& version,
      const quic::ParsedClientHello& parsed_chlo,
      quic::ConnectionIdGeneratorInterface& connection_id_generator) override;

 private:
  WebTransportCryptoConfig demo_crypto_config_;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_DISPATCHER_H_
