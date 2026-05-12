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

	}
	void Application::pickPhysicalDevice() {

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