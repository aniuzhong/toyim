add_custom_command(OUTPUT toyim.pb.cc toyim.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/toyim.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS toyim.proto
  VERBATIM )

set_source_files_properties(toyim.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR}/src/)

add_library(toyim_proto toyim.pb.cc)
target_link_libraries(toyim_proto protobuf pthread)

add_executable(im_server server.cc)
set_target_properties(im_server PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(im_server protobuf_codec toyim_proto toyim_orm toyim_hiredis)

add_executable(im_client client.cc)
set_target_properties(im_client PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(im_client protobuf_codec toyim_proto)

add_subdirectory(tests)