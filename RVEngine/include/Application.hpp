#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fmt/core.h>
#include <Input.hpp>

class Application
{
public:
	Application(const std::string& title, int width, int height);
	void Run();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnImGuiRender() = 0;
	virtual void OnShutdown() = 0;


	GLFWwindow* GetWindow();
	glm::ivec2 GetWindowSize() const;
	void SetWindowSize(glm::ivec2 size);
	float GetDeltaTime() const;
	static double GetTime();
	float GetFPS() const;
	void SetCursorState(int state);
	int GetCursorState();
	static void SetVSync(bool vsync);
	void SetTitle(const std::string& title);

	Input m_Input{nullptr};


private:
	std::string m_Title;
	int m_Width;
	int m_Height;
	bool m_Vsync;
	GLFWwindow* m_Window{};
	void UpdateDeltaTime();



	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame


};