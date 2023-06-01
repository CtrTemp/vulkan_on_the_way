#include "init_vk.h"

// 全局vulkan实例定义
VkInstance instance;

// 查看当前显卡支持的扩展
// 至于“扩展”这个词应该怎么理解？现在认为应该理解为显卡硬件所支持的一些操作/特性
void checkExtension()
{
    // 首先第一步使用该函数获取当前显卡支持的扩展数
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // 再使用以上获取到的扩展数，定义一个长度一定的数组，使用相同的函数对数组进行填充。数组中就是具体的扩展名称。
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // 以下打印出当前显卡所支持的所有扩展
    std::cout << "available extensions: \n"
              << "extensionCount = " << extensionCount << " <> " << extensions.size() << std::endl;
    for (const auto &extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << std::endl;
    }
}

/**
 *  这个函数用于创建一个vulkan实例 我认为之后这部分应该放在app应用工程中单独使用
 *
 * */
void createInstance()
{
    // 创建实例前先检查是否开启了验证层，如果开启则对验证层进行核验
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    // 建立一个用于存储 app info 信息的对象
    VkApplicationInfo appInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // 需要通过sType来指明其归于哪个类型，是哪种对象
    // 以下信息 暂不做一一说明
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // tutorial 指出：在vulkan中更多的信息传递是通过结构体来实现的，而非function中一个个的parameter
    // 以下的这个 create info 并非“可选项”，而是“必须项”
    // 它会告诉 vulkan 我们要使用哪些“全局扩展”和“验证层”

    // 创建vulkan实例所需的配置信息将以一个数据结构体的形式传入，以下对该结构体进行配置，类似一个JSON格式的option参数包
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo; // 使用刚刚创建的 appInfo 传入该项信息

    // 获取创建vulkan实例所需要的一些扩展信息，我们使能哪些扩展
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // 如果我们使能了验证层用于调试：
    // 则需要对验证层的相关信息进行配置，创建一个验证层相关的实例前需要准备一个用于配置的数据结构体
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo); // 对以上我们与定义的配置数据结构体进行填充（自定义）
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        // 如果我们没有使能这个结构体，则将对应配置置为空
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void initVulkan()
{
    // 在创建 instance 之前可以先查看以下支持的扩展，并打印输出
    checkExtension();
    // 第一步就应该是创建一个 instance 它将作为你的应用与vulkan库之间的桥梁
    createInstance();
    setupDebugMessenger(instance);
}

void vkInstanceCleanUp()
{
    if (enableValidationLayers)
    {
        // 如果这里不进行析构，且你使能了验证层，就会报一个验证层Error
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    // 创建 instance 之后不要忘了销毁它
    vkDestroyInstance(instance, nullptr);
}