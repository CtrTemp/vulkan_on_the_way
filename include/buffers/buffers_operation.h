#ifndef BUFFERS_OPERATION_H
#define BUFFERS_OPERATION_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>


#include "physical_device_queue.h"
#include "logical_device_queue.h"

#include "command_buffer.h"




/**
 *  在device上创建一个特定功用的buffer，并为其分配合适类型的内存空间
 * */
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);


/**
 *  GPU上的buffer数据拷贝（device to device）
 * */
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


/**
 *  找出当前buffer最合适的分配内存类型
 * */
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


#endif