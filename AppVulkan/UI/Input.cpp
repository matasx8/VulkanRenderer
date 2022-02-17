#include "Input.h"
#include "Window.h"

Input::Input(Window& window)
	: m_Window(window)
{
}

void Input::UpdateInput()
{
	//m_Window.GetKeys();
}

void Input::SetHandleKeyboard(bool handleKeyboard)
{
	m_HandleKeyboard = handleKeyboard;
}

void Input::SetHandleMouse(bool handleMouse)
{
	m_HandleMouse = handleMouse;
}

void Input::SetHandleInput(bool handleInput)
{
	m_HandleKeyboard = handleInput;
	m_HandleMouse = handleInput;
}

void Input::RegisterClick(WaitForClickResults& clickInfo)
{
	m_RegisteredClicks.push(clickInfo);
}

glm::vec2 Input::GetMouseCoords() const
{
	return m_Window.GetMousePos();
}

bool Input::GetMouseKey(int key, int action) const
{
	const auto& keys = m_Window.GetMouseKeys();
	return static_cast<int>(keys[key].keyAction) == action;
}

bool Input::GetKey(int key, int action) const
{
	const auto& keys = m_Window.GetKeys();
	return static_cast<int>(keys[key].keyAction) == action;
}

const std::queue<WaitForClickResults>& Input::GetRegisteredClicks() const
{
	return m_RegisteredClicks;
}

void Input::PopRegisteredClick()
{
	m_RegisteredClicks.pop();
}
