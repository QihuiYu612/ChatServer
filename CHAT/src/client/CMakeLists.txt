

# add_executable(client clientMain.cpp)
# target_link_libraries(client pthread)

# add_executable(login_test test_login.cpp)
# target_link_libraries(login_test pthread)

# aux_source_directory(. SRC_LIST)

# add_executable(ChatClient ${SRC_LIST})
# target_link_libraries(ChatClient pthread)



add_executable(client clientMain.cpp)
# add_executable(clientmain main.cpp)
target_link_libraries(client pthread)
# target_link_libraries(clientmain pthread)

# add_executable(login_test test_login.cpp)
# target_link_libraries(login_test pthread)