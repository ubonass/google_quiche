include(FetchContent)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib
  GIT_TAG v1.3.1
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zlib)
add_library(zlib::zlib_a ALIAS zlibstatic)

# 让后续依赖（如 IXWebSocket）的 find_package(ZLIB) 直接命中我们构建的版本
if(NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
set(ZLIB_FOUND        TRUE        CACHE BOOL     "" FORCE)
set(ZLIB_LIBRARY      zlibstatic  CACHE STRING   "" FORCE)
set(ZLIB_LIBRARIES    zlibstatic  CACHE STRING   "" FORCE)
set(ZLIB_INCLUDE_DIR  "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE STRING "" FORCE)
set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE STRING "" FORCE)

# ---------------------------------------------------------------------------
# Group every zlib target into a VS solution sub-folder.
# Only needed when zlib is built from source (FetchContent path).
# ---------------------------------------------------------------------------
if(IS_DIRECTORY "${zlib_SOURCE_DIR}")
  function(_zlib_collect_targets out_var dir)
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    set(_result)
    foreach(_sub IN LISTS _subdirs)
      _zlib_collect_targets(_sub_result "${_sub}")
      list(APPEND _result ${_sub_result})
    endforeach()
    get_property(_dir_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND _result ${_dir_targets})
    set(${out_var} ${_result} PARENT_SCOPE)
  endfunction()

  _zlib_collect_targets(_zlib_targets "${zlib_SOURCE_DIR}")
  foreach(_t IN LISTS _zlib_targets)
    set_target_properties("${_t}" PROPERTIES FOLDER "${PROJECT_NAME}/third_party/zlib")
  endforeach()
endif()
