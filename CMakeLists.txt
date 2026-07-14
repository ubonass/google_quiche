# CMake 3.20 provides the target and generator-expression behavior required by
# the dependency modules in this project.
cmake_minimum_required(VERSION 3.20)

# Keep generated build artifacts out of the source tree. In-source builds make
# submodule updates and source-list reviews unnecessarily difficult.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.)
endif()
# -----------------------------------------------------------------------------
# Project metadata
# -----------------------------------------------------------------------------
# The version follows the Chromium/QUICHE milestone and patch revision used by
# this source tree.
set(GOOGLE_QUICHE_VERSION_MAJOR 137)
set(GOOGLE_QUICHE_VERSION_MINOR 0)
set(GOOGLE_QUICHE_VERSION_PATCH 7151)
set(GOOGLE_QUICHE_VERSION_PATCH2 119)

project(
	google_quiche
	VERSION ${GOOGLE_QUICHE_VERSION_MAJOR}.${GOOGLE_QUICHE_VERSION_MINOR}.${GOOGLE_QUICHE_VERSION_PATCH}.${GOOGLE_QUICHE_VERSION_PATCH2}
	DESCRIPTION GURL
	HOMEPAGE_URL https://github.com/ubonass/google_quiche.git
	LANGUAGES C CXX
)

# Enable logical solution folders for IDE generators such as Visual Studio.
# Keep this at the top level so target grouping does not depend on an optional
# subdirectory (base previously enabled this property as a side effect).
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Select one Microsoft C/C++ runtime model when targeting the MSVC ABI. This
# applies to both cl.exe and ClangCL; other platforms retain their toolchain's
# native runtime selection and do not receive Windows-specific settings.
if(MSVC)
  option(GOOGLE_QUICHE_MSVC_STATIC_RUNTIME "Link the static Microsoft C/C++ runtime" ON)
  if(GOOGLE_QUICHE_MSVC_STATIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()
endif()

# Report the selected target platform during configuration to make cross-build
# and Visual Studio generator diagnostics easier to identify.
message(STATUS "Target system: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")

# -----------------------------------------------------------------------------
# Global language and output settings
# -----------------------------------------------------------------------------
# The default base/googleurl/QUIC transport core subset supports C++17. Optional
# MoQT sources require C++20 because they use a defaulted operator<=>.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build static libraries with position-independent code so they can also be
# linked into shared libraries by downstream consumers.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Source roots shared by the CMake modules below.
set(GOOGLE_URL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/google_url")
set(QUICHE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/quiche")

# Make project-owned dependency and source modules available through include().
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# QUICHE and its dependencies use C++ exceptions. Make the MSVC behavior
# explicit instead of relying on the generator's implicit default.
if(MSVC)
  add_compile_options(/EHsc)
endif()

# Provide the Abseil targets used directly by QUICHE and by the local base
# library. Keeping this at the top level ensures all modules share one instance.
include(absl)

# Provide the shared fmt target before configuring the local base library.
include(fmt)

# -----------------------------------------------------------------------------
# Third-party dependencies built from source
# -----------------------------------------------------------------------------
# QUICHE uses BoringSSL-specific APIs such as openssl/aead.h and
# openssl/bytestring.h, so a generic OpenSSL implementation is not sufficient.
include(boringssl)

# Certificate compression in the QUIC crypto implementation requires Zlib.
include(zlib)

# Fetch protobuf 30.2 and build protoc together with the lite runtime used by
# QUICHE's generated transport messages.
include(protobuf)

# -----------------------------------------------------------------------------
# Local support libraries
# -----------------------------------------------------------------------------
# Build the local base library first because it also provides the shared Abseil
# and fmt targets. Tests and samples are deliberately disabled for this build.
# set(BASE_BUILD_TESTS OFF CACHE BOOL "Build base tests" FORCE)
# set(BASE_BUILD_SAMPLES OFF CACHE BOOL "Build base samples" FORCE)
# set(BASE_ABSL_EXTERNAL ON CACHE BOOL "Using Extern absl for base lib" FORCE)
# set(BASE_FMT_EXTERNAL ON CACHE BOOL "Using Extern fmt for base lib" FORCE)

# add_subdirectory(base)

# GURL is used by QUICHE URL, hostname and tool-facing APIs. The module builds
# only the production googleurl sources required by QUICHE.
include(google_url)

# -----------------------------------------------------------------------------
# Default QUIC transport library
# -----------------------------------------------------------------------------
# The module owns protobuf generation, source selection, dependencies and all
# platform-specific settings for the default QUIC transport target.
include(quiche_core)

# HTTP/3 and QPACK can be built independently. WebTransport and MoQT imply
# this dependency even when the standalone H3 feature switch is disabled.
option(GOOGLE_QUICHE_BUILD_H3
  "Build HTTP/3 and QPACK support" ON)

# WebTransport-over-HTTP/3 and its test server are optional because they pull
# in the substantially larger HTTP/3/QPACK and command-line tool stack.
option(GOOGLE_QUICHE_BUILD_WEBTRANSPORT
  "Build WebTransport support and the WebTransport test server" ON)

option(GOOGLE_QUICHE_BUILD_MOQT
  "Build MoQT support and the MoQT chat client/server demos" ON)

option(GOOGLE_QUICHE_BUILD_DEMOS
  "Build demo executables for enabled features" ON)

# MoQT is transported over WebTransport. Build the shared protocol layer when
# either public feature is requested, without implicitly enabling the separate
# WebTransport test-server demo.
if(GOOGLE_QUICHE_BUILD_H3 OR GOOGLE_QUICHE_BUILD_WEBTRANSPORT OR GOOGLE_QUICHE_BUILD_MOQT)
  include(quiche_h3)
endif()

if(GOOGLE_QUICHE_BUILD_WEBTRANSPORT OR GOOGLE_QUICHE_BUILD_MOQT)
  include(quiche_webtransport)
endif()

if(GOOGLE_QUICHE_BUILD_MOQT)
  include(quiche_moqt)
endif()
