#pragma once
#include "veDevice.h"

namespace CharismaVulkan {

	VeDevice::VeDevice(GLFWwindow *window) {
		m_window = window;
	}

	void VeDevice::Init() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	VeDevice::~VeDevice() {
		vkDestroyDevice(m_device, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void VeDevice::createInstance() {
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

	VkInstance VeDevice::getInstance() const {
		return m_instance;
	}
	VkSurfaceKHR VeDevice::getSurface() const {
		return m_surface;
	}
	VkPhysicalDevice VeDevice::getPhysicalDevice() const {
		return m_physicalDevice;

	}
	VkDevice VeDevice::getLogicalDevice() const {
		return m_device;
	}
	VkQueue VeDevice::getGraphicQueue() const {
		return m_graphicsQueue;
	}
	VkQueue VeDevice::getPresentQueue() const {
		return m_presentQueue;
	}

	void VeDevice::setupDebugMessenger() {

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


	std::vector<const char*> VeDevice::getRequiredExtensions()
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

	void VeDevice::createSurface() {
		assert(m_instance != VK_NULL_HANDLE);
		assert(m_window != nullptr);

		if (glfwCreateWindowSurface(m_instance,m_window, nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
		else
		{
			cout << "VKSurfaceKHR created successfully!" << std::endl;
		}
	}
	void VeDevice::pickPhysicalDevice() {
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
	int VeDevice::rateDeviceSuitability(const VkPhysicalDevice &device)
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

	QueueFamilyIndices VeDevice::findQueueFamilies(const VkPhysicalDevice &device)
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


	bool VeDevice::checkDeviceExtensionSupport(const VkPhysicalDevice& device)
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

	SwapChainSupportDetails VeDevice::querySwapChainSupport(const VkPhysicalDevice &device)
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


	void VeDevice::createLogicalDevice() {
		assert(m_physicalDevice != VK_NULL_HANDLE);

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies
			= { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
		createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
		if (ENABLE_VALIDATION_LAYERS)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}
		else {
			cout << "Logical Device Created successfully." << endl;
		}

		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}
}