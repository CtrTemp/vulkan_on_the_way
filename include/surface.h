#ifndef SURFACE_H
#define SURFACE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>

#include <set>

#include "init_window.h"
#include "vk_instance.h"

extern VkSurfaceKHR surface; // surface 全局界面实例

extern VkQueue graphicsQueue; // 用于处理 “图形绘制指令” 的队列
extern VkQueue presentQueue;  // 用于处理 “图形展示指令” 的队列

/**
 *  创建surface实例桥接窗口和渲染出的图像
 * */
void createSurface();

/**
 *  销毁surface界面实例
 * */
void surfaceCleanUp();

#endif