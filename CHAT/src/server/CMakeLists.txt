# # 设置需要编译的源文件列表
# set(SRC_LIST ./muduo_server.cpp)

# # 设置可执行文件存储路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 把.指定路径下的所有源文件名字放入变量名SRC_LIST里面
aux_source_directory(. SRC_LIST)

# 指定生成的可执行文件
add_executable(ChatServer ${SRC_LIST})

# 表示server这个目标程序需要链接 muduo_net muduo_base pthread这三个库文件
target_link_libraries(ChatServer muduo_net muduo_base pthread)