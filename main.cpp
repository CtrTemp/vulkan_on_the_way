#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "init_window.h"
#include "init_vk.h"
#include "render_passes.h"
#include "init_imgui.h"

int main()
{

    initWindow();

    initVulkan();

    init_imgui_frame();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
    }

    cleanupVulkan();

    return 0;
}