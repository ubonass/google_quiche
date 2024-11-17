cmake_minimum_required(VERSION 3.20)

if (NOT QUICHE_ROOT)
  message(SEND_ERROR "not define QUICHE_ROOT" )
endif()

if (MSVC)
  set(PROTOBUF_PROTOC_EXECUTABLE protoc.exe)
else()
  set(PROTOBUF_PROTOC_EXECUTABLE protoc)
endif()

#seting output target path
set(GENERATE_PROTO_TARGET_DIR ${CMAKE_BINARY_DIR}/quiche/quic/core/proto)

#create target dir if not exists..
if(NOT EXISTS ${GENERATE_PROTO_TARGET_DIR} OR NOT IS_DIRECTORY ${GENERATE_PROTO_TARGET_DIR})
  file(MAKE_DIRECTORY ${GENERATE_PROTO_TARGET_DIR})
endif()

#Setting protoc source file path
set(PROTO_SOURCE_DIR ${QUICHE_ROOT}/quiche/quic/core/proto)

#Get the proto file that needs to be compiled
file(GLOB_RECURSE quiche_proto_files ${PROTO_SOURCE_DIR}/*.proto)

set(PROTO_FLAGS
  "-I${PROTO_SOURCE_DIR}"
  "-I${QUICHE_ROOT}"
  "--cpp_out=${GENERATE_PROTO_TARGET_DIR}"
)

foreach(proto_file ${quiche_proto_files})
  
  get_filename_component(FIL_WE ${proto_file} NAME_WE)

  message(proto_file = "${proto_file}")
  
  list(APPEND quiche_proto_srcs ${GENERATE_PROTO_TARGET_DIR}/${FIL_WE}.pb.cc)
  list(APPEND quiche_proto_hdrs ${GENERATE_PROTO_TARGET_DIR}/${FIL_WE}.pb.h)

  message(quiche_proto_srcs = "${GENERATE_PROTO_TARGET_DIR}/${FIL_WE}.pb.cc")
  message(quiche_proto_hdrs = "${GENERATE_PROTO_TARGET_DIR}/${FIL_WE}.pb.h")
  
  # cutom command
  add_custom_command(
      OUTPUT ${GENERATE_PROTO_TARGET_DIR}/${FIL_WE}.pb.cc
              ${GENERATE_PROTO_TARGET_DIR}/${FIL_WE}.pb.h
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS} ${proto_file}
      DEPENDS ${proto_file}
      COMMENT Running C++ protocol buffer compiler on ${proto_file}
      VERBATIM
  )
endforeach()

# Set the file attribute to GENERATED
set_source_files_properties(
  ${quiche_proto_srcs} ${quiche_proto_hdrs} PROPERTIES GENERATED TRUE
)

# Adding a custom target
add_custom_target(generate_proto ALL
  DEPENDS ${quiche_proto_srcs} ${quiche_proto_hdrs}
  COMMENT generate proto target
  VERBATIM
)
