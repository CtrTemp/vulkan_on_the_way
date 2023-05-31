cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(vk_demo LANGUAGES CXX)

# 注意这里不写 ./ 不会默认添加根目录作为 include 的目录
include_directories(./ ./include)

# 环境变量的引入
# message(STATUS "Compile sources into a library? $ENV{PATH}")

# add_executable(binary 
#     main.cpp
# )

aux_source_directory(./ MAIN_SRC_LIST)
aux_source_directory(./src MAIN_SRC_LIST)


add_executable(${PROJECT_NAME} ${MAIN_SRC_LIST})

# 对单一可执行文件生效
# 通过绝对路径指定 GLFW lib 文件 （成功！！！）
# TARGET_LINK_LIBRARIES(binary 
#     /home/ctrtemp/Documents/Libs/Cpp/OpenGL_Related/GLFW/installed/usr/local/lib/libglfw.so
#     /home/ctrtemp/Downloads/Lib/CppLib/Vulkan/1.3.224.1/x86_64/lib/libvulkan.so
#     )

# 通过指定相对路径 （因为按理说我修改了环境变量，指定了LIBRARY_PATH 和 LD_LIBRARY_PATH就不应该无法找到对应库文件
# 不应该使用绝对路径的相关方法 （成功！)
# 这里主要应该注意要写文件后缀 .so （这里没有验证.a）
TARGET_LINK_LIBRARIES(${PROJECT_NAME} libvulkan.so libglfw.so)

# 对所有 target 统一指定 且要添加到 add_executable 前面
# LINK_LIBRARIES()
