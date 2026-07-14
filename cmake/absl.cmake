include(FetchContent)

# Propagate the project's C++ standard to absl to avoid ABI mismatch.
set(ABSL_PROPAGATE_CXX_STD ON  CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL     OFF CACHE BOOL "" FORCE)
# Abseil overrides CMAKE_MSVC_RUNTIME_LIBRARY on MSVC-ABI builds, so propagate
# the project-wide /MT versus /MD choice only on that platform.
if(MSVC)
  set(ABSL_MSVC_STATIC_RUNTIME
      ${GOOGLE_QUICHE_MSVC_STATIC_RUNTIME} CACHE BOOL "" FORCE)
endif()
# Disable all tests and benchmarks — prevents absl from downloading GoogleTest.
set(ABSL_BUILD_TESTING      OFF CACHE BOOL "" FORCE)
set(ABSL_BUILD_TEST_HELPERS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)

# Abseil >= 20211102 requires GCC 7+.
# Fall back to the last LTS that still accepts GCC 6 when cross-compiling
# with an older toolchain (e.g. OpenWrt GCC 6.4.1).
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
   CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
  set(_absl_tag "20220623.0")
  message(STATUS "Abseil: GCC ${CMAKE_CXX_COMPILER_VERSION} < 7, using tag ${_absl_tag}")
else()
  # set(_absl_tag "20240722.0")
  # set(_absl_tag "20250512.0")
  set(_absl_tag "lts_2025_05_12")
endif()

FetchContent_Declare(
  abseil-cpp
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        ${_absl_tag}
  GIT_SHALLOW    TRUE
  # FIND_PACKAGE_ARGS NAMES absl  # use an already-provided absl if available
  EXCLUDE_FROM_ALL              # only compile targets that are actually linked
                                # (requires CMake >= 3.28)
)

FetchContent_MakeAvailable(abseil-cpp)

# ---------------------------------------------------------------------------
# Group every absl target into a VS solution sub-folder ("third_party/absl")
# so they don't clutter the top-level project list.
# Only needed when absl is built from source (FetchContent path).
# ---------------------------------------------------------------------------
if(IS_DIRECTORY "${abseil-cpp_SOURCE_DIR}")
  # Recursively collect all CMake targets created under the absl source tree.
  function(_absl_collect_targets out_var dir)
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    set(_result)
    foreach(_sub IN LISTS _subdirs)
      _absl_collect_targets(_sub_result "${_sub}")
      list(APPEND _result ${_sub_result})
    endforeach()
    get_property(_dir_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND _result ${_dir_targets})
    set(${out_var} ${_result} PARENT_SCOPE)
  endfunction()

  _absl_collect_targets(_absl_targets "${abseil-cpp_SOURCE_DIR}")
  foreach(_t IN LISTS _absl_targets)
    set_target_properties("${_t}" PROPERTIES FOLDER "${PROJECT_NAME}/third_party/absl")
  endforeach()
endif()
