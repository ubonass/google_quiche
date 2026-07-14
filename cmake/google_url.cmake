include_guard(GLOBAL)

# googleurl uses Abseil's optional type in its public API. The base target is
# configured first and provides the shared Abseil dependency.
if(NOT TARGET absl::optional)
  message(FATAL_ERROR
    "google_url requires absl::optional; configure the base dependency first")
endif()

# Source manifest for the googleurl revision consumed by QUICHE M137.
# Keep this list in sync with google_url/base/BUILD and google_url/url/BUILD.
set(GOOGLE_URL_BASE_SOURCES
  ${GOOGLE_URL_ROOT}/base/debug/crash_logging.cc
  ${GOOGLE_URL_ROOT}/base/strings/string_util.cc
  ${GOOGLE_URL_ROOT}/base/strings/string_util_constants.cc
  ${GOOGLE_URL_ROOT}/base/strings/utf_string_conversion_utils.cc
  ${GOOGLE_URL_ROOT}/base/strings/utf_string_conversions.cc
  ${GOOGLE_URL_ROOT}/base/strings/utf_ostream_operators.cc
)

if(WIN32)
  list(APPEND GOOGLE_URL_BASE_SOURCES
    ${GOOGLE_URL_ROOT}/base/strings/string_util_win.cc
  )
endif()

set(GOOGLE_URL_SOURCES
  ${GOOGLE_URL_ROOT}/url/gurl.cc
  ${GOOGLE_URL_ROOT}/url/third_party/mozilla/url_parse.cc
  ${GOOGLE_URL_ROOT}/url/url_canon.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_etc.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_filesystemurl.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_fileurl.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_host.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_internal.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_ip.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_mailtourl.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_path.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_pathurl.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_query.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_relative.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_stdstring.cc
  ${GOOGLE_URL_ROOT}/url/url_canon_stdurl.cc
  ${GOOGLE_URL_ROOT}/url/url_constants.cc
  ${GOOGLE_URL_ROOT}/url/url_features.cc
  ${GOOGLE_URL_ROOT}/url/url_parse_file.cc
  ${GOOGLE_URL_ROOT}/url/url_util.cc
)

option(GOOGLE_URL_USE_SYSTEM_ICU "Use system ICU for IDN conversion" OFF)
if(GOOGLE_URL_USE_SYSTEM_ICU)
  list(APPEND GOOGLE_URL_SOURCES ${GOOGLE_URL_ROOT}/url/url_idna_icu.cc)
else()
  list(APPEND GOOGLE_URL_SOURCES ${GOOGLE_URL_ROOT}/url/url_idna_ascii_only.cc)
endif()

add_library(google_url STATIC
  ${GOOGLE_URL_BASE_SOURCES}
  ${GOOGLE_URL_SOURCES}
)
add_library(google_url::google_url ALIAS google_url)

target_compile_features(google_url PUBLIC cxx_std_17)
target_include_directories(google_url PUBLIC "${GOOGLE_URL_ROOT}")
target_link_libraries(google_url PUBLIC absl::optional)

# This upstream revision relies on Chromium's shared build prefix to provide
# the complete string and stream declarations used by utf_ostream_operators.
# Supply them at the target level so the vendored sources remain unmodified.
target_precompile_headers(google_url PRIVATE
  <string>
  <ostream>
)

if(NOT MSVC)
  target_compile_options(google_url PRIVATE -fno-strict-aliasing)
endif()

# WebTransport only needs GURL parsing and does not require ICU. Enable this
# option when non-ASCII international domain names must be canonicalized.
if(GOOGLE_URL_USE_SYSTEM_ICU)
  find_package(ICU REQUIRED COMPONENTS uc i18n)
  target_link_libraries(google_url PRIVATE ICU::uc ICU::i18n)
endif()

set_target_properties(google_url PROPERTIES FOLDER google_url)
