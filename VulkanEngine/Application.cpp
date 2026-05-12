#include "Application.h"
namespace CharismaVulkan {
	Application::Application()
	{
		initWindow();
		initVulkan();
	}

	Application::~Application()
	{
		cleanup();
	}

	void Application::run()
	{
		mainLoop();
	}
	void Application::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Engine Charisma Tutorial 03", nullptr, nullptr);
	}

	void Application::mainLoop() {
		while (!glfwWindowShouldClose(m_window)) {
			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}
		//vkDeviceWaitIdle(m_device);
	}

	void Application::initVulkan()
	{
		//1. Create Vulkan Instance
		createInstance();
		//2. setup debug messenger
		setupDebugMessenger();
		//3. Create vk Surface for rendering
		createSurface();
		//4. Pick a physical device (GPU) to run our application on
		pickPhysicalDevice();
		//5. Create a logical device to interface with the physical device and retrieve queue handles
		createLogicalDevice();
		//6. Create a swap chain to manage the images we will render to and present to the screen
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	void Application::cleanup(){
		vkDestroyInstance(m_instance, nullptr);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
	std::vector<const char*> Application::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(
			glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (ENABLE_VALIDATION_LAYERS)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}
	void Application::createInstance() {
		auto reqExtensions = getRequiredExtensions();

		uint32_t vkExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
		std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data());

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Vulkan Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
		createInfo.ppEnabledExtensionNames = reqExtensions.data();

		// Must live in same scope as vkCreateInstance and createInfo
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (ENABLE_VALIDATION_LAYERS)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Couldn't create VkInstance!");
		}
		else {
			cout << "VkInstance created successfully!" << std::endl;
		}
	}
	void Application::setupDebugMessenger() {

		assert(m_instance != VK_NULL_HANDLE);

		if (!ENABLE_VALIDATION_LAYERS)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (
			CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &sg_debugMessenger)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
	void Application::createSurface() {
		assert(m_instance != VK_NULL_HANDLE);
		assert(m_window != nullptr);

		if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
		else
		{
			cout << "VKSurfaceKHR created successfully!" << std::endl;
		}
	}
	void Application::pickPhysicalDevice() {
		assert(m_instance != VK_NULL_HANDLE);

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> devicesByScore;
		for (const auto& device : devices)
		{
			int score = rateDeviceSuitability(device);
			devicesByScore.insert(std::make_pair(score, device));
		}

		if (!devicesByScore.empty() && devicesByScore.rbegin()->first > 0)
		{
			m_physicalDevice = devicesByScore.rbegin()->second;
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

			cout << "Physical device selected: " << deviceProperties.deviceName << std::endl;
		}
		else
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	int
		Application::rateDeviceSuitability(const VkPhysicalDevice& device)
	{
		assert(device != VK_NULL_HANDLE);

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Hard requirements
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.isComplete())
		{
			return 0;
		}
		if (!checkDeviceExtensionSupport(device))
		{
			return 0;
		}
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
		{
			return 0;
		}

		// Optional features weighted by value
		int score = 0;

		// Graphics and presentation using the same family is more performant
		if (indices.graphicsFamily == indices.presentFamily)
		{
			score += 100;
		}
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}
		score += deviceProperties.limits.maxImageDimension2D;

		return score;
	}
	QueueFamilyIndices Application::findQueueFamilies(const VkPhysicalDevice& device)
	{
		assert(device != VK_NULL_HANDLE);
		assert(m_surface != VK_NULL_HANDLE);

		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.presentFamily = i;
			}

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}

		return indices;
	}

	bool
		Application::checkDeviceExtensionSupport(const VkPhysicalDevice& device)
	{
		assert(device != VK_NULL_HANDLE);

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(
			DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device)
	{
		assert(device != VK_NULL_HANDLE);
		assert(m_surface != VK_NULL_HANDLE);

		SwapChainSupportDetails details;

		// Surface Caps
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		// Surface Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				device, m_surface, &formatCount, details.formats.data());
		}

		// Presentation Modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, m_surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, m_surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void Application::createLogicalDevice() {

	}
	void Application::createSwapChain(){
	}
	void Application::createImageViews() {

	}
	void Application::createRenderPass() {

	}
	void Application::createGraphicsPipeline() {

	}
	void Application::createFramebuffers() {

	}
	void Application::createCommandPool() {

	}
	void Application::createCommandBuffers() {

	}
	void Application::createSyncObjects() {

	}
	void Application::drawFrame() {

	}
	void Application::recreateSwapChain() {

	}
	void Application::cleanupSwapChain() {

	}

}