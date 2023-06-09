#ifndef INIT_IMGUI_H
#define INIT_IMGUI_H

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort
#include <iostream> // cout
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "init_window.h"
#include "vk_instance.h"
#include "surface.h"
#include "physical_device_queue.h"
#include "logical_device_queue.h"
#include "swapchain.h"
#include "render_passes.h"
#include "graphic_pipeline.h"
#include "uniform_buffer.h"
#include "command_buffer.h"

// void init_imgui_frame();

void imguiSetup();
static void SetupVulkan(const char **extensions, uint32_t extensions_count);


#endif