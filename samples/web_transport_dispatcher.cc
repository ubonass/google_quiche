#include "samples/web_transport_dispatcher.h"

#include <memory>
#include <utility>

#include "quiche/quic/core/quic_connection.h"
#include "samples/web_transport_server_session.h"

namespace google_quiche::samples {

WebTransportDispatcher::WebTransportDispatcher(
    const quic::QuicConfig* config,
    const quic::QuicCryptoServerConfig* crypto_config,
    quic::QuicVersionManager* version_manager,
    std::unique_ptr<quic::QuicConnectionHelperInterface> helper,
    std::unique_ptr<quic::QuicCryptoServerStreamBase::Helper> session_helper,
    std::unique_ptr<quic::QuicAlarmFactory> alarm_factory,
    quic::QuicSimpleServerBackend* backend,
    uint8_t expected_server_connection_id_length,
    quic::ConnectionIdGeneratorInterface& generator,
    WebTransportCryptoConfig demo_crypto_config)
    : quic::QuicSimpleDispatcher(
          config, crypto_config, version_manager, std::move(helper),
          std::move(session_helper), std::move(alarm_factory), backend,
          expected_server_connection_id_length, generator),
      demo_crypto_config_(demo_crypto_config) {}

std::unique_ptr<quic::QuicSession> WebTransportDispatcher::CreateQuicSession(
    quic::QuicConnectionId connection_id,
    const quic::QuicSocketAddress& self_address,
    const quic::QuicSocketAddress& peer_address, absl::string_view,
    const quic::ParsedQuicVersion& version, const quic::ParsedClientHello&,
    quic::ConnectionIdGeneratorInterface& connection_id_generator) {
  auto* connection = new quic::QuicConnection(
      connection_id, self_address, peer_address, helper(), alarm_factory(),
      writer(), /*owns_writer=*/false, quic::Perspective::IS_SERVER,
      quic::ParsedQuicVersionVector{version}, connection_id_generator);
  auto session = std::make_unique<WebTransportServerSession>(
      config(), GetSupportedVersions(), connection, this, session_helper(),
      crypto_config(), compressed_certs_cache(), server_backend(),
      demo_crypto_config_);
  session->Initialize();
  return session;
}

}  // namespace google_quiche::samples
