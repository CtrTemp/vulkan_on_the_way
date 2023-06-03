#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H

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
// 引入计时器，查看程序运行时间（渲染一帧用时）
#include <ctime>


#include "logical_device_queue.h"
#include "swapchain.h"
#include "command_buffer.h"




/*
    （题外话：在本章执行程序时发现一个问题，其实读取文件时的相对路径，“相对的”并非是可执行文件的位置，而是看
你在哪里执行的这个可执行文件！！！）

    Introduction：
    Finally！！！在这个章节中，我们最终会看到我们在Vulkan中的第一个渲染结果。我们将在main函数的主循环中
开始书写函数，让我们的三角形能够通过它展示在先前创建好的窗口中。
    在真正书写这些函数之前，我们应该对总体的流程有一个全局的概览，如下：
    1/等待上一帧图像渲染完成
    2/从交换链中获取一帧图像
    3/记录/填充一个用于将图像绘制并呈现到窗口的命令缓冲区
    4/将该命令缓冲区提交到队列
    5/窗口呈现交换链中的图像
    （之后我们可能会对其进行扩展，但请一定记住，以上是当前渲染流程的真正核心）


    第一步要明确的是，Vulkan 中很多操作都是在GPU上同步执行的，如何使得这些并行操作能够同步且明确是很重要的。
Vulkan提供给我们一些“基元”来告诉GPU我们希望运行的顺序。以下的这些事件的执行必须被明确地规定执行顺序，因为
它们完全是在GPU上并行执行：
    1/从交换链中获取图像
    2/通过执行命令缓冲区中的指令来绘制图像
    3/将图像从交换链呈现到屏幕，以及将屏幕上的图像返回交换链
    注意，以上的函数操作都是异步调用的，并且在函数内部操作执行完成之前函数就会返回！这是我们不希望看到的，因为
一旦后面的操作需要前面操作的结果，这种异步执行的方式就会造成一些不确定/未定义的错乱。所以我们需要使用一些同步语
句块/指令/原语来使得程序的执行顺序完全可控。
    （注意，我们是在CPU上编写程序，当然，这里说的函数返回也就是程序返回CPU，而异步执行都是在被我们提交到GPU的
运行任务，这样就非常合理了）


    第二步：Vulkan 中的异步操作控流
    我们在Vulkan中使用Semaphores（信号灯）来为我们在队列操作之间添加顺序。信号灯既可以用于同一队列的排序，也
可以用于对不同队列的排序工作。
    在Vulkan中一共有两种类型的信号灯，binary类型和timeline类型，在这个教程中我们只讨论binary类型的信号灯。
    信号灯的状态只有两种：有信号（signaled）以及无信号（unsignaled）。从定义开始，它就默认被设置为unsignaled
状态。我们使用信号灯的方式是：将信号灯与队列中不同的操作进行关联，在队列中相邻的两个需要按照顺序执行的操作将会按
照信号灯的状态执行操作。比如A/B两个操作，B要按顺序在A之后执行。则在最开始状态A先执行，且信号灯为unsignled状态。
当A执行完毕后，信号灯切换为signaled状态，B按照这个提示准备开始执行；一旦B开始执行，信号灯又被自动切换为
unsignled状态，等待B执行完后再进行状态切换。如此循环往复。可见在一个队列中，使用一个信号灯就足以支持其顺序执行。

    第三步：Vulkan 中的同步操作控流
    围栏（Fences）在Vulkan中也起到类似信号指示器的作用。它用于同步执行，但它在主机（CPU）上执行。如果主机需要
知道GPU是否完成了某件任务时，我们使用围栏Fences。
    其实这里围栏也是只有两种状态，与以上的信号灯相同。某个提交到GPU的任务完成了，则围栏值被置为signaled。这个
其实最类似于MPI中的同步命令。其实就是人为地为程序的某一阶段制造一个阻塞事件，直到任务完成阻塞才会打开。同样的，
这个围栏也会根据状态提供一个类似于MPI_Barrier()的函数，称为vkWaitForFence(F)，其中F就是状态指示。
    与semaphores不同的是，围栏的使用会造成CPU程序的阻塞，而信号灯只在GPU设备上造成阻塞，而不会阻止CPU提交任
务。一般情况下，不同于MPI程序，非必要情况下，我们不建议使用围栏（Fences）这种会阻塞CPU程序的原语。
    需要注意，围栏变量并不会被程序自动重置，而需要我们手动重置加以控制。

    第四步：总结以及使用策略
    现在可以总结来看以上两种用于控制程序执行的操作符：Semaphores用于控制GPU上的程序执行顺序，不会造成CPU上的
任务提交方面的阻塞；而Fences会造成CPU程序阻塞，它能最大程度保证CPU与GPU正在执行的操作（对应执行的函数）是同步
的。
    在交换链（swapchain）操作时，我们选择使用semaphores，因为它完全发生在GPU上，我们不想让主机为此等待。二对
于等待上一帧完成，我们应该使用fences，因为同一时刻不可能展示两张图片，我们需要让主机等待，每一帧我们都要重新记录/
填充命令缓冲区。这里需要尤其注意，由于每一帧要刷新缓冲区，而如果此时我们不让主机等待，那么多次提交到GPU的任务可能
会造成上一帧还在读取命令缓冲区中的内容时，当前帧却要强制刷新命令缓冲区，这就造成了冲突！所以我们这里有必要让主机
CPU进行等待。
*/


extern VkSemaphore imageAvailableSemaphore;
extern VkSemaphore renderFinishedSemaphore;
extern VkFence inFlightFence;

    
void drawFrame();

void createSyncObjects();

void cleanupRenderLoopRelated();

#endif
