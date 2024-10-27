# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_HTTP_SRCS 变量
aux_source_directory(. DIR_HTTP_SRCS)
include_directories(../Config)
# 生成链接库
add_library(HTTP ${DIR_HTTP_SRCS})
target_link_libraries(HTTP PUBLIC Config)