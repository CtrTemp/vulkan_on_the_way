#include "render_loop.h"

std::vector<VkSemaphore> imageAvailableSemaphores; // 流程控制信号1：用来指示图像已经从 swapchain 中获取到，准备渲染
std::vector<VkSemaphore> renderFinishedSemaphores; // 流程控制信号2：用来指示图像渲染已经完成并可以进行展示
std::vector<VkFence> inFlightFences;               // 流程控制信号3：用于确保每帧只渲染一次，在渲染完成前阻塞后续的操作

uint32_t currentFrame = 0; // 当前帧 index

/**
 *  主渲染函数 Render Loop
 * */
void drawFrame()
{
    std::cout << "start to draw" << std::endl;
    // 开启 CPU 端计时器
    clock_t begin, end;
    begin = clock();

    // 阻塞 CPU/host 等待上一帧已经送入屏幕展示
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // 当前image索引，
    // 注意这里不是CPU领先GPU提交任务的index，而是我们设置的swap chain中最多image个数对应的图像索引。
    uint32_t imageIndex;

    // 从交换链中获取图像索引，如果成功则自动置位 imageAvailableSemaphores 信号灯从而打开后续的GPU阻塞任务提交任务
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    /**
     *  判断当前的window是否被resize过，如果已经被resize，则后续渲染与呈现是不兼容的，需要我们用新的图片大小重建
     * 交换链，且本帧直接跳过，不进行渲染：
     *
     *      VK_ERROR_OUT_OF_DATE_KHR：表示交换链已经与现有界面不兼容，无法再用于渲染，这基本上是窗口大小调整
     * 后被触发。
     *      VK_SUBOPTIMAL_KHR：交换链仍可以成功地将结果呈现到界面，但界面属性不再完全匹配。（但是在这种情况下也要
     * 被程序判定为 run_time_error 并抛出错误么？）
     * */
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // 更新uniform buffer，通过对MVP变换阵的赋值，达到让场景中物体“动起来”的效果
    updateUniformBuffer(currentFrame);

    // fences 不同于 semaphore，它需要我们进行手动重置，否则下一帧会卡住
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // 重置并填充command buffer，用于提交到 graphic queue 对场景中的物体进行渲染
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex, currentFrame);

    // 向“图形渲染指令集队列”提交指令集合
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]}; // 通过哪个semaphore控制“放行”
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;           // 等几个信号灯
    submitInfo.pWaitSemaphores = waitSemaphores; // 等哪个信号灯（信号灯序列，因为可能要等待多个）
    submitInfo.pWaitDstStageMask = waitStages;   // 我们希望在哪个状态进入等待，这里配置为着色阶段
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    // 控流用的信号灯，当前执行完成后，要置位哪个信号灯？也就是说，当前执行完成后，会通过这个信号灯放行哪些后续的操作。
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 等待渲染完成后，从交换链中取出图像进行展示
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    /**
     *  发出一个交换链向屏幕提交图像并进行显示的请求 注意这里的提交的指令队列是 presentQueue 而非之前的 graphicsQueue。
     *  由于只有一个命令，可能不需要再进行比较繁琐的command buffer填充。
     * */
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    // 如果你是在渲染过程结束之前对 window 进行了 resize，也需要重建交换链，但不会提前返回
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // 更新当前帧的索引
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    // 当前帧渲染计时结束--CPU时间，不过由于有 fence 对CPU的任务提交进行阻塞，所以这里得到的渲染时间应该也是准确的
    end = clock();
    std::cout << "time cost = " << end - begin << "ms" << std::endl;
}

/**
 *  配置流程控制组件 semaphores 和 fences
 * */
void createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    // 可见目前版本的创建无论是semaphores还是fences都并不需要什么额外的配置

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // 创建fences时就发出一个“signaled”信号 否则默认状态是unsignaled会卡在最开始状态
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // render loop 中每个控件都要创建
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

/**
 *  注销流程控制组件 semaphores 和 fences
 * */
void cleanupRenderLoopRelated()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
}

/**
 *  交换链重建
 *  （目前一般是由于window resize后swap chain不兼容导致的）
 * */
void recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    /**
     *  处理窗口最小化时的情况：
     *  暂定当前的渲染，直到窗口恢复正常大小，这里借助GLFW的glfwWaitEvents()来实现暂停
     * */
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    // 等待GPU已经完成了当前命令缓冲区的任务执行。
    vkDeviceWaitIdle(device);

    /**
     *  重建前先要重置原来的swap chain
     * */
    loopCleanupSwapChain();

    // 而后重新执行以下三个“重建交换链”相关的函数
    /*
        注意，在这里的 createSwapChain() 函数中我们重新根据窗口分辨率重置了交换链中图像的正确大小，
    具体是在其内部的 chooseSwapExtent() 函数实现的。
    */
    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    /*
        以上的方法虽然可以正确的重建交换链，但该方法的缺点是，我们不得不在需要创建新的交换链时停掉所有正在
    运行的渲染。如果想要在运行时更改交换链且不影响旧的渲染进行，可以考虑使用将之前的交换链传递到
    VkSwapchainCreateInfoKHR 的 oldSwapChain 字段上，并在使用完当前的旧交换链后立刻销毁它。
    */
}

/**
 * 渲染循环中，在重建交换链前对交换链的清理
 * */
void loopCleanupSwapChain()
{
    // 清除swapchain中的framebuffer
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    // 清除swapchain中的ImageView
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
