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

	struct application
	{
		GLFWwindow* window;
		VkInstance instance;
		int width;
		int height;
	};

	void initWindow(struct application* app);
	void run(struct application* app);
	void initVulkan();
	void cleanup(struct application* app);
	void mainLoop(struct application* app);
	void createInstance(struct application* app);

	void run(struct application* app)
	{
		initWindow(app);
		initVulkan(app);
		mainLoop(app);
		cleanup(app);
	}

	void initWindow(struct application* app)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
		if (!app->window) {
			printf("Unable to create GLFW Window");
			exit(1);
		}
	}
	void initVulkan(struct application* app)
	{
		createInstance(app);
	}
	void cleanup(struct application* app)
	{
		glfwDestroyWindow(app->window);

		glfwTerminate();
	}
	void createInstance(struct application* app) {
		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo;
		memset(&createInfo, 0, sizeof(createInfo));
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
			printf("failed to create instance!");
			exit(1);
		}
	}

	void mainLoop(struct application* app)
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

