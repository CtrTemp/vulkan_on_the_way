#include "vaildation_layer.h"

/**
 *  验证层说明：
 *
 *  vulkan明确指出，其API的设计理念是“最偶小驱动程序开销”。这导致默认情况下vulkan的API不会自动帮助检测
 * 程序中的错误（非常有限）。好处是你可以非常细化的去处理整个流程中的细节，但必须对自己所在做的事情非常明确，
 * 否则一些错误会直接导致程序崩溃而不是显式地报错。
 *
 *  为了处理这些可能发生的错误，并显式地给出对应的错误信息，vulkan提出“验证层”系统。它将作为整个程序中的
 * 一个可选组件，挂接在vulkan的函数中，提供错误检测输出等额外操作。
 *
 *  vulkan验证层可以概括为提供以下功能：
 *
 *  1、参数检测：检测输入函数的参数是否合法合规，是否在给定的范围内进行选择；
 *  2、跟踪对象/实例的创建与销毁，从而避免疏忽造成的内存泄漏问题；
 *  3、通过跟踪调用/启动线程的来源，来检测线程的安全性；（这个还不太明白）
 *  4、可以将所有的函数调用以及传递参数进行打印输出，从而监测；
 *  5、Tracing Vulkan calls for profiling and replaying；（这个还得不太明白）
 *
 *  在你的debug版本中可以自由使用验证层，并且可以与其他你自己手写/自定义的程序叠加使用。在最终的release
 * 版本中只需要简单地禁用验证层即可。
 *
 *
 *
 * */

// 实例所需Layer扩展列表（这里主要对是否支持validation layer进行验证）
const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
// 用于调试信息输出的回调函数也需要我们手动创建，而且类似的回调我们想创建多少个就创建多少个
VkDebugUtilsMessengerEXT debugMessenger;

/**
 *  如果选择开启验证层，则使用以下函数对验证层进行核验：
 *  例举出所有的实例验证层支持，并查看当前所选的验证层是否在可用验证层列表中
 * */
bool checkValidationLayerSupport()
{
    // 获得当前可用的验证层个数
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    // 同样，使用以上函数得到的验证层个数，设置数组，并再次调用该函数，获得当前可用验证层参数/名称
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // 以下打印出当前显卡所支持的所有扩展
    std::cout << "available validation layers: \n"
              << "validation layer counts = " << layerCount << " <> " << availableLayers.size() << std::endl;
    for (const auto &layer : availableLayers)
    {
        std::cout << "\t" << layer.layerName << std::endl;
    }
    std::cout << std::endl;

    // 遍历当前定义的验证层，查看其是否在可用的验证层中
    // 其中 validationLayers 是我们定义在文件中的全局变量，表示选中的验证层
    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        // 遍历可用的验证层
        for (const auto &layerProperties : availableLayers)
        {
            std::string avaliable_layer_name = layerProperties.layerName;

            if (strcmp(layerName, layerProperties.layerName) == 0) // 在可用验证层中匹配到了当前所选中/所需的验证层
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) // 在列表中未找到所选的验证层
        {
            return false;
        }
    }
    std::cout << "All selected validation layers available" << std::endl;
    std::cout << std::endl;

    return true;
}

/**
 *  获取创建vulkan实例所需要的一些扩展信息，我们使能哪些扩展：
 *  1、其中glfw创建窗口进行展示的一些扩展是我们所必须要获取的；
 *  2、其次关于验证层相关的扩展，我们根据是否使能验证层进行添加；
 *  返回值是一个字符串列表
 * */
std::vector<const char *> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    // glfw 指定的扩展一般是必须的
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // required extension 打印输出验证
    std::cout << "Required extensions:" << std::endl;
    for (const auto &requiredExtension : extensions)
    {
        std::cout << requiredExtension << std::endl;
    }
    std::cout << std::endl;

    return extensions;
}

/**
 *  debug callback 函数
 *
 *  即：当程序中出现一些错误/生辰一些信息，并且在你使能了验证层后，程序会自动调用这个回调函数，参数的传入都是自动的
 * 下面的回调函数中，你只需要决定何时去对这些信息作何操作即可（比如当问题严重性超过某个阈值后进行打印输出）
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
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    // 可用的有关于严重性等级的比较函数，当传入消息的严重性大于既定的等级后，下面可以选择进行一些操作
    // 比如可以将问题打印输出（这里我将等级设置的高一些，避免大部分的扰乱信息输出）
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

/**
 *  对验证层配置结构体的自定义填充
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

// 使用验证层配置信息创建对应配置的验证层函数
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

// 析构函数，用于将验证层回调函数销毁，释放内存，应该放入 cleanup 函数中
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

/**
 *  启动验证层
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
