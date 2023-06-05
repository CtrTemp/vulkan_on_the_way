#ifndef INIT_VK_H
#define INIT_VK_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

// 容器
#include <string>
#include <vector>

#include "init_window.h"
#include "vk_instance.h"
#include "vaildation_layer.h"
#include "surface.h"
#include "physical_device_queue.h"
#include "logical_device_queue.h"
#include "swapchain.h"
#include "image_view.h"
#include "graphic_pipeline.h"
#include "render_passes.h"

#include "frame_buffer.h"
#include "command_buffer.h"
#include "render_loop.h"

#include "vertex_buffer.h"

/**
 *  当前vulkan图形工程总体的初始化配置，对一些渲染管线中必要的实例对象的创建
 * */
void initVulkan();



/**
 *  实例对象的析构销毁，基本遵循先创建的后销毁，后创建的先销毁
 * */
void cleanupVulkan(); // 对vulkan创建的对象进行统一销毁

#endif
