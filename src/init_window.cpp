#include "init_window.h"


GLFWwindow *window; // 声明全局窗口实例


void initWindow()
{
    // glfw 库初始化
    glfwInit();
    // 告诉 glfw 不要默认创建一个对应 OpenGL 上下文的窗口，因为我们正在使用的是vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // 在这里我们不允许在窗口创建后调整其大小（方便）
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // 使用 glfw 创建window
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
}


// 关闭窗口调用，释放内存
void windowCleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
