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

// 默认开启DEBUG模式，使得验证层可以打印输出一些调试信息
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// 实例所需Layer扩展列表（这里主要对是否支持validation layer进行验证）
extern const std::vector<const char *> validationLayers;
// 用于调试信息输出的回调函数也需要我们手动创建，而且类似的回调我们想创建多少个就创建多少个
extern VkDebugUtilsMessengerEXT debugMessenger;

/**
 *  获取vulkan实例所需的扩展
 * */
std::vector<const char *> getRequiredExtensions();

/**
 *  查看当前layer扩展中是否支持validation layer
 * */
bool checkValidationLayerSupport();

/**
 *  为instance的createinfo结构体中添加关于validation layer的配置
 * */
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

/**
 *  启用validation layer对应的messager
 * */
void setupDebugMessenger(VkInstance instance);

/**
 *  销毁validation layer对应的messager
 * */
void vkInstanceCleanUp();
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

#endif
