#include<iostream>
//fuck.. try to set up validation
#include "VulkanRenderer.h"

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

void initWindow(std::string wName = "Test window", const int width = 800, const int height = 600)
{
	//initialize glfw
	//should check for error here later
	glfwInit();

	//set glfw to not work iwth opengl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main()
{
	initWindow();

	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}