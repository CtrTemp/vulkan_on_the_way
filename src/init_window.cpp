#include "init_window.h"

// 全局窗口实例
GLFWwindow *window;

// 窗口大小 resize flag，由回调函数触发更改
bool framebufferResized = false;

/**
 *  创建窗口实例
 * */ 
void initWindow()
{
    glfwInit();
    // 告诉 glfw 不要默认创建一个对应 OpenGL 上下文的窗口，因为我们正在使用的是vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // 是否禁用窗口大小 resize
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);

    // 注册回调函数：window resize 
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

/**
 *  window resize callback function
*/
static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // 首先定位到当前的 app
    // 之后修改其内部与窗口变化相关的成员变量，将其置为true
    framebufferResized = true;
}

/**
 *  注销窗口实例
 * */ 
void windowCleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
