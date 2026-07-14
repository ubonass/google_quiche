include_guard(GLOBAL)

if(TARGET protobuf::protoc AND TARGET protobuf::libprotobuf-lite)
  return()
endif()

include(FetchContent)

# Build only the protobuf libraries and protoc required to generate QUICHE's
# three lite-runtime messages. Tests, examples and install rules stay disabled.
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
set(protobuf_BUILD_LIBPROTOC ON CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
# Keep protoc on the same /MT or /MD model as the project and Abseil for
# MSVC-ABI builds. Other platforms use their native runtime configuration.
if(MSVC)
  set(protobuf_MSVC_STATIC_RUNTIME
      ${GOOGLE_QUICHE_MSVC_STATIC_RUNTIME} CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
  protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG v30.2
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
)

# Group every target created by protobuf under a single solution folder in
# Visual Studio. Preserve the caller's folder so subsequent project targets do
# not inherit protobuf's third-party location.
set(_quiche_protobuf_saved_cmake_folder "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${PROJECT_NAME}/third_party/protobuf")
FetchContent_MakeAvailable(protobuf)
set(CMAKE_FOLDER "${_quiche_protobuf_saved_cmake_folder}")
unset(_quiche_protobuf_saved_cmake_folder)

if(NOT TARGET protobuf::protoc)
  message(FATAL_ERROR "The fetched protobuf project did not provide protobuf::protoc")
endif()

if(NOT TARGET protobuf::libprotobuf-lite)
  message(FATAL_ERROR
    "The fetched protobuf project did not provide protobuf::libprotobuf-lite")
endif()
