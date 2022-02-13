#include "Window.h"
#include "Debug.h"

Window::Window()
	: m_WindowPtr(nullptr), m_Width(800), m_Height(600), m_BufferWidth(0), m_BufferHeight(0), m_HandleMouse(false),
	m_Keys(), m_MouseKeys(), m_MousePos()
{
	// to properly initialize keys
	NotifyFrameEnded();
}

int Window::Initialise(std::string wName, const int width, const int height)
{
	if (glfwInit() == GLFW_FALSE)
		return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_WindowPtr = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_WindowPtr, this);
	CreateCallbacks();
	return 1;
}

void Window::CreateCallbacks()
{
	glfwSetKeyCallback(m_WindowPtr, HandleKeys);
	glfwSetCursorPosCallback(m_WindowPtr, HandleMouse);
	glfwSetMouseButtonCallback(m_WindowPtr, HandleMouseButtons);
}

const std::array<KeyInput, 1024>& Window::GetKeys() const
{
	return m_Keys;
}

const std::array<KeyInput, 8>& Window::GetMouseKeys() const
{
	return m_MouseKeys;
}

glm::vec2 Window::GetMousePos() const
{
	return m_MousePos.m_MouseScreenSpacePos;
}

void Window::NotifyFrameEnded()
{
	// KeyInput::keyAction should be -1 and keyMod should be 0.
	// Find out how dangerous is to do this
	constexpr int clearVal = -1 << (sizeof(KeyInput) / 2);
	memset(m_Keys.data(), clearVal, m_Keys.size() * sizeof(KeyInput));
	memset(m_MouseKeys.data(), clearVal, m_MouseKeys.size() * sizeof(KeyInput));
}

//float Window::getXChange()
//{
//	//if (mHandleMouse)
//	//	return 0.0f;
//	float theChange = xChange;
//	xChange = 0.0f;
//	return theChange;
//}
//
//float Window::getYchange()
//{
//	//if (mHandleMouse)
//	//	return 0.0f;
//	float theChange = yChange;
//	yChange = 0.0f;
//	return theChange;
//}

void Window::HandleKeys(GLFWwindow* window, int key, int code, int action, int mod)
{
	// we set the user pointer, now we can get it so we can get the instance in a static func
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (theWindow == nullptr)
	{
		// should handle this properly somehow later
		Debug::LogMsg("Window pointer was lost.\n");
		exit(1);
	}

	// move this later somewhere
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		theWindow->m_HandleMouse = !theWindow->m_HandleMouse;
		if (theWindow->m_HandleMouse)
			glfwSetInputMode(theWindow->m_WindowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(theWindow->m_WindowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// TODO: find some function that does not let cursor go out of bounds
	}

	if (key >= 0 && key < 1024)
	{
		theWindow->m_Keys[key].keyAction = action;
		theWindow->m_Keys[key].keyMod = mod;
	}
}

void Window::HandleMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	theWindow->m_MousePos.m_MouseScreenSpacePos.x = glm::clamp((float)xPos, 0.0f, (float)theWindow->m_Width);
	theWindow->m_MousePos.m_MouseScreenSpacePos.y = glm::clamp((float)yPos, 0.0f, (float)theWindow->m_Height);;

	/*if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;*/
}

void Window::HandleMouseButtons(GLFWwindow* window, int button, int action, int mods)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (button >= 0 && button < 8)
	{
		theWindow->m_MouseKeys[button].keyAction = action;
		theWindow->m_MouseKeys[button].keyMod = mods;
	}
}

Window::~Window()
{
	glfwDestroyWindow(m_WindowPtr);
	glfwTerminate();
}