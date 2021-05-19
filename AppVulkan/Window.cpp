#include "Window.h"

Window::Window()
{
	width = 0;
	height = 0;

	xChange = 0.0f;
	yChange = 0.0f;

	mHandleMouse = false;
}

int Window::Initialise(std::string wName, const int width, const int height)
{
    //create window here
    //initialize glfw
    //should check for error here later
	if (glfwInit() == GLFW_FALSE)
	{
		return -1;
	}

    //set glfw to not work iwth opengl
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);//this class
	createCallbacks();
	return 1;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(window, handleKeys);
	glfwSetCursorPosCallback(window, handleMouse);
}

float Window::getXChange()
{
	//if (mHandleMouse)
	//	return 0.0f;
	float theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

float Window::getYchange()
{
	//if (mHandleMouse)
	//	return 0.0f;
	float theChange = yChange;
	yChange = 0.0f;
	return theChange;
}
void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	//we set the user pointer, now we can get it so we can get the instance in a static func
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (theWindow == nullptr)
	{
		printf("Error! glfw window was null!");
		exit(123);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		//glfwSetWindowShouldClose(window, GL_TRUE);
		mHandleMouse = !mHandleMouse;
		if (mHandleMouse)
			glfwSetInputMode(theWindow->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(theWindow->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::handleMouse(GLFWwindow* window, double xPos, double yPos)
{
	if (mHandleMouse)
		return;

	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;

	//printf("x:%.6f, y:%.6f\n", theWindow->xChange, theWindow->yChange);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Window::mHandleMouse = true;