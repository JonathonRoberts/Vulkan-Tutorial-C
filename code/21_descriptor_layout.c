#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <cglm/cglm.h>

#include <GLFW/glfw3.h>
#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan_core.h>
#define WIDTH 800
#define HEIGHT 600

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/* for some reason CLANG doesn't realise this is in the glfw3 library */
extern GLFWAPI VkResult glfwCreateWindowSurface(VkInstance instance, GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,NULL};

#define MAX_FRAMES_IN_FLIGHT 2
#define ATTRIBUTE_DESCRIPTION_COUNT 2

float radians(float radians) {
    return radians * (180.0 / M_PI);
}

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR formats[64];
	VkPresentModeKHR presentModes[64];
	uint32_t formatCount;
	uint32_t presentModeCount;
};

struct application
{
	GLFWwindow *window;
	VkInstance instance;
	int width;
	int height;
	const char *extensions[64];
	const char enabled_layers[64][64];
	//const char devices[64][64];
	uint32_t enabled_extension_count;
	uint32_t enabled_layers_count;
	uint32_t deviceCount;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDevice devices[64];
	VkPhysicalDeviceProperties deviceProperties;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	VkImage swapChainImages[64];
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	uint32_t imageCount;
	VkImageView swapChainImageViews[64];
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkFramebuffer swapChainFramebuffers[64];
	VkCommandPool commandPool;
	VkCommandBuffer *commandBuffers;
	VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
	VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
	VkFence imagesInFlight[64];
	size_t currentFrame;
	bool framebufferResized;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkBuffer uniformBuffers[64];
	VkDeviceMemory uniformBuffersMemory[64];
};
struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
};
struct Vertex {
	vec2 pos;
	vec3 color;
};
struct UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
};

const struct Vertex vertices[] = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
const uint16_t indices[] = {0,1,2,2,3,0};

void initWindow(struct application *app);
void run(struct application *app);
void initVulkan();
void cleanup(struct application *app);
void mainLoop(struct application *app);
void createInstance(struct application *app);
void createSwapChain(struct application *app);
void createImageViews(struct application *app);
void createRenderPass(struct application *app);
void createDescriptorSetLayout(struct application *app);
void createGraphicsPipeline(struct application *app);
void createFramebuffers(struct application *app);
void createCommandPool(struct application *app);
void createCommandBuffers(struct application *app);
void createSyncObjects(struct application *app);
void createVertexBuffer(struct application *app);
void createIndexBuffer(struct application *app);
void createUniformBuffers(struct application *app);
void getRequiredExtensions(struct application *app);
bool checkValidationLayerSupport(struct application *app);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(enum VkDebugUtilsMessageSeverityFlagBitsEXT messageseverity,  unsigned int messageType,  const struct VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
void pickPhysicalDevice(struct application *app);
void createLogicalDevice(struct application *app);
bool isDeviceSuitable(VkPhysicalDevice device,VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device,VkSurfaceKHR surface);
struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface);
void createSurface(struct application *app);
void setupDebugMessenger(struct application *app);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, uint32_t formatCount);
VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes,uint32_t presentModeCount);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities,struct application *app);
struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,VkSurfaceKHR surface);
char *readFile(char *filename);
VkShaderModule createShaderModule(struct application *app,char *code);
void drawFrame(struct application *app);
void recreateSwapChain(struct application *app);
static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
void cleanupSwapChain(struct application *app) ;
static VkVertexInputBindingDescription *getBindingDescription(); 
static VkVertexInputAttributeDescription *getAttributeDescriptions(VkVertexInputAttributeDescription destination[2]);
void createBuffer(struct application *app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory);
void copyBuffer(struct application *app,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void updateUniformBuffer(struct application *app, uint32_t currentImage);

uint32_t findMemoryType(struct application *app,uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkVertexInputAttributeDescription *getAttributeDescriptions(VkVertexInputAttributeDescription destination[ATTRIBUTE_DESCRIPTION_COUNT])
{
        VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT];
	memset(&attributeDescriptions,0,sizeof(attributeDescriptions[0])*ATTRIBUTE_DESCRIPTION_COUNT);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(struct Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(struct Vertex, color);

        memcpy(destination,attributeDescriptions,sizeof(attributeDescriptions)*ATTRIBUTE_DESCRIPTION_COUNT);
	return destination;
}

VkVertexInputBindingDescription *getBindingDescription(VkVertexInputBindingDescription destination[1])
{
	VkVertexInputBindingDescription bindingDescription[1];
	memset(&bindingDescription,0,sizeof(bindingDescription));
	bindingDescription[0].binding = 0;
	bindingDescription[0].stride = sizeof(struct Vertex);
	bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        memcpy(destination,bindingDescription,sizeof(bindingDescription[0]));

	return destination;
}

void createSurface(struct application *app) {
	if (glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface) != VK_SUCCESS) {
		printf("failed to create window surface!");
		exit(1);
	}
}
void createLogicalDevice(struct application *app)
{
	struct QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice,app->surface);

	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfos[64];
	uint32_t queueCount = 0;


	if(indices.graphicsFamily!=indices.presentFamily){
		VkDeviceQueueCreateInfo queueCreateInfo;
		memset(&queueCreateInfo,0,sizeof(queueCreateInfo));
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.flags = 0;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		memcpy(&queueCreateInfos[0],&queueCreateInfo,sizeof(queueCreateInfo));
		queueCount++;
	}
	VkDeviceQueueCreateInfo queueCreateInfo;
	memset(&queueCreateInfo,0,sizeof(queueCreateInfo));
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.presentFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.flags = 0;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	memcpy(&queueCreateInfos[queueCount],&queueCreateInfo,sizeof(queueCreateInfo));
	queueCount++;

	VkPhysicalDeviceFeatures deviceFeatures;
	memset(&deviceFeatures,0,sizeof(deviceFeatures));

	VkDeviceCreateInfo createInfo;
	memset(&createInfo,0,sizeof(createInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfos[0];
	createInfo.queueCreateInfoCount = queueCount;

	createInfo.pEnabledFeatures = &deviceFeatures;

	int i;
	for(i=0;deviceExtensions[i]!=NULL;i++)
		;
	createInfo.enabledExtensionCount = i;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = ARRAY_SIZE(validationLayers);
		createInfo.ppEnabledLayerNames = validationLayers;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(app->physicalDevice, &createInfo, NULL, &app->device) != VK_SUCCESS) {
		printf("failed to create logical device!");
		exit(1);
	}
	vkGetDeviceQueue(app->device, indices.graphicsFamily, 0, &app->graphicsQueue);
	vkGetDeviceQueue(app->device, indices.presentFamily, 0, &app->presentQueue);

}

void createSwapChain(struct application *app)
{
	struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(app->physicalDevice,app->surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats,swapChainSupport.formatCount);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes,swapChainSupport.presentModeCount);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities,app);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo;
	memset(&createInfo,0,sizeof(createInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = app->surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	struct QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice,app->surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(app->device, &createInfo, NULL, &(app->swapChain)) != VK_SUCCESS) {
		printf("failed to create swap chain!");
		exit(1);
	}

	vkGetSwapchainImagesKHR(app->device, app->swapChain, &(app->imageCount), NULL);
	vkGetSwapchainImagesKHR(app->device, app->swapChain, &(app->imageCount), app->swapChainImages);

	app->swapChainImageFormat = surfaceFormat.format;
	app->swapChainExtent = extent;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, uint32_t formatCount) {
	int i;
	for (i=0;i<formatCount;i++) {
		if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormats[i];
		}
	}

	return availableFormats[0];
}
 VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes,uint32_t presentModeCount) {
	int i;
	for (i=0;i<presentModeCount;i++){
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentModes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities,struct application *app) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(app->window, &width, &height);
		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height
	};

	actualExtent.width = capabilities.maxImageExtent.width > actualExtent.width ? actualExtent.width : capabilities.maxImageExtent.width;
	actualExtent.width = capabilities.minImageExtent.width > actualExtent.width ? capabilities.minImageExtent.width : actualExtent.width;
	actualExtent.height = capabilities.maxImageExtent.height > actualExtent.height ? actualExtent.height : capabilities.maxImageExtent.height;
	actualExtent.height = capabilities.minImageExtent.height > actualExtent.height ? capabilities.minImageExtent.height : actualExtent.height;

	return actualExtent;
	}
}

struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,VkSurfaceKHR surface)
{
	struct SwapChainSupportDetails details;
	memset(&details,0,sizeof(details));

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
	if (formatCount != 0) {
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
        }
	
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);
	if (presentModeCount != 0) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes);
        }
	details.presentModeCount=presentModeCount;
	details.formatCount=formatCount;
	return details;
}
void createImageViews(struct application *app) {

	size_t i;
	for (i = 0; i < app->imageCount; i++) {
		VkImageViewCreateInfo createInfo;
		memset(&createInfo,0,sizeof(createInfo));
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = app->swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = app->swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(app->device, &createInfo, NULL, &app->swapChainImageViews[i]) != VK_SUCCESS) {
		printf("failed to create image views!");
		exit(1);
		}
	}
}
void createRenderPass(struct application *app)
{
	VkAttachmentDescription colorAttachment;
	memset(&colorAttachment,0,sizeof(colorAttachment));
	colorAttachment.format = app->swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef;
	memset(&colorAttachmentRef,0,sizeof(colorAttachmentRef));
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass;
	memset(&subpass,0,sizeof(subpass));
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo;
	memset(&renderPassInfo,0,sizeof(renderPassInfo));
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(app->device, &renderPassInfo, NULL, &(app->renderPass)) != VK_SUCCESS) {
		printf("failed to create render pass!");
		exit(1);
	}

}

void createDescriptorSetLayout(struct application *app)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding;
	memset(&uboLayoutBinding,0,sizeof(uboLayoutBinding));
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = NULL;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	memset(&layoutInfo,0,sizeof(layoutInfo));
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(app->device, &layoutInfo, NULL, &(app->descriptorSetLayout)) != VK_SUCCESS) {
		printf("failed to create descriptor set layout!");
		exit(1);
	}
}

void createGraphicsPipeline(struct application *app)
{
	//char *vertShaderCode = readFile("shaders/vert.spv");
	//char *fragShaderCode = readFile("shaders/frag.spv");
        VkShaderModule vertShaderModule = createShaderModule(app,"shaders/vert.spv");
        VkShaderModule fragShaderModule = createShaderModule(app,"shaders/frag.spv");

        VkPipelineShaderStageCreateInfo vertShaderStageInfo;
	memset(&vertShaderStageInfo,0,sizeof(vertShaderStageInfo));
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo;
	memset(&fragShaderStageInfo,0,sizeof(fragShaderStageInfo));
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	memset(&vertexInputInfo,0,sizeof(vertexInputInfo));
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription bindingDescription;
	getBindingDescription(&bindingDescription);

	VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT];
	getAttributeDescriptions(attributeDescriptions);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)ATTRIBUTE_DESCRIPTION_COUNT;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription *)&attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	memset(&inputAssembly,0,sizeof(inputAssembly));
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	memset(&viewport,0,sizeof(viewport));
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) app->swapChainExtent.width;
	viewport.height = (float) app->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	memset(&scissor,0,sizeof(scissor));
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = app->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState;
	memset(&viewportState,0,sizeof(viewportState));
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer;
	memset(&rasterizer,0,sizeof(rasterizer));
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling;
	memset(&multisampling,0,sizeof(multisampling));
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	memset(&colorBlendAttachment,0,sizeof(colorBlendAttachment));
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending;
	memset(&colorBlending,0,sizeof(colorBlending));
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo;
	memset(&pipelineLayoutInfo,0,sizeof(pipelineLayoutInfo));
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &app->descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(app->device, &pipelineLayoutInfo, NULL, &(app->pipelineLayout)) != VK_SUCCESS) {
		printf("failed to create pipeline layout!");
		exit(1);
	}
	VkGraphicsPipelineCreateInfo pipelineInfo;
	memset(&pipelineInfo,0,sizeof(pipelineInfo));
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = app->pipelineLayout;
	pipelineInfo.renderPass = app->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &(app->graphicsPipeline)) != VK_SUCCESS) {
		printf("failed to create graphics pipeline!");
		exit(1);
	}

        vkDestroyShaderModule(app->device, fragShaderModule, NULL);
        vkDestroyShaderModule(app->device, vertShaderModule, NULL);
}

VkShaderModule createShaderModule(struct application *app,char *filename)
{
	char *code = 0;
	long length;
	FILE *f = fopen (filename, "r");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		code = malloc(length);
		if(code) {
			fread(code, 1, length, f);
		}
		fclose(f);
	}
	else{
		printf("unable to open %s\n",filename);
		exit(1);
	}
	VkShaderModuleCreateInfo createInfo;
	memset(&createInfo,0,sizeof(createInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = length;
	createInfo.pCode = (uint32_t*) code;

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(app->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
		printf("failed to create shader module!");
		exit(1);
	}
	free(code);
	return shaderModule;
}

void createFramebuffers(struct application *app)
{
	size_t i;
	app->currentFrame = 0;
	app->framebufferResized = false;

	for (i = 0; i < app->imageCount; i++) {
		VkImageView attachments[] = {
			app->swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo;
		memset(&framebufferInfo,0,sizeof(framebufferInfo));
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = app->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = app->swapChainExtent.width;
		framebufferInfo.height = app->swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(app->device, &framebufferInfo, NULL, &(app->swapChainFramebuffers[i])) != VK_SUCCESS) {
			printf("failed to create framebuffer!");
			exit(1);
		}
	}
}
void createCommandPool(struct application *app)
{
	struct QueueFamilyIndices queueFamilyIndices = findQueueFamilies(app->physicalDevice,app->surface);

	VkCommandPoolCreateInfo poolInfo;
	memset(&poolInfo,0,sizeof(poolInfo));
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	if(vkCreateCommandPool(app->device, &poolInfo, NULL, &(app->commandPool)) != VK_SUCCESS) {
		printf("failed to create command pool!");
		exit(1);
	}
}

void createVertexBuffer(struct application *app)
{
	VkDeviceSize bufferSize = sizeof(vertices[0])*4; /*magic number is number of vertices*/
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(app,bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	void* data;
	vkMapMemory(app->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	    memcpy(data, vertices, (size_t) bufferSize);
	vkUnmapMemory(app->device, stagingBufferMemory);
	createBuffer(app,bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->vertexBuffer, &app->vertexBufferMemory);
	copyBuffer(app,stagingBuffer,app->vertexBuffer,bufferSize);

	vkDestroyBuffer(app->device,stagingBuffer,NULL);
	vkFreeMemory(app->device,stagingBufferMemory,NULL);
}
void createIndexBuffer(struct application *app) {
	VkDeviceSize bufferSize = sizeof(indices[0]) * 6;/*magic number is number of indices*/

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(app,bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	void* data;
	vkMapMemory(app->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices, (size_t) bufferSize);
	vkUnmapMemory(app->device, stagingBufferMemory);

	createBuffer(app,bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->indexBuffer, &app->indexBufferMemory);

	copyBuffer(app,stagingBuffer, app->indexBuffer, bufferSize);

	vkDestroyBuffer(app->device, stagingBuffer, NULL);
	vkFreeMemory(app->device, stagingBufferMemory, NULL);
}

void createUniformBuffers(struct application *app)
{
        VkDeviceSize bufferSize = sizeof(struct UniformBufferObject);

	size_t i;
        for(i = 0; i < app->imageCount; i++) {
		createBuffer(app,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &app->uniformBuffers[i], &app->uniformBuffersMemory[i]);
        }
}

void createBuffer(struct application *app,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
        VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo,0,sizeof(bufferInfo));
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(app->device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
		printf("failed to create buffer!");
		exit(1);
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(app->device, *buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo;
	memset(&allocInfo,0,sizeof(allocInfo));
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(app, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(app->device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
		printf("failed to allocate buffer memory!");
		exit(1);
        }

        vkBindBufferMemory(app->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(struct application *app,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo,0,sizeof(allocInfo));
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = app->commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(app->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo;
	memset(&beginInfo,0,sizeof(beginInfo));
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion;
	memset(&copyRegion,0,sizeof(copyRegion));
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo;
	memset(&submitInfo,0,sizeof(submitInfo));
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(app->graphicsQueue);

	vkFreeCommandBuffers(app->device, app->commandPool, 1, &commandBuffer);
}

uint32_t findMemoryType(struct application *app,uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memProperties);

	uint32_t i;
	for (i = 0; i < memProperties.memoryTypeCount; i++) {
	    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
		return i;
	    }
	}

	printf("failed to find suitable memory type!");
	exit(1);
}

void createCommandBuffers(struct application *app)
{
	app->commandBuffers = malloc(sizeof(VkCommandBuffer)* app->imageCount);
	VkCommandBufferAllocateInfo allocInfo;
	memset(&allocInfo,0,sizeof(allocInfo));
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = app->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount =  app->imageCount;

	if (vkAllocateCommandBuffers(app->device, &allocInfo, app->commandBuffers) != VK_SUCCESS) {
		printf("failed to allocate command buffers!");
		exit(1);
	}

	size_t i;
	for (i = 0; i < app->imageCount; i++) {
		VkCommandBufferBeginInfo beginInfo;
		memset(&beginInfo,0,sizeof(beginInfo));
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(app->commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			printf("failed to begin recording command buffer!");
			exit(1);
		}

		VkRenderPassBeginInfo renderPassInfo;
		memset(&renderPassInfo,0,sizeof(renderPassInfo));
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = app->renderPass;
		renderPassInfo.framebuffer = app->swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = app->swapChainExtent;

		VkClearValue clearColor;
		clearColor.color.float32[0] = 0.0f;
		clearColor.color.float32[1] = 0.0f;
		clearColor.color.float32[2] = 0.0f;
		clearColor.color.float32[3] = 1.0f;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(app->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(app->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphicsPipeline);

		VkBuffer vertexBuffers[] = {app->vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(app->commandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(app->commandBuffers[i],app->indexBuffer,0,VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(app->commandBuffers[i], (uint32_t)6,1,0,0,0);/*magic number 6 is number of indices*/

		vkCmdEndRenderPass(app->commandBuffers[i]);

		if (vkEndCommandBuffer(app->commandBuffers[i]) != VK_SUCCESS) {
			printf("failed to record command buffer!");
			exit(1);
		}
	}
}

void createSyncObjects(struct application *app)
{
	//app->imagesInFlight = (VkFence*)malloc(sizeof(app->imagesInFlight)*app->imageCount);
	memset(&app->imagesInFlight, VK_NULL_HANDLE,app->imageCount);

	VkSemaphoreCreateInfo semaphoreInfo;
	memset(&semaphoreInfo,0,sizeof(semaphoreInfo));
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo;
	memset(&fenceInfo,0,sizeof(fenceInfo));
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	size_t i;
	for (i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &(app->imageAvailableSemaphores[i])) != VK_SUCCESS ||
		vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &(app->renderFinishedSemaphores[i])) != VK_SUCCESS ||
		vkCreateFence(app->device, &fenceInfo, NULL, &(app->inFlightFences[i])) != VK_SUCCESS) {
			printf("failed to create synchronization objects for a frame!");
			exit(1);
		}
	}
}

void updateUniformBuffer(struct application *app, uint32_t currentImage)
{
	static struct timespec startTime;
	static int firstRun = 0;
	if(!firstRun){
		clock_gettime(CLOCK_MONOTONIC,&startTime);
		firstRun++;
	}
	struct timespec currentTime;
	clock_gettime(CLOCK_MONOTONIC,&currentTime);

        float time = ((currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_nsec-startTime.tv_nsec)/1000000000)*1.0;

        struct UniformBufferObject ubo;
	memset(&ubo,0,sizeof(ubo));
	mat4 m = {1.0f};
	vec3 axis = {0.0f,0.0f,1.0f};
        glm_rotate(ubo.model, time * radians(90.0f), axis);
	vec3 eye = {2.0f,2.0f,2.0f};
	vec3 center = {0.0f,0.0f,0.0f};
	vec3 up = {0.0f,0.0f,1.0f};
        glm_lookat(eye, center, up,ubo.view);
        glm_perspective(radians(45.0f), app->swapChainExtent.width / (float) app->swapChainExtent.height, 0.1f, 10.0f,ubo.proj);
        ubo.proj[1][1] *= -1;

        void *data;
        vkMapMemory(app->device, app->uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(app->device, app->uniformBuffersMemory[currentImage]);

}

void drawFrame(struct application *app)
{
	vkWaitForFences(app->device, 1, &(app->inFlightFences[app->currentFrame]), VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(app->device, app->swapChain, UINT64_MAX, app->imageAvailableSemaphores[app->currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain(app);
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		printf("failed to acquire swap chain image!");
		exit(1);
	}
	
	updateUniformBuffer(app,imageIndex);

	if (app->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
	printf("before\n");
		vkWaitForFences(app->device, 1, &(app->imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);
	printf("after\n");

	}
	app->imagesInFlight[imageIndex] = app->inFlightFences[app->currentFrame];

	VkSubmitInfo submitInfo;
	memset(&submitInfo,0,sizeof(submitInfo));
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores = {app->imageAvailableSemaphores[app->currentFrame],};
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphores;
	submitInfo.pWaitDstStageMask = &waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &app->commandBuffers[imageIndex];

	VkSemaphore signalSemaphores = app->renderFinishedSemaphores[app->currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphores;

	vkResetFences(app->device, 1, &app->inFlightFences[app->currentFrame]);

	if (vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, app->inFlightFences[app->currentFrame]) != VK_SUCCESS) {
		printf("failed to submit draw command buffer!");
		exit(1);
	}

	VkPresentInfoKHR presentInfo;
	memset(&presentInfo,0,sizeof(presentInfo));
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &signalSemaphores;

	VkSwapchainKHR swapChains = app->swapChain;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(app->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->framebufferResized) {
		app->framebufferResized = false;
		recreateSwapChain(app);
	} else if (result != VK_SUCCESS) {
		printf("failed to present swap chain image!");
		exit(1);
        }

	app->currentFrame = (app->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	struct QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties queueFamilies[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
	int i;
	int success = 0;
	for (i=0;i<queueFamilyCount;i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			success |= 1;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if(presentSupport){
			indices.presentFamily = i;
			success |= 2;
		}
	}
	if(success&3)
		return indices;

	printf("no suitable queue found\n");
	exit(1);
}

bool isDeviceSuitable(VkPhysicalDevice device,VkSurfaceKHR surface)
{
	struct QueueFamilyIndices indices = findQueueFamilies(device,surface);
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool extensionsSupported=checkDeviceExtensionSupport(device,surface);

	bool swapChainAdequate = false;
	if(extensionsSupported){
		struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device,surface);
		swapChainAdequate = (swapChainSupport.formatCount!=0 && swapChainSupport.presentModeCount!=0);
	}

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader&&extensionsSupported && swapChainAdequate;
}
bool checkDeviceExtensionSupport(VkPhysicalDevice device,VkSurfaceKHR surface)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	VkExtensionProperties availableExtensions[extensionCount];
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

	int c,i,j,ans;
	for(c=0;deviceExtensions[c]!=NULL;c++)
		;
	ans = c;
	char **requiredExtensions;
	requiredExtensions = malloc(sizeof(deviceExtensions));
	memcpy(requiredExtensions,deviceExtensions,sizeof(deviceExtensions));

	for (i=0;i!=extensionCount;i++){
		for(j=0;j!=c;j++){
			if(!strcmp(availableExtensions[i].extensionName,requiredExtensions[j])){
				ans--;
				break;
			}
		}
	}
	return ans==0;

}
void pickPhysicalDevice(struct application *app)
{
	app->physicalDevice = VK_NULL_HANDLE;
	app->deviceCount = 0;
	vkEnumeratePhysicalDevices(app->instance,&app->deviceCount,NULL);
	if(app->deviceCount == 0){
		printf("failed to find GPUs with VUlkan support!\n");
		exit(1);
	}
	vkEnumeratePhysicalDevices(app->instance,&app->deviceCount,app->devices);

	int i;
	for(i=0;i<app->deviceCount;i++){
		if(isDeviceSuitable(app->devices[i],app->surface)){
			app->physicalDevice =  app->devices[i];
			break;
		}
	}
	if(app->physicalDevice==VK_NULL_HANDLE){
		printf("failed to find a suitable GPU!\n");
		exit(1);
	}

}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(enum VkDebugUtilsMessageSeverityFlagBitsEXT messageseverity,  unsigned int messageType,  const struct VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    return false;
}


void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
        createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo->pfnUserCallback = debugCallback;
	createInfo->pNext = NULL;
    }

bool checkValidationLayerSupport(struct application *app) {

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	app->enabled_layers_count = layerCount;
	VkLayerProperties layers[64];
        vkEnumerateInstanceLayerProperties(&layerCount, layers);
	int i;
	bool layerFound = false;
        for (i=0;i<app->enabled_layers_count;i++){
		/*printf("%s\n",layers[i].layerName);*/
		strcpy((char *)app->enabled_layers[i],layers[i].layerName);
		/*TODO - enable checking for more than one validation layer*/
		if (strcmp(validationLayers[0],layers[i].layerName) == 0) {
			layerFound = true;
		}
        }
	return layerFound;
}
char *readFile(char *filename)
{
	char *buffer = 0;
	long length;
	FILE *f = fopen (filename, "r");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if(buffer) {
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}
	return buffer;
}

void getRequiredExtensions(struct application *app)
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	int i;
	for(i = 0; glfwExtensions[i] != NULL; i++){
		     app->extensions[i] = (char*)glfwExtensions[i];
	}
	app->enabled_extension_count = glfwExtensionCount;
	if(!enableValidationLayers)
		return;
	app->extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	app->enabled_extension_count++;
}

void run(struct application *app)
{
	initWindow(app);
	initVulkan(app);
	mainLoop(app);
	cleanup(app);
}

void initWindow(struct application *app)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
	glfwSetWindowUserPointer(app->window, app);
	glfwSetFramebufferSizeCallback(app->window,framebufferResizeCallback);
	if(!app->window){
		printf("Unable to create GLFW Window");
		exit(1);
	}
}
static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
        struct application *app = glfwGetWindowUserPointer(window);
        app->framebufferResized = true;
}
void initVulkan(struct application *app)
{
	createInstance(app);
	createSurface(app);
	pickPhysicalDevice(app);
	createLogicalDevice(app);
	createSwapChain(app);
	createImageViews(app);
	createRenderPass(app);
	createDescriptorSetLayout(app);
	createGraphicsPipeline(app);
	createFramebuffers(app);
	createCommandPool(app);
	createVertexBuffer(app);
	createIndexBuffer(app);
	createUniformBuffers(app);
	createCommandBuffers(app);
	createSyncObjects(app);
}
void cleanup(struct application *app)
{
	cleanupSwapChain(app);

	vkDestroyDescriptorSetLayout(app->device,app->descriptorSetLayout,NULL);

	vkDestroyBuffer(app->device,app->indexBuffer,NULL);
	vkFreeMemory(app->device,app->indexBufferMemory,NULL);

	vkDestroyBuffer(app->device,app->vertexBuffer,NULL);
	vkFreeMemory(app->device,app->vertexBufferMemory,NULL);
	int i;
	for(i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
		vkDestroySemaphore(app->device, app->renderFinishedSemaphores[i], NULL);
		vkDestroySemaphore(app->device, app->imageAvailableSemaphores[i], NULL);
		vkDestroyFence(app->device, app->inFlightFences[i], NULL);
	}

	vkDestroyCommandPool(app->device,app->commandPool,NULL);

	vkDestroyDevice(app->device, NULL);

	vkDestroySurfaceKHR(app->instance,app->surface,NULL);
	vkDestroyInstance(app->instance,NULL);
        glfwDestroyWindow(app->window);

        glfwTerminate();
}
void recreateSwapChain(struct application *app)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(app->window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(app->window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(app->device);
	cleanupSwapChain(app);
	createSwapChain(app);
	createImageViews(app);
	createRenderPass(app);
	createGraphicsPipeline(app);
	createFramebuffers(app);
	createUniformBuffers(app);
	createCommandBuffers(app);
}
void cleanupSwapChain(struct application *app) {
	int i;
	for(i=0;i<app->imageCount;i++){
		vkDestroyFramebuffer(app->device,app->swapChainFramebuffers[i],NULL);
	}

	vkFreeCommandBuffers(app->device, app->commandPool, app->imageCount, app->commandBuffers);

	vkDestroyPipeline(app->device, app->graphicsPipeline, NULL);
	vkDestroyPipelineLayout(app->device, app->pipelineLayout, NULL);
	vkDestroyRenderPass(app->device, app->renderPass, NULL);

	for(i=0;i<app->imageCount;i++){
		vkDestroyImageView(app->device,app->swapChainImageViews[i],NULL);
	}

	vkDestroySwapchainKHR(app->device, app->swapChain, NULL);
	for(i=0;i<app->imageCount;i++){
		vkDestroyBuffer(app->device,app->uniformBuffers[i],NULL);
		vkFreeMemory(app->device,app->uniformBuffersMemory[i],NULL);
	}
}
void createInstance(struct application *app) {
	if (enableValidationLayers && !checkValidationLayerSupport(app)) {
		printf("validation layers requested, but not available!\n");
		exit(1);
	}
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo;
	memset(&createInfo,0,sizeof(createInfo));

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	getRequiredExtensions(app);

	createInfo.enabledExtensionCount = app->enabled_extension_count;
	createInfo.ppEnabledExtensionNames = (const char * const*) app->extensions;

	createInfo.enabledLayerCount = 0;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	memset(&debugCreateInfo,0,sizeof(debugCreateInfo));
        if (enableValidationLayers) {
		createInfo.enabledLayerCount = ARRAY_SIZE(validationLayers);
		createInfo.ppEnabledLayerNames = validationLayers;
		populateDebugMessengerCreateInfo(&debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = NULL;
        }

	if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
		printf("failed to create instance!");
		exit(1);
	}
}

void mainLoop(struct application *app)
{
	while (!glfwWindowShouldClose(app->window)) {
		glfwPollEvents();
		drawFrame(app);
	}
	vkDeviceWaitIdle(app->device);
}
int main()
{
	struct application app;
	run(&app);
	return 0;
}

