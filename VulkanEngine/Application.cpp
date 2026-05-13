#include "Application.h"
namespace CharismaVulkan {
	Application::Application()
	{
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
	
	void Application::mainLoop() {
		while (!glfwWindowShouldClose(m_veWindow.getWindow())) {
			drawFrame();
		
			glfwPollEvents();
		}
		vkDeviceWaitIdle(m_veDevice.getLogicalDevice());
	}

	void Application::initVulkan()
	{
		m_veDevice.Init();

		//6. Create a swap chain to manage the images we will render to and present to the screen
		createSwapChain();
		//7. Create Image Views
		createImageViews();
		//8. Create Render Pass
		createRenderPass();
		//9. Create Graphics Pipeline
		createGraphicsPipeline();
		//10. Create Frame Buffers
		createFramebuffers();
		//11. Create command pool
		createCommandPool();
		//12. Create command Buffers
		createCommandBuffers();
		//13. Create Sync Objects
		createSyncObjects();
	}

	void Application::cleanup(){
		cleanupSwapChain();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(m_veDevice.getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_veDevice.getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_veDevice.getLogicalDevice(), m_inFlightFences[i], nullptr);
		}

		//m_veDevice.cleanUp();
		vkDestroyCommandPool(m_veDevice.getLogicalDevice(), m_commandPool, nullptr);
		

		//if (ENABLE_VALIDATION_LAYERS)
		//{
		//	DestroyDebugUtilsMessengerEXT(m_instance, sg_debugMessenger, nullptr);
		//}
		
		
	}
	
	
	

	void Application::createSwapChain(){

		assert(m_physicalDevice != VK_NULL_HANDLE);
		assert(m_surface != VK_NULL_HANDLE);

		SwapChainSupportDetails swapChainSupport = m_veDevice.querySwapChainSupport(m_veDevice.getPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = std::min(
			swapChainSupport.capabilities.minImageCount + 1,
			swapChainSupport.capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_veDevice.getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// QueueFamily sharing
		QueueFamilyIndices indices = m_veDevice.findQueueFamilies(m_veDevice.getPhysicalDevice());
		// NB. must remain in scope of createInfo
		uint32_t queueFamilyIndices[]
			= { indices.graphicsFamily.value(), indices.presentFamily.value() };
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;          // Optional
			createInfo.pQueueFamilyIndices = nullptr;    // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_veDevice.getLogicalDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}
		else {
			cout << "Swap Chain created successfully. " << endl;
		}

		// Get Images: (find actual imageCount of created swap chain)
		vkGetSwapchainImagesKHR(m_veDevice.getLogicalDevice(), m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		cout << "Swap Chain Image Count : " << imageCount << endl;
		vkGetSwapchainImagesKHR(m_veDevice.getLogicalDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;
	}

	//----------------------------------------------------------------------------------------
	VkSurfaceFormatKHR
		Application::chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (
				availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		if (availableFormats.empty())
		{
			throw std::runtime_error("no swapchain surface formats available to select");
		}
		return availableFormats[0];
	}

	//----------------------------------------------------------------------------------------
	VkPresentModeKHR
		Application::chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		const VkPresentModeKHR requestedMode = VK_PRESENT_MODE_MAILBOX_KHR;
		VkPresentModeKHR bestFallbackMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == requestedMode)
			{
				return requestedMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				bestFallbackMode = availablePresentMode;
			}
		}

		return bestFallbackMode;
	}

	//----------------------------------------------------------------------------------------
	VkExtent2D
		Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_veWindow.getWindow(), &width, &height);
			VkExtent2D actualExtent
				= { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));

			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}
	}
	void Application::createImageViews() {
		m_swapChainImageViews.resize(m_swapChainImages.size());
		for (size_t i = 0; i < m_swapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (
				vkCreateImageView(m_veDevice.getLogicalDevice(), &createInfo, nullptr, &m_swapChainImageViews[i])
				!= VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
			else {
				cout << "Image View created successfully. " << endl;
			}
		}

	}
	void Application::createRenderPass() {
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask
			= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		if (vkCreateRenderPass(m_veDevice.getLogicalDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
		else {
			cout << "Render Pass created successfully. " << endl;
		}

	}
	void Application::createGraphicsPipeline() {
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[]
			= { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChainExtent.width);
		viewport.height = static_cast<float>(m_swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask
			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkDynamicState dynamicStates[]
			= { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (
			vkCreatePipelineLayout(m_veDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VkPipeline graphicsPipeline;
		if (
			vkCreateGraphicsPipelines(
				m_veDevice.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		else {
			cout << "Graphics Pipeline created successfully." << endl;
		}

		vkDestroyShaderModule(m_veDevice.getLogicalDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_veDevice.getLogicalDevice(), vertShaderModule, nullptr);

	}

	//----------------------------------------------------------------------------------------
	VkShaderModule
		Application::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_veDevice.getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}

	void Application::createFramebuffers() {
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

		for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = { m_swapChainImageViews[i] };
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (
				vkCreateFramebuffer(
					m_veDevice.getLogicalDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i])
				!= VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
			else {
				cout << "Frame Buffer created successfully. " << endl;
			}
		}
		cout << "Total framebuffer created : " << m_swapChainFramebuffers.size() << endl;
	}
	void Application::createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = m_veDevice.findQueueFamilies(m_veDevice.getPhysicalDevice());

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0;

		if (vkCreateCommandPool(m_veDevice.getLogicalDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
		else {
			cout << "Command Pool created successfully. " << endl;
		}
	}
	void Application::createCommandBuffers() {
		m_commandBuffers.resize(m_swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (
			vkAllocateCommandBuffers(m_veDevice.getLogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		// Add commands
		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			// Begin commands
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;
			if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			// Begin render pass
			VkClearValue clearColor = { 1.0f, 0.0f, 1.0f, 1.0f };
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_renderPass;
			renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapChainExtent;
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
			vkCmdBeginRenderPass(
				m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(
				m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

			// Draw
			vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

			// End render pass
			vkCmdEndRenderPass(m_commandBuffers[i]);

			// End of commands
			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}
			else {
				cout << "Command Buffer created successfully. " << endl;
			}
		}
		cout << "Total command Buffer : " << m_commandBuffers.size() << endl;
	}
	void Application::createSyncObjects() {
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (
				(vkCreateSemaphore(
					m_veDevice.getLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i])
					!= VK_SUCCESS)
				|| (vkCreateSemaphore(m_veDevice.getLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
				|| (vkCreateFence(m_veDevice.getLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i])))
			{
				throw std::runtime_error("failed to create synchronization objects!");
			}
			else {
				cout << "Sync objects created successfully. " << endl;
			}
		}
		cout << "Total sync objects frame : " << MAX_FRAMES_IN_FLIGHT << endl;
	}
	void Application::drawFrame() {
		//vkWaitForFences(
		//	m_device,
		//	1,
		//	&m_inFlightFences[m_currentFrame],
		//	VK_TRUE,
		//	std::numeric_limits<uint64_t>::max());
		VkFence frameFence = m_inFlightFences[m_currentFrame];
		vkWaitForFences(m_veDevice.getLogicalDevice(), 1, &frameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

		uint32_t imageIndex;
		//VkResult result = vkAcquireNextImageKHR(
		//	m_device,
		//	m_swapChain,
		//	std::numeric_limits<uint64_t>::max(),
		//	m_imageAvailableSemaphores[m_currentFrame],
		//	VK_NULL_HANDLE,
		//	&imageIndex);
		VkSemaphore acquire_semaphore = m_imageAvailableSemaphores[m_currentFrame];
		VkResult result = vkAcquireNextImageKHR(m_veDevice.getLogicalDevice(), m_swapChain, std::numeric_limits<uint64_t>::max(), acquire_semaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		else {
			//cout << "Ready To Present Image " << endl;
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
		VkSemaphore waitSemaphores[] = { acquire_semaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_veDevice.getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);
		if (
			vkQueueSubmit(m_veDevice.getGraphicQueue(),1, &submitInfo, m_inFlightFences[m_currentFrame])
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkSwapchainKHR swapChains[] = { m_swapChain };
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		result = vkQueuePresentKHR(m_veDevice.getPresentQueue(), &presentInfo);
		if (
			result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR
			|| m_framebufferResized)
		{
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	void Application::recreateSwapChain() {
		cout << "Recreate Swap Chain " << endl;
		// Wait while window is minimized
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_veWindow.getWindow(), &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_veDevice.getLogicalDevice());

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandBuffers();
	}
	void Application::cleanupSwapChain() {
		for (auto framebuffer : m_swapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_veDevice.getLogicalDevice(), framebuffer, nullptr);
		}

		vkFreeCommandBuffers(
			m_veDevice.getLogicalDevice(),
			m_commandPool,
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data());

		vkDestroyPipeline(m_veDevice.getLogicalDevice(), m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_veDevice.getLogicalDevice(), m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_veDevice.getLogicalDevice(), m_renderPass, nullptr);
		for (auto imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_veDevice.getLogicalDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_veDevice.getLogicalDevice(), m_swapChain, nullptr);
	}

}