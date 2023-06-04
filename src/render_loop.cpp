#include "render_loop.h"

/*
    第五步，来真的了！！
    现在我们要先在成员变量中添加以上所需的控流操作原语～信号灯Semphores和围栏Fences
    第一个信号灯 imageAvailableSemaphore 用来指示图像已经从 swapchain 中获取到，准备渲染
    第二个信号灯 renderFinishedSemaphore 用来表示渲染已经完成并可以进行演示
    围栏 inFlightFence 用于确保每帧只渲染一次
*/
VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

/**
 *  完整的renderloop，在main函数的主循环中执行
 * */ 
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
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    // 上一帧展示完成后，需要我们手动重置 fence 状态，以便再下一帧继续造成阻塞
    vkResetFences(device, 1, &inFlightFence);

    /*
        第八步，下一步是从交换链中获取图像，这里的图像是未被渲染的图像，我们拿到它作为当前帧即将操作的对象。
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
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    /*
        第九步，记录/填充命令缓冲区，并在填充完毕后将其提交到队列
    */
    // 首先对上一次的命令缓冲区进行清除，第二个参数是一个标志位，可以先不管
    vkResetCommandBuffer(commandBuffer, 0);
    // 之后对其进行记录/填充，指定对应交换链中的图像索引，即：这个命令缓冲区将执行的命令绑定到操作对象
    recordCommandBuffer(commandBuffer, imageIndex);
    // 需要一个结构体来设置将命令缓冲区提交到队列时的一些配置信息
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // 控流用的信号灯，要等待哪个信号灯置位，这里才能继续往下执行？
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;           // 等几个信号灯
    submitInfo.pWaitSemaphores = waitSemaphores; // 等哪个信号灯（信号灯序列，因为可能要等待多个）
    submitInfo.pWaitDstStageMask = waitStages;   // 我们希望在哪个状态进入等待，这里配置为着色阶段

    // 提交几个命令缓冲区，以及提交哪个
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 控流用的信号灯，当前执行完成后，要置位哪个信号灯？也就是说，当前执行完成后，会通过这个信号灯放行哪些后续的操作。
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    /*
        最后我们将命令缓冲区提交到队列中，注意最后一个参数是我们在成员变量中写的围栏，这是可选的，在当前
    命令缓冲区已经被送入队列执行后，会允许下一次对命令缓冲区的重置/记录/填充操作，这保证了安全性，CPU仅在
    这个命令执行完成后才能在下一帧对命令缓冲区进行操作。
        vkQueueSubmit的执行就意味着GPU将执行一次对当前图像的绘制操作。
    */
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
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

    // 指定一个VkResult值的数组，从而检查交换链推入屏幕显示的图像是否成功。
    // 这里只有一个交换链，不必要。可以通过当前函数的返回值进行是否成功的判断。
    presentInfo.pResults = nullptr; // Optional

    // 发出一个交换链向屏幕提交图像并进行显示的请求！！！！Finally！！去运行你的程序看有没有三角形显示吧！
    // 注意这里的提交的指令队列是 presentQueue 而非之前的 graphicsQueue。
    vkQueuePresentKHR(presentQueue, &presentInfo);

    // std::cout << "render ended!" << std::endl;

    end = clock();
    std::cout << "time cost = " << end - begin << "ms" << std::endl;
}

// 第六步，对以上加入的控流原语进行配置
void createSyncObjects()
{
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
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void cleanupRenderLoopRelated()
{
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);
}
