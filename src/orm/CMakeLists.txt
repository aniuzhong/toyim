set(toyim_orm_SRCS user_model.cc group_model.cc friend_model.cc offline_message_model.cc)
add_library(toyim_orm ${toyim_orm_SRCS})
set_target_properties(toyim_orm PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(toyim_orm toyim_mysqlclient)

add_subdirectory(tests)