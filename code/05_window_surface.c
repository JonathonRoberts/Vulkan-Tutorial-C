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
void getRequiredExtensions(struct application *app);
bool checkValidationLayerSupport(struct application *app);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(enum VkDebugUtilsMessageSeverityFlagBitsEXT messageseverity,  unsigned int messageType,  const struct VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
void pickPhysicalDevice(struct application *app);
void createLogicalDevice(struct application *app);
bool isDeviceSuitable(VkPhysicalDevice device,VkSurfaceKHR surface);
struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface);
void createSurface(struct application *app);
void setupDebugMessenger(struct application *app);


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

	createInfo.enabledExtensionCount = 0;

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

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
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
}
void cleanup(struct application *app)
{
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

