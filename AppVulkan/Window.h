#pragma once

#include <stdio.h>
#include <GLFW/glfw3.h>
#include <string>

class Window
{
public:
	Window();

	int Initialise(std::string wName, const int width, const int height);

	float getBufferWidth() { return bufferWidth; }
	float getBufferHeigt() { return bufferHeight; }

	bool getShouldClose() { return glfwWindowShouldClose(window); }

	bool* getKeys() { return keys; }
	float getXChange();
	float getYchange();

	void swapBuffers() { glfwSwapBuffers(window); }//probably don't need this

	~Window();
	static bool mHandleMouse;

	GLFWwindow* window;
private:

	int width, height;
	int bufferWidth, bufferHeight;

	bool keys[1024];

	float lastX;
	float lastY;
	float xChange;
	float yChange;
	bool mouseFirstMoved;

	void createCallbacks();

	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
};