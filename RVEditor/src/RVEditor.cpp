#include "RVEditor.hpp"
#include "Renderer.hpp"
#include "Macros.hpp"
#include "Components.hpp"
#include "SceneSerializer.hpp"
#include "portable-file-dialogs.h"

RVEditor::RVEditor(const std::string &title, int width, int height) : Application(title, width, height)
{

}

void RVEditor::OnInit()
{
	model = std::make_shared<Model>("/home/lolman/git/horror-assets/Models/ServiceBell/Export/ServiceBell_low.fbx");

	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
	mainShader = std::make_shared<Shader>("res/shaders/PBR_vert.glsl", "res/shaders/PBR_frag.glsl");

	frameBuffer = std::make_shared<FrameBuffer>(GetWindowSize().x,GetWindowSize().y);
//	OpenScene("res/scenes/dupa.rvscene");
	m_EditorScene = std::make_shared<Scene>();
	m_ActiveScene = m_EditorScene;

	m_SceneHierarchyPanel.SetContext(m_ActiveScene);


	ResourceManager& rm = ResourceManager::instance();

	auto cerberus = std::make_shared<Model>("/home/lolman/Downloads/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	rm.AddModel("cerberus", cerberus);


	std::shared_ptr<Material> cerbMat = std::make_shared<Material>() ;
	rm.AddMaterial("cerberus", cerbMat);
	rm.AddTexture("cerberus_albedo",std::make_shared<Texture>(Texture("/home/lolman/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.png")));
	rm.AddTexture("cerberus_normal",std::make_shared<Texture>(Texture("/home/lolman/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.png")));
	rm.AddTexture("cerberus_orm",std::make_shared<Texture>(Texture("/home/lolman/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_ORM.png")));
	cerbMat->albedo = rm.GetTexture("cerberus_albedo");
	cerbMat->normal = rm.GetTexture("cerberus_normal");
	cerbMat->occlusionRoughnessMetallic = rm.GetTexture("cerberus_orm");

	cerberus->m_Material = cerbMat;

	auto swiatelko = m_ActiveScene->CreateEntity("light");
	swiatelko.AddComponent<LightComponent>();
	swiatelko.GetComponent<TransformComponent>().SetPosition({0, 0, 3});


	auto camera = m_ActiveScene->CreateEntity("Camera");
	camera.AddComponent<CameraComponent>();
	camera.GetComponent<TransformComponent>().SetPosition({-4.5,1.75,2});
	camera.GetComponent<TransformComponent>().SetRotation({-37,-67,16});
//	ResourceManager& rm = ResourceManager::instance();

//	auto spheremodel = rm.GetModel("sphere05");
//	spheremodel->m_Material = rm.GetMaterial("default_pbr");
//	auto sphere = m_ActiveScene->CreateEntity("Sphere");
//	sphere.AddComponent<MeshRendererComponent>(spheremodel, mainShader, flatShader);
//	sphere.GetComponent<TransformComponent>().SetScale({1,1,1});
//	sphere.GetComponent<TransformComponent>().SetPosition({0,0,-0.035});

//	auto planeModel = rm.GetModel("plane");
//
//	planeModel->m_Material = rm.GetMaterial("default_pbr");
//	auto plane = m_ActiveScene->CreateEntity("Plane");
//	plane.AddComponent<MeshRendererComponent>(planeModel, mainShader, flatShader);
//	plane.GetComponent<TransformComponent>().SetScale({1, 1, 1});
//	plane.GetComponent<TransformComponent>().SetPosition({0, -3, -0.5});
//	plane.GetComponent<TransformComponent>().SetRotation({-90, 0, 0});
//	plane.AddComponent<BoxColliderComponent>();
//	plane.GetComponent<BoxColliderComponent>().Dynamic = false;
//	plane.GetComponent<BoxColliderComponent>().Scale = {1,0.1,1};

	auto cubeModel = rm.GetModel("cube");

	cubeModel->m_Material = rm.GetMaterial("default_pbr");
	auto cube = m_ActiveScene->CreateEntity("Cube ten na dole");
	cube.AddComponent<MeshRendererComponent>(cubeModel, mainShader, flatShader);
	cube.GetComponent<TransformComponent>().SetScale({1, 1, 1});
	cube.GetComponent<TransformComponent>().SetPosition({0, -3, 0});
	cube.GetComponent<TransformComponent>().SetRotation({0, 0, 0});
	cube.AddComponent<BoxColliderComponent>();
	cube.GetComponent<BoxColliderComponent>().Dynamic = false;
	cube.GetComponent<BoxColliderComponent>().Size = {1, 1, 1};

	auto cube2 = m_ActiveScene->CreateEntity("Cube");
	cube2.AddComponent<MeshRendererComponent>(cubeModel, mainShader, flatShader);
	cube2.GetComponent<TransformComponent>().SetScale({1, 1, 1});
	cube2.GetComponent<TransformComponent>().SetPosition({0, 0, 0});
	cube2.GetComponent<TransformComponent>().SetRotation({35, 0, 0});
	cube2.AddComponent<BoxColliderComponent>();
	cube2.GetComponent<BoxColliderComponent>().Dynamic = true;
	cube2.GetComponent<BoxColliderComponent>().Size = {1, 1, 1};

	auto cube3 = m_ActiveScene->CreateEntity("Cube");
	cube3.AddComponent<MeshRendererComponent>(cubeModel, mainShader, flatShader);
	cube3.GetComponent<TransformComponent>().SetScale({1, 1, 1});
	cube3.GetComponent<TransformComponent>().SetPosition({0, 3, 0});
	cube3.GetComponent<TransformComponent>().SetRotation({35, 0, 0});
	cube3.AddComponent<BoxColliderComponent>();
	cube3.GetComponent<BoxColliderComponent>().Dynamic = true;
	cube3.GetComponent<BoxColliderComponent>().Size = {1, 1, 1};

//	auto sprite = m_ActiveScene->CreateEntity("Sprite");
//	sprite.AddComponent<SpriteRendererComponent>(glm::vec4{1,1,1,1});
	m_ActiveScene->OnStart();

}

void RVEditor::OnUpdate()
{
	RV_PROFILE_FUNCTION();
	Renderer::BeginScene(m_Camera);
	ProcessInput();

	m_HoveredEntity = frameBuffer->GetEntityID({m_MouseVieportPos.x, m_MouseVieportPos.y});

	if (LeftClickedInViewport() && !ImGuizmo::IsUsing())
	{
		if(m_HoveredEntity < 4294967295)
		{

			m_ClickedEntity = m_HoveredEntity;
			m_SceneHierarchyPanel.SetSelectedEntity(Entity((entt::entity)m_ClickedEntity, m_ActiveScene.get()));
			m_ActiveScene->SetSelectedEntity(m_ClickedEntity);

		}
		else
		{
			m_ClickedEntity = entt::null;
			m_SceneHierarchyPanel.SetSelectedEntity(Entity((entt::entity)entt::null, m_ActiveScene.get()));
			m_ActiveScene->SetSelectedEntity(entt::null);
		}
	}



	frameBuffer->Bind();

	Renderer::SetClearColor({0, 0, 0, 1});
	Renderer::Clear();


	switch (m_SceneState)
	{
		case SceneState::Edit:
		{
			if (m_ViewportFocused)
			{
				glm::mat4 view;
				view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
				view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
				m_Camera.SetViewMatrix(view);

				mainShader->Bind();
				mainShader->SetVec3("u_CamPos", cameraPos);
				mainShader->SetUInt("u_DisplayType", m_DisplayType);
			}
			m_ActiveScene->OnUpdateEditor(GetDeltaTime(), m_Camera);

			break;
		}
		case SceneState::Simulate:
		{
//			m_EditorCamera.OnUpdate(ts);
//
//			m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
			break;
		}
		case SceneState::Play:
		{
			m_ActiveScene->OnUpdateRuntime(GetDeltaTime());

			break;
		}
	}

	frameBuffer->Unbind();
}

void RVEditor::OnImGuiRender()
{
	DrawImGui();
	m_SceneHierarchyPanel.OnRender();
	m_ContentBrowserPanel.OnRender();
}

void RVEditor::OnShutdown()
{

}

void RVEditor::DrawImGui()
{
	RV_PROFILE_FUNCTION();
	Dockspace();

	ImGui::Begin("Settings");

	if (ImGui::TreeNodeEx("Debug", ImGuiTreeNodeFlags_DefaultOpen))
	{

		ImGui::Text("FPS: %f", GetFPS());
		float mouseDelta[2];
		mouseDelta[0] = m_Input.GetMouseDelta().x;
		mouseDelta[1] = m_Input.GetMouseDelta().y;
		ImGui::InputFloat2("Mouse Delta", mouseDelta);
		ImGui::InputFloat("Pitch", &m_CameraPitch);
		ImGui::InputFloat("Yaw", &m_CameraYaw);
		ImGui::InputFloat2("Viewport cursor pos", &m_MouseVieportPos[0]);
		ImGui::Text("Hovered entity: %u", m_HoveredEntity);

		ImGui::DragScalar("Visualizer:", ImGuiDataType_U32, &m_DisplayType);

		std::string drawCalls = "Draw calls: ";
		drawCalls.append(std::to_string(RenderStats::GetInstance().DrawCalls));
		ImGui::Text("%s", drawCalls.c_str());
		ImGui::TreePop();
	}

	ImGui::End();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		ImGui::Text("Gizmo: ");

		bool translate = false;
		bool rotate = false;
		bool scale = false;

		if (m_GizmoType == ImGuizmo::OPERATION::TRANSLATE)
			translate = true;
		if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
			rotate = true;
		if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
			scale = true;

		if(ImGui::RadioButton("T", translate))
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;

		if(ImGui::RadioButton("R", rotate))
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;

		if(ImGui::RadioButton("S", scale))
			m_GizmoType = ImGuizmo::OPERATION::SCALE;

		ImGui::Text("|");

		if(m_SceneState == SceneState::Edit)
		{
			if (ImGui::Button("Play"))
			{
				OnScenePlay();
			}
		}
		else
		{
			if (ImGui::Button("Stop"))
			{
				OnSceneStop();
			}
		}
		ImGui::PushItemWidth(75);
		ImGui::DragFloat("Camera speed", &m_CameraSpeed);


		ImGui::EndMenuBar();
	}

	if (ImGui::IsWindowFocused() || GetCursorState() == GLFW_CURSOR_DISABLED)
		m_ViewportFocused = true;
	else
		m_ViewportFocused = false;



	m_ViewportSize = ImGui::GetContentRegionAvail();

	if (m_ViewportSize.x != m_LastViewportSize.x || m_ViewportSize.y != m_LastViewportSize.y || m_LastWindowSize != GetWindowSize())
	{
		Renderer::SetViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y);
		m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		frameBuffer = std::make_shared<FrameBuffer>(m_ViewportSize.x, m_ViewportSize.y);

	}
	// Because I use the texture from OpenGL, I need to invert the V from the UV.

	ImGui::Image((ImTextureID)frameBuffer->GetColorTexture(), m_ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

	float posx = ImGui::GetCursorScreenPos().x - ImGui::GetMousePos().x;
	float posy = ImGui::GetCursorScreenPos().y - ImGui::GetMousePos().y;
	m_MouseVieportPos = ImVec2{-posx,posy};


	// Gizmos

	Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
	if (selectedEntity && m_GizmoType != -1)
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		glm::mat4 cameraView = m_Camera.GetViewMatrix();
		glm::mat4 cameraProjection = m_Camera.GetProjection();

		if (selectedEntity.HasComponent<TransformComponent>())
		{
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

			if(ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				tc.Translation = translation;
//			glm::vec3 deltaRot = rotation - tc.Rotation;
				tc.Rotation = rotation;
				tc.Scale = scale;
			}
		}



	}

	ImGui::End();
	m_LastViewportSize = m_ViewportSize;
	m_LastWindowSize = GetWindowSize();
	ImGui::PopStyleVar(1);

}

void RVEditor::ProcessInput()
{
	RV_PROFILE_FUNCTION();
	auto window = GetWindow();
	m_Input.UpdateMouseDelta();

	if(glfwGetKey(GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (ClickedInViewport())
	{
		m_ViewportFocused = true;
	}

	if (m_ViewportFocused)
	{
		float cameraSpeed = m_CameraSpeed * GetDeltaTime();

		if (m_Input.GetMouseDown(1) && m_SceneState == SceneState::Edit)
		{
			SetCursorState(GLFW_CURSOR_DISABLED);
			glm::vec2 mouseDelta = m_Input.GetMouseDelta();
			mouseDelta *= 0.12f;
			m_CameraYaw += mouseDelta.x;
			m_CameraPitch += mouseDelta.y;

			if(m_CameraPitch > 89.0f)
				m_CameraPitch = 89.0f;
			if(m_CameraPitch < -89.0f)
				m_CameraPitch = -89.0f;

			direction.x = cos(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
			direction.y = sin(glm::radians(m_CameraPitch));
			direction.z = sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
			cameraFront = glm::normalize(direction);


			if (m_Input.GetKeyDown(GLFW_KEY_LEFT_SHIFT))
				cameraSpeed *= 2;

			if (m_Input.GetKeyDown(GLFW_KEY_W))
				cameraPos += cameraSpeed * cameraFront;
			if (m_Input.GetKeyDown(GLFW_KEY_S))
				cameraPos -= cameraSpeed * cameraFront;
			if (m_Input.GetKeyDown(GLFW_KEY_A))
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			if (m_Input.GetKeyDown(GLFW_KEY_D))
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			if (m_Input.GetKeyDown(GLFW_KEY_E))
				cameraPos += cameraSpeed * cameraUp;
			if (m_Input.GetKeyDown(GLFW_KEY_Q))
				cameraPos -= cameraSpeed * cameraUp;

			if (m_Input.GetKeyPressed(GLFW_KEY_F1))
			{
				GLint mode[2], newMode;
				glGetIntegerv(GL_POLYGON_MODE, mode);

				if (mode[0] == GL_LINE)
				{
					newMode = GL_FILL;
				}
				else
				{
					newMode = GL_LINE;
				}

				glPolygonMode(GL_FRONT_AND_BACK, newMode);
			}

		} else
		{
			SetCursorState(GLFW_CURSOR_NORMAL);
			if (m_Input.GetKeyPressed(GLFW_KEY_W))
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			if (m_Input.GetKeyPressed(GLFW_KEY_E))
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			if (m_Input.GetKeyPressed(GLFW_KEY_R))
				m_GizmoType = ImGuizmo::OPERATION::SCALE;

			// Duplicate
			if (m_Input.GetKeyDown(GLFW_KEY_LEFT_CONTROL) && m_Input.GetKeyPressed(GLFW_KEY_D))
			{
				if (auto selected = m_SceneHierarchyPanel.GetSelectedEntity())
				{
					m_ActiveScene->DuplicateEntity(selected);
				}

			}

		}
	}



	m_Input.UpdateKeys();
}

void RVEditor::Dockspace()
{
	RV_PROFILE_FUNCTION();
	ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
	flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			 ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	const ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace Demo", nullptr, flags);
	ImGui::PopStyleVar(3);
	if ( ImGui::BeginMenuBar() )
	{
		if ( ImGui::BeginMenu( "File" ) )
		{
			if ( ImGui::MenuItem( "New scene" ) )
			{
				NewScene();
			}
			if ( ImGui::MenuItem( "Open scene..." ) )
			{
				auto selection = pfd::open_file("Select a file", ".",
												{"Scene files", "*.rvscene"},
												pfd::opt::none).result();

				std::cout << "Selected file: " << selection.at(0) << "\n";
				OpenScene(selection.at(0));

			}
			if ( ImGui::MenuItem( "Save" ) )
			{
			}
			if ( ImGui::MenuItem( "Save as..." ) )
			{
				SaveSceneAs();
			}
			if ( ImGui::MenuItem( "Exit" ) )
			{
			}
			ImGui::EndMenu();
		}
	}
	ImGui::EndMenuBar();
	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspace_id);
	ImGui::End();

}

bool RVEditor::ClickedInViewport()
{
	return LeftClickedInViewport() || RightClickedInViewport();
}

bool RVEditor::CursorInViewport() const
{
	if (m_MouseVieportPos.x > 0 && m_MouseVieportPos.x < m_ViewportSize.x && m_MouseVieportPos.y > 0 && m_MouseVieportPos.y < m_ViewportSize.y)
		return true;
	else
		return false;
}

bool RVEditor::LeftClickedInViewport()
{
	return m_Input.GetMouseDown(0) && CursorInViewport();
}

bool RVEditor::RightClickedInViewport()
{
	return m_Input.GetMouseDown(1) && CursorInViewport();
}

void RVEditor::NewScene()
{
	m_SceneHierarchyPanel.SetSelectedEntity(Entity(entt::null, m_ActiveScene.get()));
	m_ActiveScene = std::make_shared<Scene>();
	m_SceneHierarchyPanel.SetContext(m_ActiveScene);
//	m_Serializer.SetContext(m_ActiveScene);
}

std::filesystem::path RVEditor::SaveSceneAs()
{
	std::filesystem::path selection = pfd::save_file("Select a file", ".",
									{"Scene files", "*.rvscene"},
									pfd::opt::none).result();

	if (selection.extension() != ".rvscene")
	{
		selection.append(".rvscene");
	}

	std::cout << "Saved file: " << selection << "\n";

	SceneSerializer serializer(m_ActiveScene);
	serializer.Serialize(selection);
	return selection;
}

void RVEditor::OpenScene(const std::filesystem::path& path)
{
	std::shared_ptr newScene = std::make_shared<Scene>();
	SceneSerializer serializer(newScene);
	if(serializer.Deserialize(path))
	{
		m_EditorScene = newScene;
//		m_SceneHierarchyPanel.SetSelectedEntity(Entity(entt::null, m_EditorScene.get()));
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_ActiveScene = m_EditorScene;


	} else
	{
		std::cout << "Failed to deserialize scene" << std::endl;
	}
}

void RVEditor::OnScenePlay()
{
	if (m_SceneState == SceneState::Simulate)
		OnSceneStop();

	m_SceneState = SceneState::Play;

	m_ActiveScene = Scene::Copy(m_EditorScene);
	m_ActiveScene->OnRuntimeStart();

	m_SceneHierarchyPanel.SetContext(m_ActiveScene);
}

void RVEditor::OnSceneStop()
{

	if (m_SceneState == SceneState::Play)
		m_ActiveScene->OnRuntimeStop();

	m_SceneState = SceneState::Edit;

	m_ActiveScene = m_EditorScene;

	m_SceneHierarchyPanel.SetContext(m_ActiveScene);
}
