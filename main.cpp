#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "init_window.h"
#include "init_vk.h"
#include "vaildation_layer.h"

int main()
{

    initWindow();

    initVulkan();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }


    cleanupVulkan();

    return 0;
}