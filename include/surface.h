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

/*
    Brief Introduction：

    Vulkan 是一个跨平台的API，它无法依靠自身单独与窗口系统直接交互。
    为了在Vulkan和窗口之间建立链接从而向屏幕展示Vulkan的结果，我们需要使用WSI（Window System Integration 窗口系统集成）扩展

    本章节中，我们将讨论并使用第一个 VK_KHR “界面”。它会公开一个VkSurfaceKHR对象，该对象表示要呈现渲染图像的抽象类型的界面。
    程序中将由我们使用GLFW打开窗口的支持。

    其实这也算vulkan的一个外设扩展(extension)，我们在使用扩展获取函数的时候其实已经将其囊括进来了，当时我们就require了两种扩展
    一个是glfw所需扩展，另一个是validation layer扩展。而现在我们要讨论的这个抽象类型的，渲染出图要被展示到的这个界面实例，刚好被
    囊括在glfw所需的扩展中。（倒也合情合理，本来我们就是要把渲染好的图片放到glfw创建的窗口中，这样看起来在图形image和窗口window
    之间应该还有一层抽象就是它这里所说的这个界面surface--VK_KHR_surface）

    如果你想做的是一个无窗口实时展示的应用，那么这步不是必须的
*/

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