#include "Game.hpp"
#include "../../RVEngine/include/Renderer/EnvironmentMap.hpp"
#include "../../RVEngine/include/Renderer/Renderer.hpp"
#include <glm/gtx/compatibility.hpp>

#include "Random.hpp"

Game::Game(const string& title, int width, int height) : Application(title, width, height)
{

}

void Game::OnInit()
{
	ResourceManager& rm = ResourceManager::instance();
//	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
//	mainShader = rm.GetShader("pbr");
	Renderer::SetClearColor({0, 0, 0, 1});
	m_FramebufferProps.width = GetWindowSize().x;
	m_FramebufferProps.height = GetWindowSize().y;
	m_FrameBuffer = std::make_shared<FrameBuffer>(m_FramebufferProps);
	m_LastViewportSize = glm::vec2{GetWindowSize().x, GetWindowSize().y};

	m_ActiveScene = std::make_shared<Scene>();

	LoadAssets();

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
	m_Camera.GetComponent<TransformComponent>().SetPosition({0,0,40});

	m_Player = m_ActiveScene->CreateEntity("Player");
	m_Player.AddComponent<MeshRendererComponent>();
	m_Player.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
	m_Player.GetComponent<MeshRendererComponent>().model = rm.GetModel("player_ship");
	m_Player.GetComponent<TransformComponent>().SetPosition({0,0,0});
	m_Player.GetComponent<TransformComponent>().SetRotation({0,0,0});
	m_Player.AddComponent<PlayerComponent>();

	SpawnEnemyWave(9, 8, false);



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

	targetPos = m_Player.GetComponent<TransformComponent>().GetPosition();


	m_ActiveScene->OnRuntimeStart();
}

void Game::OnUpdate()
{
	RV_PROFILE_FUNCTION();
//	Renderer::BeginScene();
	ProcessInput();
	OnResize();
	CheckPlayerUpgrades();
	glm::vec2 input{0,0};
//	if (m_Input.GetKeyDown(GLFW_KEY_W))
//		input.y = 1;
//	if (m_Input.GetKeyDown(GLFW_KEY_S))z
//		input.y = -1;
//	if (m_Input.GetKeyDown(GLFW_KEY_A))
//		input.x = -1;
//	if (m_Input.GetKeyDown(GLFW_KEY_D))
//		input.x = 1;

	glm::vec3 playerPos = m_Player.GetComponent<TransformComponent>().GetPosition();

	glm::vec2 directionToCursor = -m_CursorWorldPos - playerPos;

	// move if cursor is far enough
	if (glm::length(directionToCursor) > 0.1f)
	{
		// Normalize to get direction only
		input = glm::normalize(directionToCursor);
	}

	if (playerShoot)
	{
		playerShoot = 0.15;
		SpawnBullet();
		// m_ActiveScene->SetGravityFactor(bullet, 0.0f);

	}

	if (input != glm::vec2(0,0))
		input = glm::normalize(input);

	input *= GetDeltaTime();
	input *= m_MoveSpeed;

//	targetPos.x = -m_CursorWorldPos.x;
//	targetPos.y = -m_CursorWorldPos.y;


	targetPos += glm::vec3{input.x,input.y,0};
	m_Player.GetComponent<TransformComponent>().SetPosition(glm::lerp(m_Player.GetComponent<TransformComponent>().GetPosition(), targetPos, 0.25f));

	auto targetRot = glm::lerp(m_Player.GetComponent<TransformComponent>().GetRotation(), {0,input.x * 100,0}, 0.1f);
	m_Player.GetComponent<TransformComponent>().SetRotation(targetRot);
//	m_Camera.GetComponent<TransformComponent>().Translate({0, m_CameraMoveSpeed * GetDeltaTime(), 0});




	m_FrameBuffer->Bind();

	Renderer::SetClearColor({0, 0, 0, 1});
	Renderer::Clear();


	m_ActiveScene->OnUpdateRuntime(GetDeltaTime());
	{
		RV_PROFILE_SCOPE("Update Bullets");
		auto bullets = m_ActiveScene->GetEntitiesWithComponent<BulletComponent>();

		for (auto bullet : bullets)
		{
			auto& bulletComp = bullet.GetComponent<BulletComponent>();

			if (!bulletComp.Initialized)
			{
				m_ActiveScene->SetGravityFactor(bullet, 0.0f);
				m_ActiveScene->SetVelocity(bullet, bulletComp.Velocity);

				bulletComp.Initialized = true;
			}
			bulletComp.RemainingLifeTime -= GetDeltaTime();

			if (bulletComp.RemainingLifeTime <= 0)
			{
				bullet.Destroy();
			}
		}
	}

	{
		RV_PROFILE_SCOPE("Update Enemies");
		auto enemies = m_ActiveScene->GetEntitiesWithComponent<EnemyComponent>();
		std::vector<Entity> remainingEnemies;
		auto& pc = m_Player.GetComponent<PlayerComponent>();
		for (int i = enemies.size() - 1; i >= 0; --i)
		{
			Entity enemy = enemies[i];

				auto others = enemy.GetCollidingEntities();
				for (auto other : others)
				{
					if (other.IsValid())
					{
						if (other.GetComponent<TagComponent>().Tag == "Bullet")
						{
							auto& enemyComp = enemy.GetComponent<EnemyComponent>();
							auto& bulletComp = other.GetComponent<BulletComponent>();

							enemyComp.Health -= bulletComp.Damage;
							other.Destroy();

							if (enemyComp.Health <= 0)
							{
								pc.Money += enemyComp.MoneyDropAmount;
								enemy.Destroy();
								continue;
							}
						}
					}
			}
			auto& timer = enemy.GetComponent<EnemyComponent>().MovementTimer;
			auto& moveRight = enemy.GetComponent<EnemyComponent>().MoveRight;
			auto& moveSpeed = enemy.GetComponent<EnemyComponent>().Speed;

			auto enemyPos = m_ActiveScene->GetPhysicsPosition(enemy);
			if (timer >= 3.0f)
			{
				timer = 0;
				moveRight = !moveRight;
			}
				timer += GetDeltaTime();
			if (moveRight)
				enemyPos += glm::vec3{-moveSpeed * GetDeltaTime(),0,0};
			else
				enemyPos += glm::vec3{moveSpeed * GetDeltaTime(),0,0};

			m_ActiveScene->SetPhysicsPosition(enemy, enemyPos);

			remainingEnemies.push_back(enemy);
		}

		m_Enemies = std::move(remainingEnemies);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffer->GetFBO());
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
	auto& pc = m_Player.GetComponent<PlayerComponent>();
	ImGui::Begin("Debug");
	ImGui::Text("Money: %i", pc.Money);
	ImGui::Text("Engines: %i", pc.EnginesLevel);
	if (ImGui::Button("Upgrade engines"))
	{
		if (pc.EnginesLevel < 5)
		{
			pc.EnginesLevel++;
		}
	}

	uint32_t entities = m_ActiveScene->GetEntityCount();
	ImGui::Text("Entities %u", entities);
	uint32_t drawCalls = RenderStats::GetInstance().DrawCalls;
	ImGui::Text("Draw calls %u", drawCalls);
	float frameTime = GetDeltaTime();
	ImGui::Text("Frame time %.2f ms", frameTime * 1000);
	float gpuTime = static_cast<float>(Renderer::GetTimeElapsed()) / 1000000.0f;
	ImGui::Text("GPU time %.2f ms", gpuTime);


	glm::vec3 worldPos = m_ActiveScene->ScreenToWorld(m_Input.GetMousePos(), m_Camera.GetComponent<TransformComponent>().GetPosition(), glm::vec3(0,0,-1));
	m_CursorWorldPos = worldPos + m_Camera.GetComponent<TransformComponent>().GetPosition();
	ImGui::Text("Ray X: %f", worldPos.x);
	ImGui::Text("Ray Y: %f", worldPos.y);
	ImGui::Text("Ray Z: %f", worldPos.z);
	ImGui::End();

	ImGui::Begin("Enemies");
	ImGui::Separator();
	ImGui::Text("Enemies %u", m_Enemies.size());
	for (int i = 0; i < m_Enemies.size(); ++i)
	{
		std::string str = m_Enemies.at(i).GetComponent<TagComponent>().Tag;
		str += ": ";
		str += std::to_string(m_Enemies.at(i).GetComponent<EnemyComponent>().Health);
		ImGui::Text(str.c_str());
	}
	ImGui::End();


	auto bullets = m_ActiveScene->GetEntitiesWithComponent<BulletComponent>();
	ImGui::Begin("Bullets");
	ImGui::Separator();
	ImGui::Text("Bullets %u", bullets.size());
	for (int i = 0; i < bullets.size(); ++i)
	{
		std::string str = bullets.at(i).GetComponent<TagComponent>().Tag;
		str += ": ";
		str += std::to_string(bullets.at(i).GetComponent<TransformComponent>().GetPosition().y);
		str += " | lifetime: ";
		str += std::to_string(bullets.at(i).GetComponent<BulletComponent>().RemainingLifeTime);
		str += " | body: ";
		str += std::to_string(bullets.at(i).GetComponent<BoxColliderComponent>().IndexSequence);
		ImGui::Text(str.c_str());
	}
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
		m_FramebufferProps.width = width;
		m_FramebufferProps.height = height;

		m_FrameBuffer = std::make_shared<FrameBuffer>(m_FramebufferProps);
		m_ActiveScene->SetViewportSize(width,height);
		m_LastViewportSize = glm::vec2{width, height};
	}
}

Entity Game::SpawnBullet()
{
	ResourceManager& rm = ResourceManager::instance();
	auto bullet = m_ActiveScene->CreateEntity("Bullet");

	bullet.AddComponent<BoxColliderComponent>().MotionType = RV::EMotionType::Dynamic;
	bullet.GetComponent<BoxColliderComponent>().CollisionLayer = RV::ECollisionLayer::Bullet;

	bullet.AddComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
	bullet.GetComponent<MeshRendererComponent>().model = rm.GetModel("bullet");

	bullet.GetComponent<TransformComponent>().SetRotation({0,0,0});
	bullet.GetComponent<TransformComponent>().SetPosition(m_Player.GetComponent<TransformComponent>().GetPosition());

	bullet.AddComponent<BulletComponent>().Velocity = {0,m_BulletSpeed,0};
	bullet.GetComponent<BulletComponent>().Damage = 10;
	bullet.GetComponent<BulletComponent>().LifeTime = 2;
	bullet.GetComponent<BulletComponent>().RemainingLifeTime = 2;
	bullet.GetComponent<BulletComponent>().Friendly = true;
	return bullet;
}

void Game::LoadAssets()
{
	ResourceManager& rm = ResourceManager::instance();

	rm.AddModel("player_ship", std::make_shared<Model>("res/playership.glb"));
	rm.AddModel("bullet", std::make_shared<Model>("res/bullet.glb"));
}

void Game::SpawnEnemyWave(int enemies, float yPos, bool centered)
{
	ResourceManager& rm = ResourceManager::instance();

	int enemyNum = enemies;
	for (int i = 0; i < enemyNum; ++i)
	{
		auto enemy = m_ActiveScene->CreateEntity("taxman_" + std::to_string(i));
		enemy.AddComponent<MeshRendererComponent>();
		enemy.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
		enemy.GetComponent<MeshRendererComponent>().model = rm.GetModel("cube");
		if (centered)
		{
			float totalWidth = (enemyNum - 1) * 3;
			float startX = -totalWidth / 2.0f;
			enemy.GetComponent<TransformComponent>().SetPosition({startX + i * 3,yPos , 0});
		}
		else
		{
			enemy.GetComponent<TransformComponent>().SetPosition({0 + i * 3, yPos, 0});
		}


		enemy.GetComponent<TransformComponent>().SetRotation({0,0,0});
		auto& ec = enemy.AddComponent<EnemyComponent>();
		ec.MoneyDropAmount = Random::Range(ec.MoneyDropMin, ec.MoneyDropMax);
		enemy.AddComponent<BoxColliderComponent>().MotionType = RV::EMotionType::Kinematic;
		enemy.GetComponent<BoxColliderComponent>().CollisionLayer = RV::ECollisionLayer::Enemy;
	}

}

void Game::CheckPlayerUpgrades()
{
	auto& pc = m_Player.GetComponent<PlayerComponent>();
	switch (pc.EnginesLevel)
	{
		case 0:
			m_MoveSpeed = pc.MovementSpeedLevel0;
			break;
		case 1:
			m_MoveSpeed = pc.MovementSpeedLevel1;
			break;
		case 2:
			m_MoveSpeed = pc.MovementSpeedLevel2;
			break;
		case 3:
			m_MoveSpeed = pc.MovementSpeedLevel3;
			break;
		case 4:
			m_MoveSpeed = pc.MovementSpeedLevel4;
			break;
		case 5:
			m_MoveSpeed = pc.MovementSpeedLevel5;
			break;
		default:
			m_MoveSpeed = pc.MovementSpeedLevel0;
			break;
	}
}
