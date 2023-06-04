#include "init_imgui.h"

void init_imgui_frame()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // 将vulkan特性暴露给imgui

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    // init_info.QueueFamily = g_QueueFamily; // 上面这里作者说好像随便填一个数字就行
    init_info.Queue = graphicsQueue; // 这里只能给一个queue？按理说应该有多个指令集队列的吧？
    // 后面有说明：基本上可以确定这里就之填入 graphic queue 就是正确的
    init_info.PipelineCache = VK_NULL_HANDLE; // 作者指明这里只在创建时起作用，其他时候不用也所谓
    // init_info.DescriptorPool = g_DescriptorPool; // 这里不能为nullptr！！可以新建一个无意义值供imgui使用即可
    init_info.Allocator = nullptr; // 暂时传入 nullptr 也无所谓
    init_info.MinImageCount = 2;
    init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    // init_info.CheckVkResultFn = check_vk_result;
    // ImGui_ImplVulkan_Init(&init_info, renderPass);
}
