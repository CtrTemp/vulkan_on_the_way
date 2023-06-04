#include "init_window.h"

GLFWwindow *window; // 声明全局窗口实例

/*
    第五步，显式处理“改变窗口大小”事件
    尽管很多驱动和程序平台会在调整窗口大小后自动触发 VK_ERROR_OUT_OF_DATE_KHR 事件，
但我们不能保证其在所有驱动和平台上都会发生。这样我们需要一块额外的程序代码来检测触发窗口
的resize事件。首先我们应该在成员变量中添加一个flag来指示这个事件的发生。
*/
bool framebufferResized = false; // 将其初始化为 false

void initWindow()
{
    // glfw 库初始化
    glfwInit();
    // 告诉 glfw 不要默认创建一个对应 OpenGL 上下文的窗口，因为我们正在使用的是vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // 在这里我们不允许在窗口创建后调整其大小（方便）
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // 使用 glfw 创建window
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);

    // GLFW 当检测到窗口变化时，会调用参数2传入的回调函数
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

/*
    在这个函数中，我们对 framebufferResized 在哪些情况下会进行改变进行判断。
注意，这步我们交给GLFW相关函数进行处理里。因为是GLFW为我们创建的窗口，所以窗口相关的活
动GLFW是一定检测的到的。
    当GLFW检测到当前窗口变化时，会调用以下这个回调函数。
*/
static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // 首先定位到当前的 app
    // 之后修改其内部与窗口变化相关的成员变量，将其置为true
    framebufferResized = true;
}

// 关闭窗口调用，释放内存
void windowCleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
