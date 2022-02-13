#pragma once
#include "NonCopyable.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <array>

struct KeyInput
{
	// -1 for no action
	int16_t keyAction;
	int16_t keyMod;
};

struct MouseInput
{
	glm::vec2 m_MouseScreenSpacePos;
};

class Window : NonCopyable
{
public:
	Window();

	int Initialise(std::string wName, const int width, const int height);

	float GetBufferWidth() { return m_BufferWidth; }
	float GetBufferHeight() { return m_BufferHeight; }

	bool GetShouldClose() { return glfwWindowShouldClose(m_WindowPtr); }
	const std::array<KeyInput, 1024>& GetKeys() const;
	const std::array<KeyInput, 8>& GetMouseKeys() const;
	glm::vec2 GetMousePos() const;

	void NotifyFrameEnded();

/*	float GetXChange();
	float GetYchange()*/;

	void SwapBuffers() { glfwSwapBuffers(m_WindowPtr); }

	~Window();

	// Find out if I can make this private
	GLFWwindow* m_WindowPtr;
private:

	int m_Width, m_Height;
	int m_BufferWidth, m_BufferHeight;
	bool m_HandleMouse;

	std::array<KeyInput, 1024> m_Keys;
	std::array<KeyInput, 8> m_MouseKeys;
	MouseInput m_MousePos;

	//float lastX;
	//float lastY;
	//float xChange;
	//float yChange;
	//bool mouseFirstMoved;

	void CreateCallbacks();

	static void HandleKeys(GLFWwindow* window, int key, int code, int action, int mod);
	static void HandleMouse(GLFWwindow* window, double xPos, double yPos);
	static void HandleMouseButtons(GLFWwindow* window, int button, int action, int mods);
};