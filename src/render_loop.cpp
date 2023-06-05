#include "render_loop.h"

/*
    第五步，来真的了！！
    现在我们要先在成员变量中添加以上所需的控流操作原语～信号灯Semphores和围栏Fences
    第一个信号灯 imageAvailableSemaphore 用来指示图像已经从 swapchain 中获取到，准备渲染
    第二个信号灯 renderFinishedSemaphore 用来表示渲染已经完成并可以进行演示
    围栏 inFlightFence 用于确保每帧只渲染一次
*/
// VkSemaphore imageAvailableSemaphore;
// VkSemaphore renderFinishedSemaphore;
// VkFence inFlightFence;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;

// 用于指示当前帧
uint32_t currentFrame = 0;

/**
 *  完整的renderloop，在main函数的主循环中执行
 * */
// std::cout << "111111111111111111111111111111111" << std::endl;

void drawFrame()
{

    clock_t begin, end;
    begin = clock();
    // std::cout << "start to render!" << std::endl;
    /*
        第七步，终于，在完成控流原语的介绍和配置后，我们再次回到了主循环中的函数，在此我们将基本完全按照
    以上本章Introduction部分内容的顺序，书写这部分核心代码。
    */

    /*
        vkWaitForFences() 函数参数详解：
        参数1：～
        参数2：围栏数量
        参数3：要等待的围栏是哪个围栏
        参数4：是否要等待所有围栏生效后再继续执行（因为现在只定义了一个围栏成员函数，所以这个无关紧要）
        参数5：超时时间，这里设为64位int最大值，从而禁用了超时事件
    */
    // 阻塞 CPU/host 等待上一帧展示完成
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    /*
        死锁问题的发现：
            目前如果运行代码，可能会遭遇“死锁”问题（程序被卡住无法继续向下运行）。调试代码我们发现，程序会在以上
        的 vkWaitForFences 进入等待，且一直无法通过。这是因为 vkAcquireNextImageKHR 返回
        VK_ERROR_OUT_OF_DATE_KHR 时，我们重新创建了交换链，然后从drawFrame()函数返回。但在这之前，当前帧的
        围栏是等待重置的状态。由于我们立即返回了，还没有手动重置当前帧的围栏，所以围栏的signaled信号永远无法到来，
        于是程序进入了“死锁”状态，一直等待这个不会到来的signaled信号。
    */
    // // 上一帧展示完成后，需要我们手动重置 fence 状态，以便再下一帧继续造成阻塞
    // vkResetFences(device, 1, &inFlightFences[currentFrame]);

    /*
        当我们有了重新创建交换链的函数，那么现在我们应该确定何时触发这个函数的执行。
    */

    uint32_t imageIndex;
    /*
        vkAcquireNextImageKHR() 函数参数详解：
        参数1：指定设备
        参数2：指定交换链
        参数3：超时时间，这里同样通过给一个极大值禁用超时事件（单位ns）
        参数4：指定信号灯，注意当前函数执行完后，这个的信号灯会被置为signaled
        参数5：指定一个fences而非semaphores！用于阻塞当前函数的执行
        参数6：交换链中的image索引映射
    */
    /*
        从交换链中获取图像，这里的图像是未被渲染的图像，我们拿到它作为当前帧即将操作的对象。
    */
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    /*
        以下的结果使用两个值进行判断：
            VK_ERROR_OUT_OF_DATE_KHR：表示交换链已经与现有界面不兼容，无法再用于渲染，这基本上是窗口大小调整
        后被触发。
            VK_SUBOPTIMAL_KHR：交换链仍可以成功地将结果呈现到界面，但界面属性不再完全匹配。（但是在这种情况下
        也要被程序判定为 run_time_error 并抛出错误么？）
        framebufferResized 判断值请参见第五步
    */
    // 情况01
    // if (result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized) // 如果已经不兼容了，那么直接重建～ 并且返回，本帧不渲染
    if (result == VK_ERROR_OUT_OF_DATE_KHR) // 如果已经不兼容了，那么直接重建～ 并且返回，本帧不渲染
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    /*
        第八步，更新“统一缓冲区”，实际上就是在每一帧都更新并应用一次新的变换阵，从而达到让视图中的
    元素“动起来”的效果。
    */
    updateUniformBuffer(currentFrame);

    /*
        第四步，死锁问题的修复
        解决方案如下，我们将 vkResetFences 函数写在这个位置，也就是说，如果程序在以上的代码段中被提前返回了，
    则 vkResetFences 不会被执行，围栏信号也就不会被重置，那么在下一个loop中也就不会被卡住～
        （通过延迟重置围栏信号的方法解决）
    */

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    /*
        第九步，记录/填充命令缓冲区，并在填充完毕后将其提交到队列
    */
    // 首先对上一次的命令缓冲区进行清除，第二个参数是一个标志位，可以先不管
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    // 之后对其进行记录/填充，指定对应交换链中的图像索引，即：这个命令缓冲区将执行的命令绑定到操作对象
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex, currentFrame);
    // 需要一个结构体来设置将命令缓冲区提交到队列时的一些配置信息
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // 控流用的信号灯，要等待哪个信号灯置位，这里才能继续往下执行？
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;           // 等几个信号灯
    submitInfo.pWaitSemaphores = waitSemaphores; // 等哪个信号灯（信号灯序列，因为可能要等待多个）
    submitInfo.pWaitDstStageMask = waitStages;   // 我们希望在哪个状态进入等待，这里配置为着色阶段

    // 提交几个命令缓冲区，以及提交哪个
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    // 控流用的信号灯，当前执行完成后，要置位哪个信号灯？也就是说，当前执行完成后，会通过这个信号灯放行哪些后续的操作。
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    /*
        最后我们将命令缓冲区提交到队列中，注意最后一个参数是我们在成员变量中写的围栏，这是可选的，在当前
    命令缓冲区已经被送入队列执行后，会允许下一次对命令缓冲区的重置/记录/填充操作，这保证了安全性，CPU仅在
    这个命令执行完成后才能在下一帧对命令缓冲区进行操作。
        vkQueueSubmit的执行就意味着GPU将执行一次对当前图像的绘制操作。
    */
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    /*
        第十一步，将图像从交换链送入屏幕
    */
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // 同样，等待信号置位后才允许执行，这里设置要等待哪个信号。
    // 这里等待的信号就是刚刚在提交图形绘制指令时设置的“任务执行完后会置位的信号”，所以说，以上的vkQueueSubmit所提交的
    // 任务在GPU上执行完毕后，以下的图形展示才会在GPU上被执行。
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    // 下面的参数指定交换链索引，这里只有一条交换链，对应的 imageIndex 就是我们在函数最开始所指定从交换链中取出的Image
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    /*
            情况02, 你也可以写成如下形式，当“不完全匹配但仍能使用”时，也选择重建交换链，但仍进行本帧的渲染。
        此时 VK_SUBOPTIMAL_KHR 也算做是“成功”返回！我们暂时不选择该方式
            注意，这里已经不是“情况2”这么简单了，不是“可选项”而是“必须做”！因为如果在尝试获取图像时交换链
        已经过时，则当前图像就已经无法再呈现给界面，所以这时候我们应该立刻重新创建交换链，并在下一次drawFrame
        调用时重试！！！
    */

    // 发出一个交换链向屏幕提交图像并进行显示的请求！！！！Finally！！去运行你的程序看有没有三角形显示吧！
    // 注意这里的提交的指令队列是 presentQueue 而非之前的 graphicsQueue。
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // 这步很重要，不要忘记在展现完之后，更新当前帧的索引
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    end = clock();
    std::cout << "time cost = " << end - begin << "ms" << std::endl;
}

// 第六步，对以上加入的控流原语进行配置
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
    /*
        以下的设置意味着在创建fences时就发出一个“signaled”信号（定义时如果不指定则默认是unsignaled），从而使得
    第一次询问fence总是通过。这在我们的程序中避免了首次询问“上一帧图像是否渲染完成”造成阻塞的问题（因为对于第一帧
    图像不可能有“上一帧”这个概念）
        尝试注释掉下面的语句，发现程序会一直卡在drawFrame最开始等待fences置位。
    */
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // 一次性为三个成员变量进行初始化配置
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

void cleanupRenderLoopRelated()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
}

/*
    第一步，
    首先，在这里我们创建这“重建交换链”的函数
*/
void recreateSwapChain()
{

    /*
        第七步，最小化问题？
        当窗口最小化时，framebuffer的size会被置为0，我们的处理手法是暂停当前的渲染，直到
    窗口恢复正常的大小（从最小化返回）。我们还是借助GLFW来实现。
    */
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    // 应该在函数的最开始，等待GPU已经完成了当前命令缓冲区的任务执行。
    vkDeviceWaitIdle(device);

    /*
        第二步
        为了确保重建这些交换链相关对象之前，旧版本的交换链已经被清除干净，我们有必要写一些清理代码。
    */
    loopCleanupSwapChain();

    // 而后重新执行以下三个“重建交换链”相关的函数
    /*
        注意，在这里的 createSwapChain() 函数中我们重新根据窗口分辨率重置了交换链中图像的正确大小，
    具体是在其内部的 chooseSwapExtent() 函数实现的。
    */
    createSwapChain();
    createImageViews();
    createFramebuffers();
    /*
        以上的方法虽然可以正确的重建交换链，但该方法的缺点是，我们不得不在需要创建新的交换链时停掉所有正在
    运行的渲染。如果想要在运行时更改交换链且不影响旧的渲染进行，可以考虑使用将之前的交换链传递到
    VkSwapchainCreateInfoKHR 的 oldSwapChain 字段上，并在使用完当前的旧交换链后立刻销毁它。
    */
}

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
