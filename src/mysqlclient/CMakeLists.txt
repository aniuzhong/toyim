set(toyim_mysqlclient_SRCS connection.cc connection_pool.cc)
add_library(toyim_mysqlclient ${toyim_mysqlclient_SRCS})
set_target_properties(toyim_mysqlclient PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(toyim_mysqlclient mysqlclient muduo_base pthread)

add_subdirectory(tests)