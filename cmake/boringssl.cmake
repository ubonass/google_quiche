include(FetchContent)

FetchContent_Declare(
  boringssl
  GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
  # Match the BoringSSL release declared by QUICHE M137's MODULE.bazel.
  GIT_TAG 0.20250212.0
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
)
set(BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)
set(BORINGSSL_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(OPENSSL_NO_ASM             ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(boringssl)

# 让后续的 find_package(OpenSSL) 直接命中 BoringSSL
# BoringSSL 构建后导出 ssl 和 crypto 两个静态库 target
set(OPENSSL_FOUND       TRUE                        CACHE BOOL   "" FORCE)
set(OPENSSL_VERSION     "1.1.0"                     CACHE STRING "" FORCE)
set(OPENSSL_INCLUDE_DIR "${boringssl_SOURCE_DIR}/include" CACHE PATH "" FORCE)
set(OPENSSL_LIBRARIES   "ssl;crypto"                CACHE STRING "" FORCE)
set(OPENSSL_SSL_LIBRARY     ssl                     CACHE STRING "" FORCE)
set(OPENSSL_CRYPTO_LIBRARY  crypto                  CACHE STRING "" FORCE)

if(NOT TARGET OpenSSL::SSL)
  add_library(OpenSSL::SSL    ALIAS ssl)
endif()
if(NOT TARGET OpenSSL::Crypto)
  add_library(OpenSSL::Crypto ALIAS crypto)
endif()

# ---------------------------------------------------------------------------
# Group every boringssl target into a VS solution sub-folder.
# Only needed when boringssl is built from source (FetchContent path).
# ---------------------------------------------------------------------------
if(IS_DIRECTORY "${boringssl_SOURCE_DIR}")
  function(_boringssl_collect_targets out_var dir)
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    set(_result)
    foreach(_sub IN LISTS _subdirs)
      _boringssl_collect_targets(_sub_result "${_sub}")
      list(APPEND _result ${_sub_result})
    endforeach()
    get_property(_dir_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND _result ${_dir_targets})
    set(${out_var} ${_result} PARENT_SCOPE)
  endfunction()

  _boringssl_collect_targets(_boringssl_targets "${boringssl_SOURCE_DIR}")
  foreach(_t IN LISTS _boringssl_targets)
    set_target_properties("${_t}" PROPERTIES FOLDER "${PROJECT_NAME}/third_party/boringssl")
  endforeach()
endif()
