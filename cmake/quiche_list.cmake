include_guard(GLOBAL)

if(NOT DEFINED QUICHE_ROOT)
  message(FATAL_ERROR "QUICHE_ROOT must be set before including quiche_list.cmake")
endif()

# Explicit production source manifests for QUICHE M137. Headers intentionally
# stay out of target source lists; expose them with
# target_include_directories() instead. Sample and test manifests live in
# quiche_sample_list.cmake and quiche_test_list.cmake respectively. Keep each
# entry explicit so revision upgrades produce reviewable diffs.
#
# Source-level dependency overview:
#
#   quiche_blind_sign_auth_sources
#       `-- base utilities + BlindSignAuth protobuf messages
#
#   quiche_base_sources <-> quiche_core_sources
#
#   quiche_qbone_sources
#       `-- base + core
#
#   quiche_h3_sources
#       `-- base + core
#
#   quiche_masque_sources
#       `-- base + core + h3 (POSIX networking implementation)
#
#   quiche_webtransport_sources
#       `-- base + core
#
#   quiche_moqt_sources
#       `-- base + core + webtransport
#
#   quiche_webtransport_h3_sources
#       |-- base + core
#       |-- h3
#       `-- webtransport
#
# base and core form a dependency cycle in upstream QUICHE and should normally
# be combined into one quiche_core target. H3 sources also reference the H3
# WebTransport adapter APIs, so quiche_h3_sources and
# quiche_webtransport_h3_sources should currently be combined into one H3
# target when WebTransport-over-H3 is enabled. The source lists remain separate
# to make the protocol boundary explicit and support a future adapter refactor.

set(quiche_blind_sign_auth_sources
  ${QUICHE_ROOT}/quiche/blind_sign_auth/blind_sign_auth.cc
  ${QUICHE_ROOT}/quiche/blind_sign_auth/blind_sign_message_response.cc
  ${QUICHE_ROOT}/quiche/blind_sign_auth/cached_blind_sign_auth.cc
)

set(quiche_base_sources
  ${QUICHE_ROOT}/quiche/common/bug_utils.cc
  ${QUICHE_ROOT}/quiche/common/http/http_header_block.cc
  ${QUICHE_ROOT}/quiche/common/http/http_header_storage.cc
  ${QUICHE_ROOT}/quiche/common/internet_checksum.cc
  ${QUICHE_ROOT}/quiche/common/platform/api/quiche_file_utils.cc
  ${QUICHE_ROOT}/quiche/common/platform/api/quiche_hostname_utils.cc
  ${QUICHE_ROOT}/quiche/common/platform/api/quiche_mem_slice.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_command_line_flags_impl.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_default_proof_providers_impl.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_file_utils_impl.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_flags_impl.cc
  # ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_logging_impl.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_stack_trace_impl.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_time_utils_impl.cc
  ${QUICHE_ROOT}/quiche/common/platform/default/quiche_platform_impl/quiche_url_utils_impl.cc
  ${QUICHE_ROOT}/quiche/common/quiche_buffer_allocator.cc
  ${QUICHE_ROOT}/quiche/common/quiche_crypto_logging.cc
  ${QUICHE_ROOT}/quiche/common/quiche_data_reader.cc
  ${QUICHE_ROOT}/quiche/common/quiche_data_writer.cc
  ${QUICHE_ROOT}/quiche/common/quiche_ip_address.cc
  ${QUICHE_ROOT}/quiche/common/quiche_ip_address_family.cc
  ${QUICHE_ROOT}/quiche/common/quiche_mem_slice_storage.cc
  ${QUICHE_ROOT}/quiche/common/quiche_random.cc
  ${QUICHE_ROOT}/quiche/common/quiche_simple_arena.cc
  ${QUICHE_ROOT}/quiche/common/quiche_socket_address.cc
  ${QUICHE_ROOT}/quiche/common/quiche_text_utils.cc
  ${QUICHE_ROOT}/quiche/common/simple_buffer_allocator.cc
  ${QUICHE_ROOT}/quiche/common/structured_headers.cc
  ${QUICHE_ROOT}/quiche/common/vectorized_io_utils.cc
)

set(quiche_core_sources
  ${QUICHE_ROOT}/quiche/quic/core/chlo_extractor.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bandwidth_sampler.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr2_drain.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr2_misc.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr2_probe_bw.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr2_probe_rtt.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr2_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr2_startup.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/bbr_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/cubic_bytes.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/general_loss_algorithm.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/hybrid_slow_start.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/pacing_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/prague_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/prr_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/rtt_stats.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/send_algorithm_interface.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/tcp_cubic_sender_bytes.cc
  ${QUICHE_ROOT}/quiche/quic/core/congestion_control/uber_loss_algorithm.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aead_base_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aead_base_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_128_gcm_12_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_128_gcm_12_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_128_gcm_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_128_gcm_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_256_gcm_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_256_gcm_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_base_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/aes_base_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/cert_compressor.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/certificate_util.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/certificate_view.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/chacha20_poly1305_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/chacha20_poly1305_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/chacha20_poly1305_tls_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/chacha20_poly1305_tls_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/chacha_base_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/chacha_base_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/channel_id.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/client_proof_source.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/crypto_framer.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/crypto_handshake.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/crypto_handshake_message.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/crypto_secret_boxer.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/crypto_utils.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/curve25519_key_exchange.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/key_exchange.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/null_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/null_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/p256_key_exchange.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/proof_source.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/proof_source_x509.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_client_session_cache.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_compressed_certs_cache.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_crypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_crypto_client_config.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_crypto_proof.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_crypto_server_config.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_decrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_encrypter.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/quic_hkdf.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/tls_client_connection.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/tls_connection.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/tls_server_connection.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/transport_parameters.cc
  ${QUICHE_ROOT}/quiche/quic/core/deterministic_connection_id_generator.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_ack_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_ack_frequency_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_blocked_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_connection_close_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_crypto_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_goaway_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_handshake_done_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_immediate_ack_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_max_streams_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_message_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_new_connection_id_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_new_token_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_padding_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_path_challenge_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_path_response_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_ping_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_reset_stream_at_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_retire_connection_id_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_rst_stream_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_stop_sending_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_stop_waiting_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_stream_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_streams_blocked_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/frames/quic_window_update_frame.cc
  ${QUICHE_ROOT}/quiche/quic/core/io/event_loop_connecting_client_socket.cc
  ${QUICHE_ROOT}/quiche/quic/core/io/event_loop_socket_factory.cc
  ${QUICHE_ROOT}/quiche/quic/core/io/quic_default_event_loop.cc
  ${QUICHE_ROOT}/quiche/quic/core/io/quic_poll_event_loop.cc
  ${QUICHE_ROOT}/quiche/quic/core/io/quic_server_io_harness.cc
  ${QUICHE_ROOT}/quiche/quic/core/io/socket.cc
  ${QUICHE_ROOT}/quiche/quic/core/legacy_quic_stream_id_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_ack_listener_interface.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_alarm.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_bandwidth.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_blocked_writer_list.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_buffered_packet_store.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_chaos_protector.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_coalesced_packet.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_config.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_connection.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_connection_alarms.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_connection_context.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_connection_id.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_connection_id_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_connection_stats.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_constants.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_control_frame_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_crypto_client_handshaker.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_crypto_client_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_crypto_handshaker.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_crypto_server_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_crypto_server_stream_base.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_crypto_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_data_reader.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_data_writer.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_datagram_queue.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_default_clock.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_default_packet_writer.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_dispatcher.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_dispatcher_stats.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_error_codes.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_flow_controller.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_framer.cc
  ${QUICHE_ROOT}/quiche/http2/core/spdy_protocol.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_idle_network_detector.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_mtu_discovery.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_network_blackhole_detector.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_packet_creator.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_packet_number.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_packet_reader.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_packet_writer_wrapper.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_packets.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_path_validator.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_ping_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_queue_alarm_factory.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_received_packet_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_sent_packet_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_server_id.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_session.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_socket_address_coder.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_stream_id_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_stream_priority.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_stream_send_buffer.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_stream_sequencer.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_stream_sequencer_buffer.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_sustained_bandwidth_recorder.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_tag.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_time.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_time_wait_list_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_transmission_info.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_types.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_udp_socket.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_unacked_packet_map.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_utils.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_version_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_versions.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_write_blocked_list.cc
  ${QUICHE_ROOT}/quiche/quic/core/tls_chlo_extractor.cc
  ${QUICHE_ROOT}/quiche/quic/core/tls_client_handshaker.cc
  ${QUICHE_ROOT}/quiche/quic/core/tls_handshaker.cc
  ${QUICHE_ROOT}/quiche/quic/core/tls_server_handshaker.cc
  ${QUICHE_ROOT}/quiche/quic/core/uber_quic_stream_id_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/uber_received_packet_manager.cc
  ${QUICHE_ROOT}/quiche/quic/load_balancer/load_balancer_config.cc
  ${QUICHE_ROOT}/quiche/quic/load_balancer/load_balancer_decoder.cc
  ${QUICHE_ROOT}/quiche/quic/load_balancer/load_balancer_encoder.cc
  ${QUICHE_ROOT}/quiche/quic/load_balancer/load_balancer_server_id.cc
)

set(quiche_qbone_sources
  ${QUICHE_ROOT}/quiche/quic/qbone/bonnet/icmp_reachable.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/bonnet/qbone_tunnel_info.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/bonnet/qbone_tunnel_silo.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/bonnet/tun_device.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/bonnet/tun_device_controller.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/bonnet/tun_device_packet_exchanger.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/platform/icmp_packet.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/platform/ip_range.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/platform/netlink.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/platform/rtnetlink_message.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/platform/tcp_packet.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_client_session.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_constants.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_control_stream.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_packet_exchanger.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_packet_processor.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_packet_processor_test_tools.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_server_session.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_session_base.cc
  ${QUICHE_ROOT}/quiche/quic/qbone/qbone_stream.cc
)

set(quiche_h3_sources
  ${QUICHE_ROOT}/quiche/balsa/balsa_enums.cc
  ${QUICHE_ROOT}/quiche/balsa/balsa_frame.cc
  ${QUICHE_ROOT}/quiche/balsa/balsa_headers.cc
  ${QUICHE_ROOT}/quiche/balsa/balsa_headers_sequence.cc
  ${QUICHE_ROOT}/quiche/balsa/header_properties.cc
  ${QUICHE_ROOT}/quiche/balsa/simple_buffer.cc
  ${QUICHE_ROOT}/quiche/balsa/standard_header_map.cc
  ${QUICHE_ROOT}/quiche/binary_http/binary_http_message.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/chunked_buffer.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/event_forwarder.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/header_validator.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/http2_protocol.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/http2_util.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/noop_header_validator.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/oghttp2_adapter.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/oghttp2_session.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/oghttp2_util.cc
  ${QUICHE_ROOT}/quiche/http2/adapter/window_manager.cc
  ${QUICHE_ROOT}/quiche/http2/core/array_output_buffer.cc
  ${QUICHE_ROOT}/quiche/http2/core/http2_constants.cc
  ${QUICHE_ROOT}/quiche/http2/core/http2_frame_decoder_adapter.cc
  ${QUICHE_ROOT}/quiche/http2/core/http2_structures.cc
  ${QUICHE_ROOT}/quiche/http2/core/http2_trace_logging.cc
  ${QUICHE_ROOT}/quiche/http2/core/recording_headers_handler.cc
  ${QUICHE_ROOT}/quiche/http2/core/spdy_alt_svc_wire_format.cc
  ${QUICHE_ROOT}/quiche/http2/core/spdy_frame_builder.cc
  ${QUICHE_ROOT}/quiche/http2/core/spdy_framer.cc
  ${QUICHE_ROOT}/quiche/http2/core/spdy_no_op_visitor.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/decode_buffer.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/decode_http2_structures.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/decode_status.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/frame_decoder_state.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/http2_frame_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/http2_frame_decoder_listener.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/http2_structure_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/altsvc_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/continuation_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/data_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/goaway_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/headers_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/ping_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/priority_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/priority_update_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/push_promise_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/rst_stream_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/settings_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/unknown_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/decoder/payload_decoders/window_update_payload_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_block_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_decoder_listener.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_decoder_state.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_decoder_string_buffer.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_decoder_tables.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_decoding_error.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_entry_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_entry_decoder_listener.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_entry_type_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_string_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_string_decoder_listener.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_whole_entry_buffer.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/decoder/hpack_whole_entry_listener.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_constants.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_decoder_adapter.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_encoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_entry.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_header_table.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_output_stream.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/hpack_static_table.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/http2_hpack_constants.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/huffman/hpack_huffman_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/huffman/hpack_huffman_encoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/huffman/huffman_spec_tables.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/varint/hpack_varint_decoder.cc
  ${QUICHE_ROOT}/quiche/http2/hpack/varint/hpack_varint_encoder.cc
  ${QUICHE_ROOT}/quiche/oblivious_http/buffers/oblivious_http_request.cc
  ${QUICHE_ROOT}/quiche/oblivious_http/buffers/oblivious_http_response.cc
  ${QUICHE_ROOT}/quiche/oblivious_http/common/oblivious_http_header_key_config.cc
  ${QUICHE_ROOT}/quiche/oblivious_http/oblivious_http_client.cc
  ${QUICHE_ROOT}/quiche/oblivious_http/oblivious_http_gateway.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/http_constants.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/http_decoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/http_encoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/metadata_decoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_header_list.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_headers_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_receive_control_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_send_control_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_server_initiated_spdy_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_server_session_base.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_client_session.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_client_session_base.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_client_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_server_stream_base.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_session.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/quic_spdy_stream_body_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/spdy_utils.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/new_qpack_blocking_manager.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_decoded_headers_accumulator.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_decoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_decoder_stream_receiver.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_decoder_stream_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_encoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_encoder_stream_receiver.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_encoder_stream_sender.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_header_table.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_index_conversions.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_instruction_decoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_instruction_encoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_instructions.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_progressive_decoder.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_receive_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_required_insert_count.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_send_stream.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/qpack_static_table.cc
  ${QUICHE_ROOT}/quiche/quic/core/qpack/value_splitting_header_list.cc
)

# MASQUE is not part of the HTTP/3 transport library itself. Its current tool
# implementation uses POSIX networking APIs (including netdb.h), so keep it in
# a separate manifest for a future platform-specific MASQUE target.
set(quiche_masque_sources
  ${QUICHE_ROOT}/quiche/common/masque/connect_ip_datagram_payload.cc
  ${QUICHE_ROOT}/quiche/common/masque/connect_udp_datagram_payload.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_client.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_client_session.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_client_tools.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_connection_pool.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_dispatcher.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_encapsulated_client.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_encapsulated_client_session.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_h2_connection.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_server.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_server_backend.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_server_session.cc
  ${QUICHE_ROOT}/quiche/quic/masque/masque_utils.cc
)

set(quiche_webtransport_sources
  ${QUICHE_ROOT}/quiche/common/capsule.cc
  ${QUICHE_ROOT}/quiche/quic/core/crypto/web_transport_fingerprint_proof_verifier.cc
  ${QUICHE_ROOT}/quiche/quic/core/web_transport_stats.cc
  ${QUICHE_ROOT}/quiche/quic/core/web_transport_write_blocked_list.cc
  ${QUICHE_ROOT}/quiche/web_transport/complete_buffer_visitor.cc
  ${QUICHE_ROOT}/quiche/web_transport/encapsulated/encapsulated_web_transport.cc
  ${QUICHE_ROOT}/quiche/web_transport/web_transport_headers.cc
  ${QUICHE_ROOT}/quiche/web_transport/web_transport_priority_scheduler.cc
)

set(quiche_moqt_sources
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_bitrate_adjuster.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_cached_object.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_framer.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_known_track_publisher.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_live_relay_queue.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_messages.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_outgoing_queue.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_parser.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_priority.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_probe_manager.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_session.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_subscribe_windows.cc
  ${QUICHE_ROOT}/quiche/quic/moqt/moqt_track.cc
)

set(quiche_webtransport_h3_sources
  ${QUICHE_ROOT}/quiche/quic/core/http/web_transport_http3.cc
  ${QUICHE_ROOT}/quiche/quic/core/http/web_transport_stream_adapter.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_generic_session.cc
)

# Linux packet batching uses sendmmsg and UDP GSO. Although the base/buffer
# class names are generic, their current implementation stores Linux-specific
# BufferedWrite/QuicMsgHdr types from quic_linux_socket_utils.h. Windows uses
# QuicDefaultPacketWriter through the Winsock-backed QuicUdpSocketApi instead;
# QUICHE does not currently provide a native Windows batch-writer backend.
set(quiche_linux_batch_writer_sources
  ${QUICHE_ROOT}/quiche/quic/core/batch_writer/quic_batch_writer_base.cc
  ${QUICHE_ROOT}/quiche/quic/core/batch_writer/quic_batch_writer_buffer.cc
  ${QUICHE_ROOT}/quiche/quic/core/batch_writer/quic_gso_batch_writer.cc
  ${QUICHE_ROOT}/quiche/quic/core/batch_writer/quic_sendmmsg_batch_writer.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_linux_socket_utils.cc
  ${QUICHE_ROOT}/quiche/quic/core/quic_syscall_wrapper.cc
)

# Linux-only UDP batching implementation.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND quiche_core_sources
    ${quiche_linux_batch_writer_sources}
  )
endif()

# Libevent integration is available to POSIX builds when the dependency is
# enabled; it is independent from the Linux-only batch writer above.
if(UNIX)
  list(APPEND quiche_core_sources
    ${QUICHE_ROOT}/quiche/quic/bindings/quic_libevent.cc
  )
endif()

# Load the separately maintained optional manifests after all production groups
# are defined. This preserves the existing include(quiche_list) contract:
# callers receive production, sample and test variables from one entry point,
# while each source category remains maintained in its own file.
include("${CMAKE_CURRENT_LIST_DIR}/quiche_sample_list.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/quiche_test_list.cmake")
