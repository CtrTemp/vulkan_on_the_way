#ifndef VALIDATION_LAYER_H
#define VALIDATION_LAYER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// 容器
#include <string>
#include <vector>

#include <cstring>

// #define NDEBUG  // 是否禁用验证层



std::vector<const char *> getRequiredExtensions();
void setupDebugMessenger(VkInstance &instance);

// 默认开启DEBUG模式，使得验证层可以打印输出一些调试信息
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

extern const std::vector<const char *> validationLayers;
extern VkDebugUtilsMessengerEXT debugMessenger;


void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

bool checkValidationLayerSupport();
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);


void vkInstanceCleanUp();

#endif
