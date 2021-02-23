#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>

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
};
struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
};

void initWindow(struct application *app);
void run(struct application *app);
void initVulkan();
void cleanup(struct application *app);
void mainLoop(struct application *app);
void createInstance(struct application *app);
void createSwapChain(struct application *app);
void createImageViews(struct application *app);
void createGraphicsPipeline(struct application *app);
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

void createGraphicsPipeline(struct application *app)
{
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

        vkDestroyShaderModule(app->device, fragShaderModule, NULL);
        vkDestroyShaderModule(app->device, vertShaderModule, NULL);
}

VkShaderModule createShaderModule(struct application *app,char *filename)
{
	char *code = 0;
	long length;
	FILE *f = fopen (filename, "rb");
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

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	struct QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties *queueFamilies;
	queueFamilies = malloc(sizeof(*queueFamilies) * queueFamilyCount);
	if (!queueFamilies) {
		printf("Unable to allocate memory\n");
		exit(1);
	}
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
	free(queueFamilies);
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

	VkExtensionProperties *availableExtensions;
	availableExtensions = malloc(sizeof(*availableExtensions) * extensionCount);
	if (!availableExtensions) {
		printf("Unable to allocate memory\n");
		exit(1);
	}
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
	free(availableExtensions);
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
	FILE *f = fopen (filename, "rb");
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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
	if(!app->window){
		printf("Unable to create GLFW Window");
		exit(1);
	}
}
void initVulkan(struct application *app)
{
	createInstance(app);
	createSurface(app);
	pickPhysicalDevice(app);
	createLogicalDevice(app);
	createSwapChain(app);
	createImageViews(app);
	createGraphicsPipeline(app);
}
void cleanup(struct application *app)
{
	int i;
	for(i=0;i<app->imageCount;i++){
		vkDestroyImageView(app->device,app->swapChainImageViews[i],NULL);
	}
	vkDestroySwapchainKHR(app->device, app->swapChain, NULL);
	vkDestroyDevice(app->device, NULL);

	vkDestroySurfaceKHR(app->instance,app->surface,NULL);
	vkDestroyInstance(app->instance,NULL);
        glfwDestroyWindow(app->window);

        glfwTerminate();
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
	}
}
int main()
{
	struct application app;
	memset(&app, 0, sizeof(app));
	run(&app);
	return 0;
}

