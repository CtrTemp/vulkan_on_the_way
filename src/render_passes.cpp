#include "render_passes.h"

VkRenderPass renderPass;

// 创建 Render Pass 
void createRenderPass()
{
    /*
        第一步，Attachment description 附件说明
    */
    // 在我们的例子中，我们只有一个由交换链中的一个图像表示的单色缓冲区附件。（可能意思就是不需要全局配置）
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    // 由于我们还没有开启多重采样（抗锯齿），故这里我们仅进行单采样
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    /*
        loadOp和storeOp决定在渲染之前和渲染之后如何处理附件中的数据。这些配置将会被应用在framebuffer中
    的depth数据项和color数据项中。

        关于 loadOp 字段：它表示渲染一张新的图片之前，framebuffer应该是一个怎样的状态；
        VK_ATTACHMENT_LOAD_OP_LOAD： framebuffer 中现有内容（应该是上一帧的内容）将会被保留；
        VK_ATTACHMENT_LOAD_OP_CLEAR：在开始呈现下一张图像前， framebuffer 现有值将会被清除为常量
    （但这个常量值是什么呢？下面的描述说明说将会被置为“黑色”，那么这个常量值应该就是0）；
        VK_ATTACHMENT_LOAD_OP_DONT_CARE： framebuffer 中的值会被置为一个“未定义”的状态（不确定状
    态，并且我们也不在乎其中的值）
    */
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // 其实就是清零，置为常数0，代表黑色
    /*
        关于 storeOp 字段：它表示在渲染一张图片之后，framebuffer应该会被置为什么状态；
        VK_ATTACHMENT_STORE_OP_STORE： framebuffer 中的内容将存储在内存中，稍后可以读取
        VK_ATTACHMENT_STORE_OP_DONT_CARE： framebuffer 中的内容将被置为一个“未定义”状态
    */
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 存储下来
    /*
        上面提到，loadOp和storeOp这两个字段的配置是应用在framebuffer中的depth和color数据上的，而对于
    stencil（模板）数据项，我们使用stencilLoadOp / stencilStoreOp字段来进行配置（其中枚举值含义相同）
        因为我们现在完全没有启用 stencil buffer（可以参见上一个章节对应部分），所以以下可以暂时设置为“未
    定义”的状态。
    */
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    /*
        在Vulkan中，Textures和Framebuffers由具有特定像素格式的VkImage对象表示，但内存中的像素布局可能
    会根据您尝试对图像进行的操作而改变（直译）。
        根据我的理解，它可能向说明的是：目前在Framebuffer中已经有图像了，但下一步Framebuffer中的图像将
    被用作什么处理，交付给哪一个组件（可能是直接进行展示，也可能有其他操作），这些将要进行的操作需要给入特定
    “格式”的数据，也就是我们这里说的pixel存储的layout。所以在这个部分，我们为了提高程序运行效率，应该预先
    设置好输出的“格式”。
        可选的像素布局根据下一步操作的不同，分为了如下几种：
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL：用作颜色附件的图像
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR：要在交换链中显示的图像（这个应该就是直接用来展示的那种设置）
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL：要用作内存复制操作目标的图像

        其中 initialLayout 字段指示渲染过程（Render Pass）开始之前的图像布局。 这个字段我们使用
    VK_IMAGE_LAYOUT_UNDEFINED 表示我们并不关心图像在之前的布局是什么样子。但值得注意的是，图像的内容并不
    能保证被保存，但这也无关紧要了，因为在我们当前的这个教程中，图像渲染之后都要被清除掉。
        finalLayout 字段指示的是渲染过程（Render Pass）完成之后，图像自动转换到的布局。在这里我们使用
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR 进行设置，表示我们希望图像在渲染之后可以使用交换链进行直接显示。
    */
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;     // 最开始的“格式”
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 最终要输出的“格式”

    /*
        第二步，Subpasses and attachment reference 子过程和附件参考
        首先明确“子过程”Subpass是什么？子过程就是渲染之后的分支操作，其中我们最先能想到的就是直接呈现到屏幕
    进行展示，当然，这是一种最基本的“子过程”。其他可能还会有更多的子过程以及适配的操作，在此我们不提及。
        注意到，每一个子过程都需要一些组件/附件（Attachment）进行支持。下面我们针对“直接向页面进行呈现”这
    一子过程配置其需要的附件。
    */
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    // 针对刚刚上一个步骤中配置的输出的“像素布局”这里作为其下一个步骤就应用到了，并为其布局配置为最优性能
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // 以下是颜色附件的引用，因为我们只需要一个子过程，就是窗口显示渲染结果，那么下面的count配置为1
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // 这里为创建的子过程引入刚刚配置的附件
    /*
        注意这里配置的字段是 pColorAttachments，然而除了这种类型的组件，还可以引入其他类型的附件，如下，
    可配置的附件字段还有：（具体的细节之后遇到需求再进行讨论）
        pInputAttachments:从着色器读取的附件
        pResolveAttachments：用于多采样颜色附件的附件
        pDepthStencil附件：深度和模具数据的附件
        pPreserveAttachments：此子过程未使用但必须保留其数据的附件
    */

    /*
        第三步，Render Pass 的创建
    */
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment; // 关于 framebuffer 的设置
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass; // 关于子过程的设置

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void cleanupRenderPass()
{
    vkDestroyRenderPass(device, renderPass, nullptr);
}
