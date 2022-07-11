# test codec
add_custom_command(OUTPUT test.pb.cc test.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/test.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS test.proto
  VERBATIM )

set_source_files_properties(test.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR}/src/protorpc/tests/proto_test)

add_library(test_proto test.pb.cc)
target_link_libraries(test_proto protobuf pthread)

add_executable(codec_test codec_test.cc)
set_target_properties(codec_test PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(codec_test protobuf_codec test_proto)

# test dispatcher
add_custom_command(OUTPUT query.pb.cc query.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/query.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS query.proto
  VERBATIM )

set_source_files_properties(query.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR}/src/protorpc/tests/proto_test)

add_library(query_proto query.pb.cc)
target_link_libraries(query_proto protobuf pthread)

add_executable(protobuf_server server.cc)
set_target_properties(protobuf_server PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_server protobuf_codec query_proto)

add_executable(protobuf_client client.cc)
set_target_properties(protobuf_client PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_client protobuf_codec query_proto)
