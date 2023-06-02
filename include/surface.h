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
// 显示队列
extern VkQueue presentQueue;


void createSurface(VkInstance instance, GLFWwindow *window);
void surfaceCleanUp(VkInstance instance);

#endif