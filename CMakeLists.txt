cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.)
endif()
#
# Project configuration
#
set(GOOGLE_QUICHE_VERSION_MAJOR 131)
set(GOOGLE_QUICHE_VERSION_MINOR 0)
set(GOOGLE_QUICHE_VERSION_PATCH 6778)
set(GOOGLE_QUICHE_VERSION_PATCH2 45)

project(
	google_quiche
	VERSION ${GOOGLE_QUICHE_VERSION_MAJOR}.${GOOGLE_QUICHE_VERSION_MINOR}.${GOOGLE_QUICHE_VERSION_PATCH}.${GOOGLE_QUICHE_VERSION_PATCH2}
	DESCRIPTION GURL
	HOMEPAGE_URL https://github.com/ubonass/google_quiche.git
	LANGUAGES C CXX
)

message(STATUS "CMAKE_SYSTEM_NAME:"${CMAKE_SYSTEM_NAME})
message(STATUS "CMAKE_SYSTEM_PROCESSOR:"${CMAKE_SYSTEM_PROCESSOR})

###############################################################################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif()

if (MSVC)
  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "debug")
      add_compile_definitions(_CRT_SECURE_NO_DEPRECATE)
      add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
      add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
      add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS)
  else()
      add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
      add_compile_definitions(_CRT_NONSTDC_NO_DEFPRECATES)
      add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS)
      # add debug info for exefile and output pdb file
      # set(CMAKE_BUILD_TYPE "RelWithDebInfo")
      set(CMAKE_CXX_FLAGS_RELEASE "/Zi")
      set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /INCREMENTAL:NO")
  endif()
  
  add_compile_definitions(UNICODE)
  add_compile_definitions(_UNICODE)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(NOGDI)
  add_compile_definitions(WIN32_LEAN_AND_MEAN)
  add_compile_options(/EHsc)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
  set(CMAKE_C_FLAGS$ "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  set(CMAKE_CXX_FLAGS$ "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  
  add_compile_options(-fPIC)
  add_compile_options(-Werror=shift-overflow)
  add_compile_options(-Wno-unused-command-line-argument)
  # set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g2 -ggdb -Wl,--no-keep-memory")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -g2 -ggdb")
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -O3 -Wall -g -Wno-error=unused-result") 
  
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-command-line-argument")
      if(CMAKE_SYSTEM_NAME STREQUAL "Android")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -Wno-error=implicit-function-declaration")  
      endif()
  endif() 
endif()
###############################################################################################################################
set(QUICHE_ROOT ${CMAKE_SOURCE_DIR}/quiche)
set(QUICHE_OVERRIDES ${CMAKE_SOURCE_DIR}/overrides)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH})
include(${CMAKE_TOOLCHAIN_FILE})
include(quiche_list)
include(quiche_proto)
###############################################################################################################################
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(absl REQUIRED)
find_package(protobuf REQUIRED)
find_package(double-conversion REQUIRED)
###############################################################################################################################
include_directories(${QUICHE_ROOT})
include_directories(${QUICHE_OVERRIDES})
include_directories(${CMAKE_BINARY_DIR})
###############################################################################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/google_url)
include_directories(${CMAKE_SOURCE_DIR}/google_url)
###############################################################################################################################
add_library(quiche_platform_impl
  ${default_platform_impl_srcs}
  ${default_platform_impl_hdrs}
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_flags_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_logging_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_mutex_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_stack_trace_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_time_utils_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_time_utils_impl.cc
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_url_utils_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_googleurl_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_export_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_logging_impl.cc
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_logging_impl.h
  # --->build for quiche_core lib
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_iovec_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_bug_tracker_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_prefetch_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_mem_slice_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_containers_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_flag_utils_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_reference_counted_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_client_stats_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_testvalue_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_server_stats_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_lower_case_string_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_header_policy_impl.h
)

target_link_libraries(quiche_platform_impl PRIVATE
  OpenSSL::Crypto
  absl::core_headers
  absl::log_severity
  absl::flat_hash_map
  absl::flat_hash_set
  absl::stacktrace
  absl::symbolize
  absl::flags
  absl::absl_check
  absl::absl_log
  absl::strings
  absl::str_format
  absl::synchronization
  absl::time
  absl::optional
  absl::span
  google_url::google_url)

###############################################################################################################################
add_library(quiche_core
  ${quiche_core_srcs}
  ${quiche_core_hdrs}
  ${quiche_proto_srcs}
  ${quiche_proto_hdrs} 
)

target_link_libraries(quiche_core PRIVATE
  quiche_platform_impl
  protobuf::libprotobuf-lite
  OpenSSL::Crypto
  OpenSSL::SSL
  absl::algorithm
  absl::core_headers
  absl::nullability
  absl::cleanup
  absl::btree
  absl::fixed_array
  absl::flat_hash_map
  absl::flat_hash_set
  absl::inlined_vector
  absl::node_hash_map
  absl::any_invocable
  absl::function_ref
  absl::hash
  absl::memory
  absl::bits
  absl::int128
  absl::status
  absl::statusor
  absl::strings
  absl::str_format
  absl::time
  absl::optional
  absl:ypes:span
  absl::variant
  ZLIB::ZLIB)

###############################################################################################################################
add_library(binary_http
  ${binary_http_srcs}
  ${binary_http_hdrs}
)

target_link_libraries(binary_http PRIVATE
  quiche_core
  absl::core_headers
  absl::flat_hash_map
  absl::status
  absl::statusor
  absl::strings)
###############################################################################################################################
add_library(oblivious_http
  ${oblivious_http_srcs}
  ${oblivious_http_hdrs}
)

target_link_libraries(oblivious_http PRIVATE
  quiche_core
  OpenSSL::Crypto
  absl::core_headers
  absl::btree
  absl::flat_hash_map
  absl::flat_hash_set
  absl::memory
  absl::status
  absl::statusor
  absl::strings
  absl::optional)
###############################################################################################################################
add_library(quiche_platform_impl_tools
  ${default_platform_impl_tool_support_srcs}
  ${default_platform_impl_tool_support_hdrs}
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_command_line_flags_impl.cc
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_command_line_flags_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_file_utils_impl.h
  ${QUICHE_OVERRIDES}/quiche_platform_impl/quiche_default_proof_providers_impl.h
)

target_link_libraries(quiche_platform_impl_tools PRIVATE
  quiche_core
  absl::flags
  absl::flags_parse
  absl::flags_usage
  absl::absl_log
  absl::strings
  absl::optional)
###############################################################################################################################
add_library(balsa
  ${balsa_srcs}
  ${balsa_hdrs}
)

target_link_libraries(balsa PRIVATE
  quiche_core
  absl::algorithm
  absl::core_headers
  absl::flat_hash_map
  absl::flat_hash_set
  absl::inlined_vector
  absl::memory
  absl::strings
  absl::str_format
  absl::optional
  absl::span
  absl::variant)
###############################################################################################################################
add_library(quiche_tool_support
  ${quiche_tool_support_srcs}
  ${quiche_tool_support_hdrs}
)

target_link_libraries(quiche_tool_support PRIVATE
  quiche_core
  quiche_platform_impl
  OpenSSL::Crypto
  absl::core_headers
  absl::flat_hash_map
  absl::flat_hash_set
  absl::bind_front
  absl::memory
  absl::status
  absl::statusor
  absl::strings
  absl::str_format
  absl::optional
  absl::span)
################################################################################################################################
add_library(io_tool_support
  ${io_tool_support_srcs}
  ${io_tool_support_hdrs}
)

target_link_libraries(io_tool_support PRIVATE
  quiche_core
  quiche_platform_impl
  quiche_platform_impl_tools
  quiche_tool_support
  OpenSSL::Crypto
  absl::algorithm
  absl::core_headers
  absl::cleanup
  absl::btree
  absl::flat_hash_map
  absl::flat_hash_set
  absl::memory
  absl::status
  absl::statusor
  absl::strings
  absl::str_format
  absl::optional
  absl::span
  absl::variant)
###############################################################################################################################