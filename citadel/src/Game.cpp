#include "Game.hpp"
#include "EnvironmentMap.hpp"
#include "Renderer.hpp"
#include <glm/gtx/compatibility.hpp>
Game::Game(const string& title, int width, int height) : Application(title, width, height)
{

}

void Game::OnInit()
{
	ResourceManager& rm = ResourceManager::instance();
//	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
//	mainShader = rm.GetShader("pbr");
	Renderer::SetClearColor({0, 0, 0, 1});

	frameBuffer = std::make_shared<FrameBuffer>(GetWindowSize().x,GetWindowSize().y);
	m_LastViewportSize = glm::vec2{GetWindowSize().x, GetWindowSize().y};

	m_ActiveScene = std::make_shared<Scene>();
	EnvironmentMap envMap("res/overcast_soil_puresky_4k.hdr");
	envMap.Capture();


//	m_SceneHierarchyPanel.SetContext(m_ActiveScene);


	auto skybox = m_ActiveScene->CreateEntity("skybox");
	skybox.AddComponent<SkyboxComponent>().envCubemap = envMap.envCubemap;
	skybox.GetComponent<SkyboxComponent>().irradianceMap = envMap.irradianceMap;
	skybox.GetComponent<SkyboxComponent>().prefilterMap = envMap.prefilterMap;
	skybox.GetComponent<SkyboxComponent>().brdfLUTTexture = envMap.brdfLUTTexture;

	m_Camera = m_ActiveScene->CreateEntity("Camera");
	m_Camera.AddComponent<CameraComponent>();
	m_Camera.GetComponent<TransformComponent>().SetPosition({0,0,16});

	m_Player = m_ActiveScene->CreateEntity("Player");
	m_Player.AddComponent<MeshRendererComponent>();
	m_Player.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
	m_Player.GetComponent<MeshRendererComponent>().model = rm.GetModel("cube");
	m_Player.GetComponent<TransformComponent>().SetPosition({0,0,0});
//	rm.GetModel("cube")->m_Material = rm.GetMaterial("brickwall");



//	auto grid = m_ActiveScene->CreateEntity("grid");
//	grid.AddComponent<MeshRendererComponent>();
//	grid.GetComponent<MeshRendererComponent>().shader = rm.GetShader("grid");
//	grid.GetComponent<MeshRendererComponent>().model = rm.GetModel("plane");
//	grid.GetComponent<TransformComponent>().SetRotation({180,0,0});
//	grid.GetComponent<TransformComponent>().SetPosition({0,0,0});
//	grid.GetComponent<TransformComponent>().SetScale({100,100,1});
//	rm.GetModel("plane")->m_Material = rm.GetMaterial("grid");




	m_ActiveScene->OnStart();
	Renderer::SetViewport(0, 0, GetWindowSize().x,GetWindowSize().y);

	m_ActiveScene->SetViewportSize(GetWindowSize().x, GetWindowSize().y);




	m_ActiveScene->OnRuntimeStart();
}

void Game::OnUpdate()
{
	RV_PROFILE_FUNCTION();
//	Renderer::BeginScene();
	ProcessInput();
	OnResize();

	glm::vec2 input{0,0};
	if (m_Input.GetKeyDown(GLFW_KEY_W))
		input.y = 1;
	if (m_Input.GetKeyDown(GLFW_KEY_S))
		input.y = -1;
	if (m_Input.GetKeyDown(GLFW_KEY_A))
		input.x = -1;
	if (m_Input.GetKeyDown(GLFW_KEY_D))
		input.x = 1;

	input *= GetDeltaTime();
	input *= m_MoveSpeed;

	glm::vec3 pos = m_Player.GetComponent<TransformComponent>().GetPosition();
	m_Player.GetComponent<TransformComponent>().SetPosition(pos + glm::vec3{input.x,input.y,0});


	frameBuffer->Bind();

	Renderer::SetClearColor({0, 0, 0, 1});
	Renderer::Clear();


	m_ActiveScene->OnUpdateRuntime(GetDeltaTime());


	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer->GetFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, GetWindowSize().x, GetWindowSize().y, 0, 0,
					  GetWindowSize().x, GetWindowSize().y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

//	frameBuffer->Unbind();
//	Renderer::EndScene();
}

void Game::OnShutdown()
{

}

void Game::OnImGuiRender()
{
	RV_PROFILE_FUNCTION();

	ImGui::Begin("Debug");

	uint32_t entities = m_ActiveScene->GetEntityCount();
	ImGui::Text("Entities %u", entities);
	uint32_t drawCalls = RenderStats::GetInstance().DrawCalls;
	ImGui::Text("Draw calls %u", drawCalls);
	float frameTime = GetDeltaTime();
	ImGui::Text("Frame time %.2f ms", frameTime * 1000);
	float gpuTime = static_cast<float>(Renderer::GetTimeElapsed()) / 1000000.0f;
	ImGui::Text("GPU time %.2f ms", gpuTime);
	ImGui::End();


}

void Game::ProcessInput()
{
	RV_PROFILE_FUNCTION();
	auto window = GetWindow();
	m_Input.UpdateMouseDelta();

	if(glfwGetKey(GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	m_Input.UpdateKeys();
}

void Game::OpenScene(const filesystem::path& path)
{
	std::shared_ptr newScene = std::make_shared<Scene>();
	SceneSerializer serializer(newScene);
	if(serializer.Deserialize(path))
	{
//		m_EditorScene = newScene;
//		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_ActiveScene = newScene;
		m_SavedScenePath = path;
//
//		UpdateWindowTitle();

	} else
	{
		std::cout << "Failed to deserialize scene" << std::endl;
	}
}

void Game::OpenProject(const filesystem::path& path)
{
//	m_ProjectSettings.Deserialize(path);
//	UpdateWindowTitle();
//	m_AssetsPanel.SetAssetDirectory(path.parent_path() /= m_ProjectSettings.ResourcesDirectory);
}

void Game::OnResize()
{
	uint32_t width = GetWindowSize().x;
	uint32_t height = GetWindowSize().y;
	if (m_LastViewportSize != glm::vec2{width, height})
	{

		frameBuffer = std::make_shared<FrameBuffer>(width, height);
		m_ActiveScene->SetViewportSize(width,height);
		m_LastViewportSize = glm::vec2{width, height};
	}
}
