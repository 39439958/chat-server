aux_source_directory(. SRC_LIST)

# 指定生成的可执行文件
add_executable(ChatServer ${SRC_LIST})

# 指定可执行文件需要依赖的动态库
target_link_libraries(ChatServer muduo_net muduo_base pthread)
