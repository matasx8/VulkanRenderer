#include "NonCopyable.h"
#include "glm/glm.hpp"
#include <queue>

class Window;

struct WaitForClickResults
{
	glm::vec2 m_ClickCoords;
	int m_CommandBufferToWaitFor;
};

class Input
{
public:
	Input(Window& window);

	void UpdateInput();

	// not sure if I want to keep these
	void SetHandleKeyboard(bool handleKeyboard);
	void SetHandleMouse(bool handleMouse);
	void SetHandleInput(bool handleInput);

	void RegisterClick(WaitForClickResults& clickInfo);
	void PopRegisteredClick();

	glm::vec2 GetMouseCoords() const;
	bool GetMouseKey(int key, int action) const;
	bool GetKey(int key, int action) const;
	const std::queue<WaitForClickResults>& GetRegisteredClicks() const;

private:

	Window& m_Window;
	bool m_HandleMouse;
	bool m_HandleKeyboard;

	std::queue<WaitForClickResults> m_RegisteredClicks;
};