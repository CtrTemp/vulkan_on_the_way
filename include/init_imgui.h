#ifndef INIT_IMGUI_H
#define INIT_IMGUI_H

#include "imgui/imgui.h"
#include "imgui/imconfig.h"
#include "imgui/imgui_internal.h"
#include "imgui/imstb_rectpack.h"
#include "imgui/imstb_textedit.h"
#include "imgui/imstb_truetype.h"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"


#include "init_window.h"
#include "vk_instance.h"
#include "surface.h"
#include "physical_device_queue.h"
#include "logical_device_queue.h"
#include "swapchain.h"
#include "render_passes.h"
#include "graphic_pipeline.h"


void init_imgui_frame();


#endif