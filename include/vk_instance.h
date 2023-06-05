#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// 容器
#include <string>
#include <vector>

#include "vaildation_layer.h"

// 声明全局vulkan实例
extern VkInstance instance; 


/**
 *  查看并打印输出当前显卡支持的扩展
 * */
void checkExtension(); 


/**
 *  创建 vulkan 实例
 * */
void createInstance(); // 创建vulkan实例


/**
 *  销毁 vulkan 实例
 * */
void vkInstanceCleanUp(); // 销毁vulkan实例

#endif
