add_library(protobuf_codec codec.cc)
target_link_libraries(protobuf_codec protobuf muduo_net muduo_base pthread z)

