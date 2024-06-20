aux_source_directory(. SRC_LIST)

#编译生成动态库ltmuduo
add_library(ltmuduo SHARED ${SRC_LIST})

#编译生成静态库ltmuduo
#add_library(ltmuduo STATIC ${SRC_LIST})