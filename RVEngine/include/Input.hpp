#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>

class Input
{
public:
	Input(GLFWwindow* window);

	bool GetKeyDown(int key);
	bool GetKeyPressed(int key);
	bool GetKeyReleased(int key);
	bool GetMouseDown(int button);
	bool GetMouseUp(int button);
	glm::vec2 GetMousePos();
	void SetMousePos(glm::vec2 pos);
	glm::vec2 GetMouseDelta();
	float GetScrollDelta();
	void UpdateMouseDelta();
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void UpdateKeys();

private:
	GLFWwindow* m_Window = nullptr;
	glm::vec2 m_LastCursorPos = glm::vec2(0,0);
	glm::vec2 m_CursorDelta = glm::vec2(0,0);
	bool m_FirstUpdate = true;

	std::map<int, bool> m_Keys {};
	std::map<int, bool> m_LastKeys {};
};
