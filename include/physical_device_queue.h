#ifndef DEVICE_QUEUE_H
#define DEVICE_QUEUE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <optional>
// 容器
#include <string>
#include <vector>

#include <cstring>

#include "surface.h"


extern VkPhysicalDevice physicalDevice;

// struct QueueFamilyIndices
// {
//     std::optional<uint32_t> graphicsFamily;

//     bool isComplete()
//     {
//         return graphicsFamily.has_value();
//     }
// };

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};




void pickPhysicalDevice(VkInstance &instance);
bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);


#endif
