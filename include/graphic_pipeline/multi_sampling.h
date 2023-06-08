#ifndef MULTI_SAMPLING_H
#define MULTI_SAMPLING_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

#include "physical_device_queue.h"
#include "swapchain.h"
#include "image_view.h"

extern VkSampleCountFlagBits msaaSamples;

extern VkImage colorImage;
extern VkDeviceMemory colorImageMemory;
extern VkImageView colorImageView;


/**
 *  配置多重采样，进行图像抗锯齿、提高生成图像质量的之而有效的手段
 * */
void configure_multi_sampling(VkPipelineMultisampleStateCreateInfo &multisampling);

VkSampleCountFlagBits getMaxUsableSampleCount();

void createColorResources();

void cleanupMultiSampleColorResource();

#endif