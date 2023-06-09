#include "init_window.h"



// 全局窗口实例
GLFWwindow *window;

// 窗口大小 resize flag，由回调函数触发更改
bool framebufferResized = false;

// 鼠标移动位置记录
float lastX = WIDTH / 2, lastY = HEIGHT / 2;
// 缩放视野
float fov = 45.0f;
bool firstMouse = true;

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

    // 开启后会隐藏光标，，，使用win+D撤出，并退出程序
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetWindowSizeCallback(window, HelloTriangleApplication::onWindowResized);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPos(window, 400, 300);
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
 *  mouse movement callback function
 * */
void mouse_callback(GLFWwindow *window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    float deltaX, deltaY;
    deltaX = xPos - lastX;
    deltaY = yPos - lastY;
    lastX = xPos;
    lastY = yPos;
    prim_camera.ProcessMouseMovement(deltaX, deltaY);
    std::cout << deltaX << ";" << deltaY << std::endl;
}

/**
 *  keyboard control function
 * */
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // esc退出程序
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        prim_camera.speedZ = MOVING_SPEED;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        prim_camera.speedZ = -MOVING_SPEED;
    }
    else
    {
        prim_camera.speedZ = 0.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        prim_camera.speedX = -MOVING_SPEED;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        prim_camera.speedX = MOVING_SPEED;
    }
    else
    {
        prim_camera.speedX = 0.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        prim_camera.speedY = MOVING_SPEED;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        prim_camera.speedY = -MOVING_SPEED;
    }
    else
    {
        prim_camera.speedY = 0.0f;
    }
}

/**
 *  mouse scroll callback function
 * */
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

/**
 *  注销窗口实例
 * */
void windowCleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
