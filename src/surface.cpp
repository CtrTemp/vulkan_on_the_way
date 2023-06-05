#include "surface.h"


VkSurfaceKHR surface; // surface 界面实例

VkQueue graphicsQueue; // 用于处理 “图形绘制指令” 的队列（将在之后的 logical device 部分进行初始化）
VkQueue presentQueue;  // 用于处理 “图形展示指令” 的队列（将在之后的 logical device 部分进行初始化）

/**
 *  创建界面实例
 * */
void createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

/**
 *  注销界面实例
 * */
void surfaceCleanUp()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}
