#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>

#include <GLFW/glfw3.h>
#define GLFW_INCLUDE_VULKAN

#define WIDTH 800
#define HEIGHT 600

struct application
{
	GLFWwindow *window;
	int width;
	int height;
};

void initWindow(struct application *app);
void run(struct application *app);
void initVulkan();
void cleanup(struct application *app);
void mainloop(struct application *app);

void run(struct application *app)
{
	initWindow(app);
	initVulkan(app);
	mainloop(app);
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
void initVulkan()
{
}
void cleanup(struct application *app)
{
        glfwDestroyWindow(app->window);

        glfwTerminate();
}

void mainloop(struct application *app)
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
	mainloop(&app);
	return 0;
}

