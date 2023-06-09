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

#include "./font/font_lishu_CN_base85.h"
#include "./font/font_lishu_CN_nocompress.h"
#include "./font/font_lishu_CN_nostatic.h"

int main()
{

    initWindow();

    initVulkan();

    imguiSetup();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
        processInput(window);
        prim_camera.UpdataCameraPosition();
    }

    cleanupVulkan();

    return 0;
}