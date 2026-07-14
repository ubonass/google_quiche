include_guard(GLOBAL)

# Generate QUICHE's protobuf messages with the protoc target provided by the
# top-level protobuf dependency. QUICHE links only the lite runtime.
include(quiche_proto)

# QUICHE common and QUIC transport are mutually dependent upstream, so they
# intentionally form one static library. Optional QBone, H3, WebTransport,
# sample and test source groups are not part of this target.
include(quiche_list)
add_library(quiche_core STATIC
  ${quiche_base_sources}
  ${quiche_core_sources}
  ${quiche_proto_srcs}
  ${CMAKE_SOURCE_DIR}/overrides/quiche_platform_impl/quiche_logging_impl.cc
)
add_library(quiche::core ALIAS quiche_core)

# Publish the language requirement and include roots because QUICHE public
# headers include other QUICHE headers, generated protobuf headers and platform
# implementation headers directly.
target_compile_features(quiche_core PUBLIC cxx_std_17)
target_include_directories(quiche_core
  PUBLIC
    ${CMAKE_SOURCE_DIR}/overrides
    ${QUICHE_ROOT}
    ${QUICHE_ROOT}/quiche/common/platform/default
    ${CMAKE_CURRENT_BINARY_DIR}
)

# This upstream revision expects Chromium's shared build prefix to expose the
# Abseil ASCII helpers used by the Windows socket implementation. Provide the
# header at the target level so the vendored QUICHE sources remain unchanged.
target_precompile_headers(quiche_core PRIVATE
  <absl/strings/ascii.h>
)

target_link_libraries(quiche_core
  PUBLIC
    # Project libraries.
    # base::base

    # Crypto, compression and generated-message runtime.
    OpenSSL::Crypto
    OpenSSL::SSL
    protobuf::libprotobuf-lite
    ZLIB::ZLIB

    # Abseil components exposed by or used throughout QUICHE core headers.
    absl::algorithm
    absl::base
    absl::btree
    absl::cleanup
    absl::core_headers
    absl::flat_hash_map
    absl::flat_hash_set
    absl::flags
    absl::flags_parse
    absl::flags_usage
    absl::function_ref
    absl::hash
    absl::inlined_vector
    absl::log_initialize
    absl::log_severity
    absl::absl_check
    absl::absl_log
    absl::memory
    absl::node_hash_map
    absl::span
    absl::status
    absl::statusor
    absl::str_format
    absl::strings
    absl::synchronization
    absl::time

  PUBLIC
    # QUICHE's public platform adapter quiche_googleurl_impl.h includes
    # url/gurl.h, so consumers need googleurl's include root as well.
    google_url::google_url
)

# Prevent Windows SDK min/max macros from breaking standard-library calls and
# link the socket/IP helper libraries used by the QUIC transport implementation.
if(WIN32)
  target_compile_definitions(quiche_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
  target_link_libraries(quiche_core PUBLIC ws2_32 iphlpapi)
endif()

# Keep the target organized in IDE generators such as Visual Studio.
set_target_properties(quiche_core PROPERTIES FOLDER ${PROJECT_NAME})
