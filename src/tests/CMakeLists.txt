add_executable(login_loadtest login_loadtest.cc)
set_target_properties(login_loadtest PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(login_loadtest protobuf_codec toyim_proto)