#pragma once

#include "veWindow.h"
#include "veDevice.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;


constexpr int MAX_FRAMES_IN_FLIGHT = 2;


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



namespace CharismaVulkan {
	class Application {
		public:
		Application();
		~Application();
		void run();
	private:
		void initVulkan();
		void mainLoop();
		void cleanup();
		
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
		
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkShaderModule createShaderModule(const std::vector<char>& code);

		VeWindow m_veWindow{ WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Engine" };
		VeDevice m_veDevice{ m_veWindow.getWindow() };

		
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