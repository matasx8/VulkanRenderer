#include "NonCopyable.h"
#include "glm/glm.hpp"

class Window;

class Input
{
public:
	Input(Window& window);

	void UpdateInput();

	// not sure if I want to keep these
	void SetHandleKeyboard(bool handleKeyboard);
	void SetHandleMouse(bool handleMouse);
	void SetHandleInput(bool handleInput);

	glm::vec2 GetMouseCoords() const;
	bool GetMouseKey(int key, int action) const;
	bool GetKey(int key, int action) const;

private:

	Window& m_Window;
	bool m_HandleMouse;
	bool m_HandleKeyboard;
};