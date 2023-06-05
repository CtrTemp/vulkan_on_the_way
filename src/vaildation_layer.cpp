#include "vaildation_layer.h"

// 实例所需Layer扩展列表（这里主要对是否支持validation layer进行验证）
const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

// 用于调试信息输出的回调函数也需要我们手动创建，而且类似的回调我们想创建多少个就创建多少个
VkDebugUtilsMessengerEXT debugMessenger;

/**
 *  核验当前显卡对 validation layer 的支持
 * */
bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::cout << "available validation layers: \n"
              << "validation layer counts = " << layerCount << " <> " << availableLayers.size() << std::endl;
    for (const auto &layer : availableLayers)
    {
        std::cout << "\t" << layer.layerName << std::endl;
    }
    std::cout << std::endl;

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            std::string avaliable_layer_name = layerProperties.layerName;

            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) // 在列表中未找到所选的验证层
        {
            std::cout << "Validation layers NOT Supported" << std::endl;
            std::cout << std::endl;
            return false;
        }
    }
    std::cout << "All selected validation layers available" << std::endl;
    std::cout << std::endl;

    return true;
}

/**
 *  获取创建vulkan实例所需要的一些扩展信息：
 *  1、glfw创建窗口进行展示的扩展（必要）；
 *  2、验证层相关扩展（可选）；
 * */
std::vector<const char *> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

/**
 *  启动验证层，创建并注册回调函数
 * */
void setupDebugMessenger(VkInstance instance)
{
    if (!enableValidationLayers)
        return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

/**
 *  自定义 validation layer 配置数据结构体
 * */
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback; // 以上的配置是传入这个回调函数的配置，对应选项在以上的回调函数参数中有说明
    createInfo.pUserData = nullptr;             // Optional 用户传入的额外信息是可选的，不必须

    // std::string userTransData = "CtrTemp";  // 你想传入的自定义信息，可以在回调函数中打印
    // createInfo.pUserData = (char *)userTransData.c_str();
}

/**
 *  使用验证层配置信息创建对应配置的验证层函数
 * */
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
 *  注销验证层debugger
 * */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

/**
 *  debug callback 函数
 *
 *  使能验证层并注册该回调函数后，当error或warning发生时，会自动调用以下函数，你可以根据需要处理这些传入的信息。
 *
 *
 *  1、参数1：VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
 *  指出消息的严重性等级，是一系列的枚举值，并可以进行大小比较，以下的枚举值，从上到下严重性依次增加：
 * VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ：   诊断性消息（比如一些无关紧要的叙述性消息，告诉你vulkan现在在做啥）
 * VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ：      信息性消息
 * VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ：   可能不是error但可能是程序bug的消息（比如我程序最后没析构一些实例对象）
 * K_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ：      无效操作可能造成程序crush的消息
 *
 *  2、参数2：VkDebugUtilsMessageTypeFlagsEXT messageType
 *  指出message类型，用于指明具体发生了什么事情
 * VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ：       发生了一些与规范或性能无关的事件
 * VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ：    发生了违反规范或表名可能错误的事情
 * VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ：   潜在对Vulkan的非最佳应用
 *
 *
 *  3、参数3：const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData
 *  消息的内在细节，通过结构体中的以下参数来指明其具体细节
 * pMessage  以null结尾的字符串形式的调试消息
 * pObjects  与消息相关的Vulkan对象句柄数组
 * objectCount 阵列中的对象数
 *
 *  4、参数4：void *pUserData
 *  用户自己想向回调函数传入的数据信息，额外传入数据用于打印显示或调试
 *
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    // 对问题“严重性”等级进行判断，高于一定等级在控制台输出
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl
                  << std::endl;
    }

    // 当然你也可以像下面这样，无差别地打印出回调函数传入的信息
    // std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    // std::cout << (char*)(pUserData) << std::endl; // 这里可以打印自己传入的信息

    // 回调函数返回一个bool值，指示触发验证层消息的vulkan调用是否应该终止，若返回true则终止
    // 一般用于测试验证层本身，一般情况下返回false即可
    return VK_FALSE;
}
