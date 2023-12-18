#include <iostream>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <Application.hpp>
#include <Shader.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Renderer.hpp>
#include <ImGuizmo.h>


Application::Application(const std::string &title, int width, int height)
{
	m_Title = title;
	m_Width = width;
	m_Height = height;
	m_Vsync = true;
}

void Application::Run()
{
	if (!glfwInit())
	{
		std::cout << "Failed to init GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	printf("GLFW version: %s\n", glfwGetVersionString());
	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);

	if (m_Window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		return;
	}

	glfwSetWindowUserPointer(m_Window, reinterpret_cast<void *>(this));

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
	{
		auto* data = static_cast<Application*>(glfwGetWindowUserPointer(window));

		data->m_Width = width;
		data->m_Height = height;

		glViewport(0, 0, width, height);

		fmt::print("Resized: {} x {}\n", data->m_Width, data->m_Height);
	});

	glfwMakeContextCurrent(m_Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to init GLAD" << std::endl;
		return;
	}

//	fmt::print("OpenGL version: {}\n", *glGetString(GL_VERSION));
	glfwSwapInterval(m_Vsync);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	ImGui::StyleColorsDark();



	m_Input = Input(m_Window);

	glfwSetWindowUserPointer(m_Window, &m_Input);
	glfwSetKeyCallback(m_Window, Input::KeyCallback);

	ImGui_ImplGlfw_InitForOpenGL(GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");

	OnInit();
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	float clearColor[3] {1,0,1};

	Renderer::Init();

	while (!glfwWindowShouldClose(m_Window))
	{
		RV_PROFILE_FRAME();
		UpdateDeltaTime();

		Renderer::SetClearColor(glm::vec4{clearColor[0], clearColor[1], clearColor[2], 1});
		Renderer::Clear();

		{
			RV_PROFILE_SCOPE("OnImGuiRender");
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			OnImGuiRender();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		{
			RV_PROFILE_SCOPE("OnUpdate");
			OnUpdate();
		}


		{
			RV_PROFILE_SCOPE("Swap Buffers");
			glfwSwapBuffers(m_Window);
			glfwPollEvents();
		}

	}

	OnShutdown();
	Renderer::Shutdown();
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

GLFWwindow* Application::GetWindow()
{
	RV_PROFILE_FUNCTION();
	return m_Window;
}

void Application::SetWindowSize(glm::ivec2 size)
{
	glfwSetWindowSize(m_Window,size.x, size.y);
	glViewport(0, 0, size.x, size.y);
}

glm::ivec2 Application::GetWindowSize() const
{
	RV_PROFILE_FUNCTION();
	return {m_Width, m_Height};
}



float Application::GetDeltaTime() const
{
	RV_PROFILE_FUNCTION();
	return deltaTime;
}

void Application::UpdateDeltaTime()
{
	RV_PROFILE_FUNCTION();
	auto currentFrame = (float)GetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

double Application::GetTime()
{
	RV_PROFILE_FUNCTION();
	return glfwGetTime();
}

float Application::GetFPS() const
{
	RV_PROFILE_FUNCTION();
	return 1 / GetDeltaTime();
}

void Application::SetCursorState(int state)
{
	glfwSetInputMode(m_Window, GLFW_CURSOR, state);
}

int Application::GetCursorState()
{
	return glfwGetInputMode(m_Window, GLFW_CURSOR);
}

void Application::SetVSync(bool vsync)
{
	glfwSwapInterval(vsync);
}



