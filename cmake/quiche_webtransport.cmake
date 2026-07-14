include_guard(GLOBAL)

if(NOT TARGET quiche_h3)
  message(FATAL_ERROR "WebTransport requires the quiche_h3 target")
endif()

add_library(quiche_webtransport STATIC
  ${quiche_webtransport_sources}
  ${quiche_webtransport_h3_sources}
)
add_library(quiche::webtransport ALIAS quiche_webtransport)

target_compile_features(quiche_webtransport PUBLIC cxx_std_17)
target_link_libraries(quiche_webtransport PUBLIC quiche::h3)

set_target_properties(quiche_webtransport PROPERTIES FOLDER ${PROJECT_NAME})

# QUICHE ships a native WebTransport test server (echo and devious-baton
# endpoints), but no native sample client. Browsers can connect to this server
# with the JavaScript WebTransport API.
if(GOOGLE_QUICHE_BUILD_WEBTRANSPORT AND GOOGLE_QUICHE_BUILD_DEMOS)
  add_executable(web_transport_test_server
    ${QUICHE_ROOT}/quiche/quic/tools/devious_baton.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_backend_response.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_server.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_crypto_server_stream_helper.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_dispatcher.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_server_session.cc
    ${QUICHE_ROOT}/quiche/quic/tools/quic_simple_server_stream.cc
    ${QUICHE_ROOT}/quiche/quic/tools/web_transport_only_backend.cc
    ${QUICHE_ROOT}/quiche/quic/tools/web_transport_test_server.cc
  )
  target_compile_features(web_transport_test_server PRIVATE cxx_std_17)
  target_link_libraries(web_transport_test_server PRIVATE quiche::webtransport)
  set_target_properties(web_transport_test_server PROPERTIES
    FOLDER ${PROJECT_NAME}/samples
  )
endif()
