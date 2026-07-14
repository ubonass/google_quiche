include_guard(GLOBAL)

if(NOT TARGET quiche_webtransport)
  message(FATAL_ERROR "MoQT requires the quiche_webtransport target")
endif()

# MoQT uses C++20 features (including defaulted three-way comparison), while
# the underlying QUIC and WebTransport libraries remain C++17-compatible.
add_library(quiche_moqt STATIC ${quiche_moqt_sources})
add_library(quiche::moqt ALIAS quiche_moqt)
target_compile_features(quiche_moqt PUBLIC cxx_std_20)
target_link_libraries(quiche_moqt PUBLIC quiche::webtransport)
set_target_properties(quiche_moqt PROPERTIES FOLDER ${PROJECT_NAME})

if(GOOGLE_QUICHE_BUILD_DEMOS)
set(_moqt_server_tool_sources
  ${QUICHE_ROOT}/quiche/quic/tools/quic_backend_response.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_server.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_crypto_server_stream_helper.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_dispatcher.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_server_session.cc
  ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_server_stream.cc
  ${QUICHE_ROOT}/quiche/quic/tools/web_transport_only_backend.cc
)

add_executable(moqt_chat_server
  ${_moqt_server_tool_sources}
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_server.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_server_bin.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moq_chat.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/tools/moqt_server.cc
)
target_compile_features(moqt_chat_server PRIVATE cxx_std_20)
target_link_libraries(moqt_chat_server PRIVATE quiche::moqt)
set_target_properties(moqt_chat_server PROPERTIES
  FOLDER ${PROJECT_NAME}/samples
)

# The upstream interactive chat client uses poll(), termios and unistd APIs.
# Keep it available on its supported POSIX platforms instead of exposing a
# Windows target that cannot compile without porting the terminal frontend.
if(UNIX)
  add_executable(moqt_chat_client
    ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_client.cc
    ${QUICHE_ROOT}/quiche/quic/moqt/tools/chat_client_bin.cc
    ${QUICHE_ROOT}/quiche/quic/moqt/tools/moq_chat.cc
    ${QUICHE_ROOT}/quiche/quic/moqt/tools/moqt_client.cc
    ${QUICHE_ROOT}/quiche/quic/tools/interactive_cli.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_client_base.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_client_default_network_helper.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_default_client.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_name_lookup.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_client_session.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_client_stream.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_spdy_client_base.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_url.cc
  )
  target_compile_features(moqt_chat_client PRIVATE cxx_std_20)
  target_link_libraries(moqt_chat_client PRIVATE quiche::moqt)
  set_target_properties(moqt_chat_client PROPERTIES
    FOLDER ${PROJECT_NAME}/samples
  )
else()
  message(STATUS
    "MoQT chat client is unavailable on Windows because its upstream terminal frontend is POSIX-only")
endif()
endif()
