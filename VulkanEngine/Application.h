#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <fstream>

#include <optional>
#include <cassert>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

using namespace std;

constexpr bool ENABLE_VALIDATION_LAYERS = true;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

static VkDebugUtilsMessengerEXT sg_debugMessenger;

static VkResult
CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	//UNREFERENCED_PARAMETER(messageType);
	//UNREFERENCED_PARAMETER(pUserData);

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		cout << "VALIDATION LAYER: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}

static void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}
static std::vector<char>
readFile(const std::string& filename)
{
	try
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			cerr << "Failed to open file : " << filename << endl;
		}

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		return buffer;
	}
	catch (const std::exception& ex)
	{
		throw ex;
	}
}
static void
DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

namespace CharismaVulkan {
	class Application {
		public:
		Application();
		~Application();
		void run();
	 private:
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanup();
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();
		void drawFrame();
		void recreateSwapChain();
		void cleanupSwapChain();
		std::vector<const char*> getRequiredExtensions();
		int rateDeviceSuitability(const VkPhysicalDevice& device);
		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);
		bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkShaderModule createShaderModule(const std::vector<char>& code);

		GLFWwindow* m_window;
		VkInstance m_instance  = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		VkRenderPass m_renderPass;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_graphicsPipeline;

		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		size_t m_currentFrame = 0;

		bool m_framebufferResized = false;
	};
}