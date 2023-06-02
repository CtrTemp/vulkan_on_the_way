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

// surface 类加入
extern VkSurfaceKHR surface;

extern VkQueue graphicsQueue;   // 用于处理 “图形绘制指令” 的队列
extern VkQueue presentQueue;    // 用于处理 “图形展示指令” 的队列


void createSurface(VkInstance instance, GLFWwindow *window);
void surfaceCleanUp(VkInstance instance);

#endif