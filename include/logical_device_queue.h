#ifndef LOGICAL_DEVICE_QUEUQ_H
#define LOGICAL_DEVICE_QUEUQ_H

// 做如下定义后，GLFW 会自动帮我们引入 vulkan
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <optional>
// 容器
#include <string>
#include <vector>
#include <set>

#include <cstring>

#include "vaildation_layer.h"
#include "physical_device_queue.h"

extern VkDevice device; // 逻辑设备，物理设备的映射

/**
 *  逻辑设备是物理设备的映射，
 * 
 *  1、一个物理设备将可以映射出多个逻辑设备（这里我们只映射一个逻辑设备）
 *  2、我们之后的操作都是基于逻辑设备的操作而非物理设备
 * */ 


/**
 *  创建逻辑设备实例
 * */ 
void createLogicalDevice();

/**
 *  销毁逻辑设备实例
 * */ 
void logicalDeviceCleanup();

#endif