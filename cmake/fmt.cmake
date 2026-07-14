include(FetchContent)

set(FMT_DOC     OFF CACHE BOOL "" FORCE)
set(FMT_TEST    OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.2.1
  GIT_SHALLOW    TRUE
  # FIND_PACKAGE_ARGS 10.2 NAMES fmt  # use an already-provided fmt if version >= 10.2
  EXCLUDE_FROM_ALL                  # only compile targets that are actually linked
)

FetchContent_MakeAvailable(fmt)

# ---------------------------------------------------------------------------
# Group every fmt target into a VS solution sub-folder ("third_party/fmt").
# Only needed when fmt is built from source (FetchContent path).
# ---------------------------------------------------------------------------
if(IS_DIRECTORY "${fmt_SOURCE_DIR}")
  function(_fmt_collect_targets out_var dir)
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    set(_result)
    foreach(_sub IN LISTS _subdirs)
      _fmt_collect_targets(_sub_result "${_sub}")
      list(APPEND _result ${_sub_result})
    endforeach()
    get_property(_dir_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND _result ${_dir_targets})
    set(${out_var} ${_result} PARENT_SCOPE)
  endfunction()

  _fmt_collect_targets(_fmt_targets "${fmt_SOURCE_DIR}")
  foreach(_t IN LISTS _fmt_targets)
    set_target_properties("${_t}" PROPERTIES FOLDER "${PROJECT_NAME}/third_party/fmt")
  endforeach()
endif()
