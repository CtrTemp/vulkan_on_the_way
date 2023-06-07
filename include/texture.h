#ifndef TEXTURE_H
#define TEXTURE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include "image_view.h"
#include "command_buffer.h"

extern VkImage textureImage;              // 声明 纹理贴图实例
extern VkDeviceMemory textureImageMemory; // 声明 纹理贴图内存
extern VkImageView textureImageView;      // 声明 纹理图的 ImageView 实例
extern VkSampler textureSampler;          // 声明 纹理图采样器实例

/**
 *  创建纹理贴图实例
 * */
void createTextureImage();


/**
 *  为纹理贴图创建配套的 ImageView
 * */
void createTextureImageView();

/**
 *  创建纹理采样器
 * */
void createTextureSampler();

/**
 *  注销纹理贴图相关的组件
 * */
void cleanupTextureRelated();

#endif