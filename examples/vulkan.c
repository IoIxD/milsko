/* $Id$ */
#include <Mw/Milsko.h>
#include <Mw/Vulkan.h>

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

MwWidget window, vulkan, button;
int	 ow = 300;
int	 oh = 250;

PFN_vkGetInstanceProcAddr instanceProcAddr;
VkInstance		  instance;
VkDevice		  device;
VkPhysicalDevice	  physicalDevice;

VkImage	   renderImage;
VkPipeline pipeline;

void vulkan_setup(MwWidget handle) {
	VkFormat		imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	VkDeviceMemory		renderImageMemory;
	VkResult		res;
	VkImageView		renderImageView;
	VkRenderPass		renderPass;
	VkShaderModule		fragShaderModule;
	VkShaderModule		vertShaderModule;
	VkPipelineLayout	pipelineLayout;
	VkMemoryRequirements	memRequirements;
	FILE*			vertFile;
	FILE*			fragFile;
	void*			vertBuf;
	void*			fragBuf;
	size_t			vertFileSize;
	size_t			fragFileSize;
	VkAttachmentDescription colorAttachment;
	VkAttachmentReference	colorAttachmentRef;
	VkSubpassDescription	subpass;
	VkViewport		viewport;
	VkFramebuffer		framebuffer;
	VkRect2D		scissor;
	VkCommandPool		cmdPool;
	VkCommandBuffer		cmdBuffer;
	int			graphicsIdx;
	uint32_t		i;
	uint32_t		memoryTypeIndex;
	size_t			amountRead;

	VkImageViewCreateInfo		       imgViewCreateInfo;
	VkImageCreateInfo		       imgCreateInfo;
	VkMemoryAllocateInfo		       memAllocInfo;
	VkRenderPassCreateInfo		       renderPassInfo;
	VkShaderModuleCreateInfo	       vertInfo;
	VkShaderModuleCreateInfo	       fragInfo;
	VkPipelineLayoutCreateInfo	       pipelineLayoutInfo;
	VkPipelineShaderStageCreateInfo	       vertShaderStageInfo;
	VkPipelineShaderStageCreateInfo	       fragShaderStageInfo;
	VkPipelineShaderStageCreateInfo	       shaderStages[2];
	VkPipelineVertexInputStateCreateInfo   vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineViewportStateCreateInfo      viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo   multisampling;
	VkPipelineColorBlendAttachmentState    colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo    colorBlending;
	VkGraphicsPipelineCreateInfo	       pipelineInfo;
	VkFramebufferCreateInfo		       framebufferInfo;
	VkCommandBufferAllocateInfo	       allocInfo;
	VkCommandPoolCreateInfo		       poolInfo;
	VkPhysicalDeviceMemoryProperties       memProperties;

	PFN_vkCreateImage			createImageFunc;
	PFN_vkGetImageMemoryRequirements	getImageMemoryRequirementsFunc;
	PFN_vkAllocateMemory			allocateMemoryFunc;
	PFN_vkBindImageMemory			bindImageMemoryFunc;
	PFN_vkGetPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryPropertiesFunc;
	PFN_vkCreateImageView			createImageViewFunc;
	PFN_vkCreateRenderPass			createRenderPassFunc;
	PFN_vkCreateShaderModule		createShaderModuleFunc;
	PFN_vkCreatePipelineLayout		createPipelineLayoutFunc;
	PFN_vkCreateGraphicsPipelines		createGraphicsPipelinesFunc;
	PFN_vkCreateFramebuffer			createFramebufferFunc;
	PFN_vkCreateCommandPool			createCommandPoolFunc;
	PFN_vkAllocateCommandBuffers		allocateCommandBuffersFunc;

	instanceProcAddr = MwVulkanGetInstanceProcAddr(handle);
	instance	 = MwVulkanGetInstance(handle);
	device		 = MwVulkanGetLogicalDevice(handle);
	graphicsIdx	 = MwVulkanGetGraphicsQueueIndex(handle);
	physicalDevice	 = MwVulkanGetPhysicalDevice(handle);

	createImageFunc = (PFN_vkCreateImage)instanceProcAddr(instance, "vkCreateImage");
	assert(createImageFunc);
	getImageMemoryRequirementsFunc = (PFN_vkGetImageMemoryRequirements)
	    instanceProcAddr(instance, "vkGetImageMemoryRequirements");
	assert(getImageMemoryRequirementsFunc);
	allocateMemoryFunc = (PFN_vkAllocateMemory)
	    instanceProcAddr(instance, "vkAllocateMemory");
	assert(allocateMemoryFunc);
	bindImageMemoryFunc = (PFN_vkBindImageMemory)
	    instanceProcAddr(instance, "vkBindImageMemory");
	assert(bindImageMemoryFunc);
	getPhysicalDeviceMemoryPropertiesFunc =
	    (PFN_vkGetPhysicalDeviceMemoryProperties)instanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
	assert(getPhysicalDeviceMemoryPropertiesFunc);
	createImageViewFunc = (PFN_vkCreateImageView)instanceProcAddr(instance, "vkCreateImageView");
	assert(createImageViewFunc);
	createRenderPassFunc = (PFN_vkCreateRenderPass)instanceProcAddr(instance, "vkCreateRenderPass");
	assert(createRenderPassFunc);
	createShaderModuleFunc = (PFN_vkCreateShaderModule)instanceProcAddr(instance, "vkCreateShaderModule");
	assert(createShaderModuleFunc);
	createPipelineLayoutFunc = (PFN_vkCreatePipelineLayout)instanceProcAddr(instance, "vkCreatePipelineLayout");
	assert(createPipelineLayoutFunc);
	createGraphicsPipelinesFunc = (PFN_vkCreateGraphicsPipelines)instanceProcAddr(instance, "vkCreateGraphicsPipelines");
	assert(createGraphicsPipelinesFunc);
	createFramebufferFunc = (PFN_vkCreateFramebuffer)instanceProcAddr(instance, "vkCreateFramebuffer");
	assert(createFramebufferFunc);
	createCommandPoolFunc = (PFN_vkCreateCommandPool)instanceProcAddr(instance, "vkCreateCommandPool");
	assert(createCommandPoolFunc);
	allocateCommandBuffersFunc = (PFN_vkAllocateCommandBuffers)instanceProcAddr(instance, "vkAllocateCommandBuffers");
	assert(allocateCommandBuffersFunc);

	// create a 256x256 image to draw onto
	imgCreateInfo.sType		    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgCreateInfo.pNext		    = NULL;
	imgCreateInfo.flags		    = 0;
	imgCreateInfo.imageType		    = VK_IMAGE_TYPE_2D;
	imgCreateInfo.format		    = imageFormat;
	imgCreateInfo.extent.width	    = 256;
	imgCreateInfo.extent.height	    = 256;
	imgCreateInfo.extent.depth	    = 1;
	imgCreateInfo.mipLevels		    = 1;
	imgCreateInfo.arrayLayers	    = 1,
	imgCreateInfo.samples		    = VK_SAMPLE_COUNT_1_BIT;
	imgCreateInfo.tiling		    = VK_IMAGE_TILING_OPTIMAL;
	imgCreateInfo.usage		    = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // | VK_IMAGE_USAGE_SAMPLED_BIT,
	imgCreateInfo.sharingMode	    = VK_SHARING_MODE_EXCLUSIVE;
	imgCreateInfo.queueFamilyIndexCount = 0;
	imgCreateInfo.pQueueFamilyIndices   = NULL;
	imgCreateInfo.initialLayout	    = VK_IMAGE_LAYOUT_UNDEFINED;
	if((res = createImageFunc(device, &imgCreateInfo, NULL, &renderImage)) != VK_SUCCESS) {
		printf("error creating image: %s\n", string_VkResult(res));
		exit(0);
	};

	// get the memory requirments for the image.
	getImageMemoryRequirementsFunc(device, renderImage, &memRequirements);

	// Find a memory type based on the requirements.
	getPhysicalDeviceMemoryPropertiesFunc(physicalDevice, &memProperties);
	for(i = 0; i < memProperties.memoryTypeCount; i++) {
		if((memRequirements.memoryTypeBits & (1 << i)) &&
		   (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			memoryTypeIndex = i;
			break;
		}
	}

	// Based on the memory requirements specify the allocation information.
	memAllocInfo.sType	     = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext	     = NULL;
	memAllocInfo.allocationSize  = memRequirements.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;

	// Allocate memory.
	if((res = allocateMemoryFunc(device, &memAllocInfo, NULL, &renderImageMemory)) != VK_SUCCESS) {
		printf("error allocating image memory: %s\n", string_VkResult(res));
		exit(0);
	}

	// bind image to memory
	bindImageMemoryFunc(device, renderImage, renderImageMemory, 0);

	//  Create an Image View for the Render Target Image.
	imgViewCreateInfo.sType				  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.pNext				  = NULL;
	imgViewCreateInfo.flags				  = 0;
	imgViewCreateInfo.image				  = renderImage;
	imgViewCreateInfo.viewType			  = VK_IMAGE_VIEW_TYPE_2D;
	imgViewCreateInfo.format			  = imageFormat;
	imgViewCreateInfo.components.r			  = VK_COMPONENT_SWIZZLE_IDENTITY;
	imgViewCreateInfo.components.g			  = VK_COMPONENT_SWIZZLE_IDENTITY;
	imgViewCreateInfo.components.b			  = VK_COMPONENT_SWIZZLE_IDENTITY;
	imgViewCreateInfo.components.a			  = VK_COMPONENT_SWIZZLE_IDENTITY;
	imgViewCreateInfo.subresourceRange.aspectMask	  = VK_IMAGE_ASPECT_COLOR_BIT;
	imgViewCreateInfo.subresourceRange.baseMipLevel	  = 0;
	imgViewCreateInfo.subresourceRange.levelCount	  = 1;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount	  = 1;

	if((res = createImageViewFunc(device, &imgViewCreateInfo, NULL, &renderImageView)) != VK_SUCCESS) {
		printf("error creating image views: %s\n", string_VkResult(res));
		exit(0);
	}

	//  Create a Render Pass.
	colorAttachment.flags	       = 0;
	colorAttachment.format	       = imageFormat;
	colorAttachment.samples	       = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp	       = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp	       = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentRef.attachment  = 0;
	colorAttachmentRef.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	subpass.flags			= 0;
	subpass.pipelineBindPoint	= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount	= 0;
	subpass.pInputAttachments	= NULL;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments	= &colorAttachmentRef;
	subpass.pResolveAttachments	= NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments	= NULL;

	renderPassInfo.sType	       = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext	       = NULL;
	renderPassInfo.flags	       = 0;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments    = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies   = NULL;

	if((res = createRenderPassFunc(device, &renderPassInfo, NULL, &renderPass)) != VK_SUCCESS) {
		printf("error creating the render pass: %s\n", string_VkResult(res));
		exit(0);
	}

	// Create the Vertex Shader Module.
	vertFile = fopen("triangle.vert.spv", "rb");
	fragFile = fopen("triangle.frag.spv", "rb");

	fseek(vertFile, 0L, SEEK_END);
	vertFileSize = ftell(vertFile);
	rewind(vertFile);
	vertBuf	   = malloc(vertFileSize);
	amountRead = fread(vertBuf, sizeof(char), vertFileSize, vertFile);
	printf("triangle.vert.spv: read %ld bytes\n", amountRead);

	vertInfo.sType	  = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertInfo.pNext	  = NULL;
	vertInfo.flags	  = 0;
	vertInfo.codeSize = vertFileSize * sizeof(uint32_t);
	vertInfo.pCode	  = vertBuf;

	if(createShaderModuleFunc(device, &vertInfo, NULL, &vertShaderModule) != VK_SUCCESS) {
		printf("failed to create the shader module: %s\n", string_VkResult(res));
		exit(0);
	}

	// Create the Fragment Shader Module.
	fseek(fragFile, 0L, SEEK_END);
	fragFileSize = ftell(fragFile);
	rewind(fragFile);
	fragBuf	   = malloc(fragFileSize);
	amountRead = fread(fragBuf, sizeof(char), fragFileSize, fragFile);
	printf("triangle.frag.spv: read %ld bytes\n", amountRead);

	fragInfo.sType	  = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragInfo.pNext	  = NULL;
	fragInfo.flags	  = 0;
	fragInfo.codeSize = fragFileSize * sizeof(uint32_t);
	fragInfo.pCode	  = fragBuf;

	if(createShaderModuleFunc(device, &fragInfo, NULL, &fragShaderModule) != VK_SUCCESS) {
		printf("error creating the shader module: %s\n", string_VkResult(res));
		exit(0);
	}

	// Create Pipeline Layout.
	pipelineLayoutInfo.sType		  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext		  = NULL;
	pipelineLayoutInfo.flags		  = 0;
	pipelineLayoutInfo.setLayoutCount	  = 0;
	pipelineLayoutInfo.pSetLayouts		  = NULL;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if((res = createPipelineLayoutFunc(device, &pipelineLayoutInfo, NULL, &pipelineLayout)) != VK_SUCCESS) {
		printf("error creating the pipeline layout: %s\n", string_VkResult(res));
		exit(0);
	}

	VkPipeline pipeline;
	{
		VkPipelineShaderStageCreateInfo vertShaderStageInfo;
		{
			vertShaderStageInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.pNext		= NULL;
			vertShaderStageInfo.flags		= 0;
			vertShaderStageInfo.stage		= VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module		= vertShaderModule;
			vertShaderStageInfo.pName		= "main";
			vertShaderStageInfo.pSpecializationInfo = NULL;
		}

		VkPipelineShaderStageCreateInfo fragShaderStageInfo;
		{
			fragShaderStageInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.pNext		= NULL;
			fragShaderStageInfo.flags		= 0;
			fragShaderStageInfo.stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module		= fragShaderModule;
			fragShaderStageInfo.pName		= "main";
			fragShaderStageInfo.pSpecializationInfo = NULL;
		}

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		{
			vertexInputInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.pNext				= NULL;
			vertexInputInfo.flags				= 0;
			vertexInputInfo.vertexBindingDescriptionCount	= 0;
			vertexInputInfo.pVertexBindingDescriptions	= NULL;
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions	= NULL;
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		{
			inputAssembly.sType		     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.pNext		     = NULL;
			inputAssembly.flags		     = 0;
			inputAssembly.topology		     = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;
		}

		VkViewport viewport;
		{
			viewport.x	  = 0.0f;
			viewport.y	  = 0.0f;
			viewport.width	  = (float)256;
			viewport.height	  = (float)256;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
		}

		VkRect2D scissor;
		{
			scissor.offset = (VkOffset2D){0, 0};
			scissor.extent = (VkExtent2D){(uint32_t)viewport.width, (uint32_t)viewport.height};
		}

		viewportState = (VkPipelineViewportStateCreateInfo){};
		{
			viewportState.sType	    = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports    = &viewport;
			viewportState.scissorCount  = 1;
			viewportState.pScissors	    = &scissor;
		}

		VkPipelineRasterizationStateCreateInfo rasterizer;
		{
			rasterizer.sType		   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.pNext		   = NULL;
			rasterizer.flags		   = 0;
			rasterizer.depthClampEnable	   = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode		   = VK_POLYGON_MODE_FILL;
			rasterizer.cullMode		   = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace		   = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable	   = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0;
			rasterizer.depthBiasClamp	   = 0.0;
			rasterizer.depthBiasSlopeFactor	   = 0.0;
			rasterizer.lineWidth		   = 1.0f;
		}

		VkPipelineMultisampleStateCreateInfo multisampling;
		{
			multisampling.sType		    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.pNext		    = NULL;
			multisampling.flags		    = 0;
			multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
			multisampling.sampleShadingEnable   = VK_FALSE;
			multisampling.minSampleShading	    = 0.0;
			multisampling.pSampleMask	    = NULL;
			multisampling.alphaToCoverageEnable = VK_FALSE;
			multisampling.alphaToOneEnable	    = VK_FALSE;
		}

		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		{
			colorBlendAttachment.blendEnable	 = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.colorBlendOp	 = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp	 = VK_BLEND_OP_ADD;
			colorBlendAttachment.colorWriteMask	 = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending;
		{
			colorBlending.sType		= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.pNext		= NULL;
			colorBlending.flags		= 0;
			colorBlending.logicOpEnable	= VK_FALSE;
			colorBlending.logicOp		= VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount	= 1;
			colorBlending.pAttachments	= &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo;
		{
			pipelineInfo.sType		 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext		 = NULL;
			pipelineInfo.flags		 = 0;
			pipelineInfo.stageCount		 = 2;
			pipelineInfo.pStages		 = shaderStages;
			pipelineInfo.pVertexInputState	 = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pTessellationState	 = NULL;
			pipelineInfo.pViewportState	 = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState	 = &multisampling;
			pipelineInfo.pDepthStencilState	 = NULL;
			pipelineInfo.pColorBlendState	 = &colorBlending;
			pipelineInfo.pDynamicState	 = NULL;
			pipelineInfo.layout		 = pipelineLayout;
			pipelineInfo.renderPass		 = renderPass;
			pipelineInfo.subpass		 = 0;
			pipelineInfo.basePipelineHandle	 = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex	 = 0;
		}

		if((res = createGraphicsPipelinesFunc(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline)) != VK_SUCCESS) {
			printf("failed to create graphics pipeline: %s\n", string_VkResult(res));
			exit(0);
		}
	}

	framebufferInfo.sType		= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext		= NULL;
	framebufferInfo.flags		= 0;
	framebufferInfo.renderPass	= renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments	= &renderImageView;
	framebufferInfo.width		= 256;
	framebufferInfo.height		= 256;
	framebufferInfo.layers		= 1;

	if((res = createFramebufferFunc(device, &framebufferInfo, NULL, &framebuffer)) != VK_SUCCESS) {
		printf("error creating the frame buffer: %s\n", string_VkResult(res));
		exit(0);
	}

	// Create Command Pool.
	poolInfo.sType		  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext		  = NULL;
	poolInfo.flags		  = 0;
	poolInfo.queueFamilyIndex = graphicsIdx;

	if((res = createCommandPoolFunc(device, &poolInfo, NULL, &cmdPool)) != VK_SUCCESS) {
		printf("error creating the command pool: %s\n", string_VkResult(res));
		exit(0);
	}

	// 15. Create Command Buffer to record draw commands.
	allocInfo.sType		     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext		     = NULL;
	allocInfo.commandPool	     = cmdPool;
	allocInfo.level		     = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if((res = allocateCommandBuffersFunc(device, &allocInfo, &cmdBuffer)) != VK_SUCCESS) {
		printf("error allocating the command buffers: %s\n", string_VkResult(res));
		exit(0);
	}
}

void tick(MwWidget handle, void* user_data, void* call_data) {
	(void)handle;
	(void)user_data;
	(void)call_data;
}

int main() {
	window = MwVaCreateWidget(MwWindowClass, "main", NULL, 0, 0, 400, 450,
				  MwNtitle, "hello world",
				  NULL);
	vulkan = MwCreateWidget(MwVulkanClass, "vulkan", window, 50, 50, ow, oh);

	MwAddUserHandler(window, MwNtickHandler, tick, NULL);

	vulkan_setup(vulkan);

	MwLoop(window);
}
