include_guard(GLOBAL)

if(NOT DEFINED QUICHE_ROOT)
  message(FATAL_ERROR "QUICHE_ROOT must be set before including this module")
endif()

# QBone sample support.
# Dependencies: quiche_core_sources + quiche_qbone_sources. The trace visitor
# uses core tracing APIs; qbone_client.cc uses the QBone session/packet stack.
# This group contains reusable implementation sources, not a standalone main.
set(quiche_sample_qbone_sources
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_client.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_trace_visitor.cc
)

# MASQUE executable entry points.
# Dependencies: quiche_core_sources + quiche_h3_sources and the matching client
# or server implementation subset from quiche_sample_tool_sources. Each
# *_bin.cc owns a separate main() and must be placed in its own executable.
set(quiche_sample_masque_sources
  ${QUICHE_ROOT}/quiche/quic/masque/masque_client_bin.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_ohttp_client_bin.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_server_bin.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_tcp_client_bin.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_tcp_server_bin.cc
)

# MoQT sample implementations and executable entry points.
# Dependencies: quiche_core_sources + quiche_h3_sources +
# quiche_webtransport_sources + quiche_webtransport_h3_sources +
# quiche_moqt_sources, plus the required networking subset from
# quiche_sample_tool_sources. The *_bin.cc
# files are separate executables and must not be linked into one target.
set(quiche_sample_moqt_sources
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_client.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_client_bin.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_server.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_server_bin.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moq_chat.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moqt_client.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moqt_ingestion_server_bin.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moqt_server.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moqt_simulator_bin.cc
)

# Shared QUIC/HTTP tool implementations and assorted command-line entry points.
# Dependencies: quiche_core_sources; HTTP/3 clients, servers, QPACK and CONNECT
# tools additionally require quiche_h3_sources. Other sample groups reuse
# selected files from this list, but the list itself is not one linkable target
# because it contains multiple independent *_bin.cc main() functions.
set(quiche_sample_tool_sources
  ${QUICHE_ROOT}/quiche/quic/tools/connect_server_backend.cc
  ${QUICHE_ROOT}/quiche/quic/tools/connect_tunnel.cc
  ${QUICHE_ROOT}/quiche/quic/tools/connect_udp_tunnel.cc
  ${QUICHE_ROOT}/quiche/quic/tools/crypto_message_printer_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/interactive_cli.cc
  ${QUICHE_ROOT}/quiche/quic/tools/interactive_cli_demo_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/qpack_offline_decoder_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_backend_response.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_client_base.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_client_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_client_default_network_helper.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_client_interop_test_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_default_client.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_memory_cache_backend.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_name_lookup.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_packet_printer_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_reject_reason_decoder_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_server.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_server_bin.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_server_factory.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_client_session.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_client_stream.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_crypto_server_stream_helper.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_dispatcher.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_server_session.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_server_stream.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_spdy_client_base.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_tcp_like_trace_converter.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_toy_client.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_toy_server.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_url.cc
  ${QUICHE_ROOT}/quiche/quic/tools/simple_ticket_crypter.cc
)

# WebTransport test/sample server support.
# Dependencies: quiche_core_sources + quiche_h3_sources +
# quiche_webtransport_sources + quiche_webtransport_h3_sources, together with
# the QUIC server/backend subset from quiche_sample_tool_sources.
# web_transport_test_server.cc owns main(); no native sample client is present.
set(quiche_sample_webtransport_sources
  ${QUICHE_ROOT}/quiche/quic/tools/devious_baton.cc
  ${QUICHE_ROOT}/quiche/quic/tools/web_transport_only_backend.cc
  ${QUICHE_ROOT}/quiche/quic/tools/web_transport_test_server.cc
)

# Aggregate list retained for consumers that build every optional sample.
# Executable entry points remain classified above so targets can select only
# one *_bin.cc (or other main-bearing source) at a time.
set(quiche_sample_sources
  ${quiche_sample_qbone_sources}
  ${quiche_sample_masque_sources}
  ${quiche_sample_moqt_sources}
  ${quiche_sample_tool_sources}
  ${quiche_sample_webtransport_sources}
)


# POSIX sample networking implementation.
if(NOT WIN32)
  list(APPEND quiche_sample_tool_sources
    ${QUICHE_ROOT}/quiche/quic/tools/quic_epoll_client_factory.cc
  )
  list(APPEND quiche_sample_sources
    ${QUICHE_ROOT}/quiche/quic/tools/quic_epoll_client_factory.cc
  )
endif()
