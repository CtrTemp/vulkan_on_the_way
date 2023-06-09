#include "init_imgui.h"

static VkAllocationCallbacks *g_Allocator = NULL;
static VkInstance g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice g_Device = VK_NULL_HANDLE;
static uint32_t g_QueueFamily = (uint32_t)-1;
static VkQueue g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int g_MinImageCount = 2;
static bool g_SwapChainRebuild = false;

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

// 对一些可以共用的不要去创建
static void SetupVulkan(const char **extensions, uint32_t extensions_count)
{
    VkResult err;
    // Create Vulkan Instance
    {
        g_Instance = instance;
    }

    // Select GPU
    {
        g_PhysicalDevice = physicalDevice;
    }

    // Select graphics queue family
    {
        g_QueueFamily = queueIndices.graphicsFamily.value();
    }

    // Create Logical Device (with 1 queue)
    {
        g_Device = device;
        g_Queue = graphicsQueue;
    }

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] =
            {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

static void CleanupVulkan()
{
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);
    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX); // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window *wd)
{
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

// void init_imgui_frame()
// {
//     // Setup Dear ImGui context
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO &io = ImGui::GetIO();
//     (void)io;
//     // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//     // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

//     // 这里导入全局字体
//     io.Fonts->AddFontFromFileTTF("../font/lishu_fanti.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
//     // ImFont *font = io.Fonts->AddFontFromFileTTF("../font/lishu_fanti.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

//     // 这里导入全局样式
//     // 注意,使用自定义样式时不要打开系统默认样式
//     ImGuiStyle *style = &ImGui::GetStyle();
//     style->Colors[ImGuiCol_TitleBg] = ImColor(234, 65, 79, 255);
//     style->Colors[ImGuiCol_TitleBgActive] = ImColor(55, 65, 249, 255);
//     style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(24, 65, 79, 130);

//     // Setup Dear ImGui style
//     // 注意,使用样式时不要打开系统默认样式(后者覆盖前者)
//     // ImGui::StyleColorsDark();
//     // ImGui::StyleColorsLight();
//     ImGui::StyleColorsClassic();

//     // 将vulkan特性暴露给imgui

//     // Setup Platform/Renderer bindings
//     ImGui_ImplGlfw_InitForVulkan(window, true);
//     ImGui_ImplVulkan_InitInfo init_info = {};
//     init_info.Instance = instance;
//     init_info.PhysicalDevice = physicalDevice;
//     init_info.Device = device;
//     // init_info.QueueFamily = g_QueueFamily; // 上面这里作者说好像随便填一个数字就行
//     init_info.Queue = graphicsQueue; // 这里只能给一个queue？按理说应该有多个指令集队列的吧？
//     // 后面有说明：基本上可以确定这里就之填入 graphic queue 就是正确的
//     init_info.PipelineCache = VK_NULL_HANDLE;  // 作者指明这里只在创建时起作用，其他时候不用也所谓
//     init_info.DescriptorPool = descriptorPool; // 这里不能为nullptr！！可以新建一个无意义值供imgui使用即可
//     init_info.Allocator = nullptr;             // 暂时传入 nullptr 也无所谓
//     init_info.MinImageCount = 2;
//     init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
//     // init_info.CheckVkResultFn = check_vk_result;
//     ImGui_ImplVulkan_Init(&init_info, renderPass);
// }

void imguiSetup()
{

    // Setup Vulkan
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return ;
    }
    uint32_t extensions_count = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    SetupVulkan(extensions, extensions_count);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    // // Setup Dear ImGui context
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO &io = ImGui::GetIO();
    // (void)io;
    // // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // // ImGui::StyleColorsLight();

    // // Setup Platform/Renderer backends
    // ImGui_ImplGlfw_InitForVulkan(window, true);
    // ImGui_ImplVulkan_InitInfo init_info = {};
    // init_info.Instance = g_Instance;
    // init_info.PhysicalDevice = g_PhysicalDevice;
    // init_info.Device = g_Device;
    // init_info.QueueFamily = g_QueueFamily;
    // init_info.Queue = g_Queue;
    // init_info.PipelineCache = g_PipelineCache;
    // init_info.DescriptorPool = g_DescriptorPool;
    // init_info.Subpass = 0;
    // init_info.MinImageCount = g_MinImageCount;
    // init_info.ImageCount = wd->ImageCount;
    // init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // init_info.Allocator = g_Allocator;
    // init_info.CheckVkResultFn = check_vk_result;
    // ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    // // Load Fonts
    // // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // // - Read 'docs/FONTS.md' for more instructions and details.
    // // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // // io.Fonts->AddFontDefault();
    // // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // // IM_ASSERT(font != NULL);

    // // Upload Fonts
    // {
    //     // Use any command queue
    //     VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
    //     VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

    //     err = vkResetCommandPool(g_Device, command_pool, 0);
    //     check_vk_result(err);
    //     VkCommandBufferBeginInfo begin_info = {};
    //     begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //     begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //     err = vkBeginCommandBuffer(command_buffer, &begin_info);
    //     check_vk_result(err);

    //     ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    //     VkSubmitInfo end_info = {};
    //     end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //     end_info.commandBufferCount = 1;
    //     end_info.pCommandBuffers = &command_buffer;
    //     err = vkEndCommandBuffer(command_buffer);
    //     check_vk_result(err);
    //     err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
    //     check_vk_result(err);

    //     err = vkDeviceWaitIdle(g_Device);
    //     check_vk_result(err);
    //     ImGui_ImplVulkan_DestroyFontUploadObjects();
    // }
}

void imguiRenderLoop()
{
}
