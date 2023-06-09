#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "interaction/camera.h"

#define WIDTH 800
#define HEIGHT 600

#define MOVING_SPEED    25    

// 声明全局窗口实例
extern GLFWwindow *window;

// 声明窗口 resize 标志位
extern bool framebufferResized;

// 鼠标移动位置记录
extern float lastX;
// 缩放视野
extern float fov;
extern bool firstMouse;

/**
 *  创建窗口实例
 * */
void initWindow();

/**
 *  注销窗口实例
 * */
void windowCleanup();

/**
 *  window resize callback function
 */
static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

/**
 *  mouse movement callback function
 * */
void mouse_callback(GLFWwindow *window, double xPos, double yPos);

/**
 *  mouse scroll callback function
 * */
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);


/**
 *  keyboard control function
 * */
void processInput(GLFWwindow *window);

#endif
