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

/**
 *  Brief Introduction：
 *
 *  vulkan明确指出，其API的设计理念是“最偶小驱动程序开销”。这导致默认情况下vulkan的API不会自动帮助检测
 * 程序中的错误（非常有限）。好处是你可以非常细化的去处理整个流程中的细节，但必须对自己所在做的事情非常明确，
 * 否则一些错误会直接导致程序崩溃而不是显式地报错。
 *
 *  为了处理这些可能发生的错误，并显式地给出对应的错误信息，vulkan提出“验证层”系统。它将作为整个程序中的
 * 一个可选组件，挂接在vulkan的函数中，提供错误检测输出等额外操作。
 *
 *  vulkan验证层可以概括为提供以下功能：
 *
 *  1、参数检测：检测输入函数的参数是否合法合规，是否在给定的范围内进行选择；
 *  2、跟踪对象/实例的创建与销毁，从而避免疏忽造成的内存泄漏问题；
 *  3、通过跟踪调用/启动线程的来源，来检测线程的安全性；（这个还不太明白）
 *  4、可以将所有的函数调用以及传递参数进行打印输出，从而监测；
 *  5、Tracing Vulkan calls for profiling and replaying；（这个还得不太明白）
 *
 *  在你的debug版本中可以自由使用验证层，并且可以与其他你自己手写/自定义的程序叠加使用。在最终的release
 * 版本中只需要简单地禁用验证层即可。
 *
 * */

// #define NDEBUG  // 设置宏决定是否禁用验证层

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
 *  启用validation layer对应的messager
 * */
void setupDebugMessenger(VkInstance instance);

/**
 *  为instance的createinfo结构体中添加关于validation layer的配置
 * */
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

/**
 *  使用验证层配置信息创建对应配置的验证层函数
 * */
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);


/**
 *  销毁validation layer对应的messager
 * */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

/**
 *  debug callback 函数
 * */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

#endif
