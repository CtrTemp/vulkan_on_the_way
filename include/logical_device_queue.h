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

extern VkDevice device;
extern VkQueue graphicsQueue;


void createLogicalDevice();
void logicalDeviceCleanup();



#endif