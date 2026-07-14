include_guard(GLOBAL)

add_library(quiche_h3 STATIC ${quiche_h3_sources})
add_library(quiche::h3 ALIAS quiche_h3)

target_compile_features(quiche_h3 PUBLIC cxx_std_17)
target_link_libraries(quiche_h3 PUBLIC quiche::core)

# Balsa uses the POSIX ssize_t name directly. The Windows SDK exposes the
# equivalent SSIZE_T type from BaseTsd.h; map the name only while compiling the
# affected source so other sources retain their own ssize_t declarations. Both
# cl.exe and ClangCL accept the /FI forced-include option.
if(MSVC)
  set_source_files_properties(
    ${QUICHE_ROOT}/quiche/balsa/balsa_headers.cc
    PROPERTIES
      COMPILE_OPTIONS "/FIBaseTsd.h"
      COMPILE_DEFINITIONS "ssize_t=SSIZE_T"
  )
endif()

set_target_properties(quiche_h3 PROPERTIES FOLDER ${PROJECT_NAME})
