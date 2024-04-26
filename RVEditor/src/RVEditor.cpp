#include "RVEditor.hpp"
#include "Renderer.hpp"
#include "Macros.hpp"
#include "Components.hpp"
#include "SceneSerializer.hpp"
#include "portable-file-dialogs.h"
#include "EnvironmentMap.hpp"
#include "roboto.cpp"
#include "fa-solid-900.cpp"
#include "IconsFontAwesome6.h"

RVEditor::RVEditor(const std::string &title, int width, int height) : Application(title, width, height)
{
	m_IsEditor = true;
}

void RVEditor::OnInit()
{
	float fontSize = 15;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(roboto_compressed_data, roboto_compressed_size, fontSize);


	ImGuiIO& io = ImGui::GetIO();
//	io.Fonts->AddFontDefault();
	float baseFontSize = fontSize; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &icons_config, icons_ranges);


	ResourceManager& rm = ResourceManager::instance();

	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
	mainShader = rm.GetShader("pbr");

	frameBuffer = std::make_shared<FrameBuffer>(GetWindowSize().x,GetWindowSize().y);
	m_EditorScene = std::make_shared<Scene>();
	m_ActiveScene = m_EditorScene;

	EnvironmentMap envMap("res/buikslotermeerplein_4k.hdr");
	envMap.Capture();


	m_SceneHierarchyPanel.SetContext(m_ActiveScene);


	auto skybox = m_ActiveScene->CreateEntity("skybox");
	skybox.AddComponent<SkyboxComponent>().envCubemap = envMap.envCubemap;
	skybox.GetComponent<SkyboxComponent>().irradianceMap = envMap.irradianceMap;
	skybox.GetComponent<SkyboxComponent>().prefilterMap = envMap.prefilterMap;
	skybox.GetComponent<SkyboxComponent>().brdfLUTTexture = envMap.brdfLUTTexture;

	auto shader = rm.GetShader("pbr");

	auto plane = m_ActiveScene->CreateEntity("cube");
	plane.AddComponent<MeshRendererComponent>();
	plane.GetComponent<MeshRendererComponent>().shader = shader;
	plane.GetComponent<MeshRendererComponent>().model = rm.GetModel("cube");
	plane.GetComponent<TransformComponent>().SetPosition({2,0,0});
	rm.GetModel("cube")->m_Material = rm.GetMaterial("brickwall");

	auto sphere = m_ActiveScene->CreateEntity("sphere");
	sphere.AddComponent<MeshRendererComponent>();
	sphere.GetComponent<MeshRendererComponent>().shader = shader;
	sphere.GetComponent<MeshRendererComponent>().model = rm.GetModel("sphere05");
	rm.GetModel("sphere05")->m_Material = rm.GetMaterial("brickwall");

	m_ActiveScene->OnStart();

}

void RVEditor::OnUpdate()
{
	RV_PROFILE_FUNCTION();
	Renderer::BeginScene(m_Camera);
	ProcessInput();

	if (m_AppData.fileDropped)
	{
		m_AppData.fileDropped = false;

		if(m_ProjectSettings.ProjectPath.empty())
			std::cout << "No project opened!" << std::endl;
		else
			m_AssetImporterModal.Open(&m_AppData, &m_ProjectSettings, m_AssetsPanel.GetCurrentDirectory());
	}

	m_HoveredEntity = frameBuffer->GetEntityID({m_MouseVieportPos.x, m_MouseVieportPos.y});

	if (LeftClickedInViewport() && !ImGuizmo::IsUsing() && m_SceneState == SceneState::Edit && !IsAnyPopupOpen())
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
	Renderer::EndScene();
}

void RVEditor::OnImGuiRender()
{
	DrawImGui();
	m_SceneHierarchyPanel.OnRender();
	m_AssetsPanel.OnRender();
	m_AssetImporterModal.Render();
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

		ImGui::Text("Draw calls: %d", RenderStats::GetInstance().DrawCalls);

		ImGui::Text("%u tris", Renderer::GetPrimitivesGenerated());
		ImGui::Text("Gizmo: %d", m_GizmoType);

		float timeElapsedInMilliseconds = static_cast<float>(Renderer::GetTimeElapsed()) / 1000000.0f;
		ImGui::Text("%f ms", timeElapsedInMilliseconds);
		ImGui::TreePop();
	}

	if (!m_ProjectSettings.ProjectName.empty())
	{
		if (ImGui::TreeNodeEx("Project", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputText("Project name", &m_ProjectSettings.ProjectName);
			if (ImGui::Button("Save project"))
			{
				m_ProjectSettings.Serialize(m_ProjectSettings.ProjectPath);
				UpdateWindowTitle();
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		float avail = ImGui::GetWindowContentRegionWidth();

		const char* items[] = {"Translate", "Rotate", "Scale"};

		if (m_GizmoType == ImGuizmo::OPERATION::TRANSLATE)
			m_SelectedGizmo = 0;
		if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
			m_SelectedGizmo = 1;
		if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
			m_SelectedGizmo = 2;

		ImGui::PushItemWidth(ImGui::CalcTextSize("Translate").x*2);
		ImGui::Combo("Gizmo", &m_SelectedGizmo, items, IM_ARRAYSIZE(items));


		switch (m_SelectedGizmo)
		{
			case 0:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case 1:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case 2:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
		}

		float width = 0.0f;
		width += ImGui::CalcTextSize(ICON_FA_PLAY).x;
		ImGui::AlignForWidth(width, 0.5, avail);
		if(m_SceneState == SceneState::Edit)
		{
			if (ImGui::Button(ICON_FA_PLAY))
			{
				OnScenePlay();
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_STOP))
			{
				OnSceneStop();
			}
		}

		ImGui::AlignForWidth(ImGui::CalcTextSize("Camera speed").x + 85, 1, avail);
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

	if (ClickedInViewport() && !IsAnyPopupOpen())
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

			m_CameraSpeed += m_Input.GetScrollDelta();
			m_CameraSpeed = std::clamp(m_CameraSpeed, 0.0f, 100.0f);

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
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New project..."))
			{
				m_NewProjectModal.Open();
				std::cout << "New project menu item selected\n";
			}
			if (ImGui::MenuItem("Open project..."))
			{
				auto selection = pfd::open_file("Select a file", ".",
												{"Project files", "*.rvproj"},
												pfd::opt::none).result();

				std::cout << "Selected file: " << selection.at(0) << "\n";
				OpenProject(selection.at(0));
			}

			if (ImGui::MenuItem("New scene"))
			{
				NewScene();
			}
			if (ImGui::MenuItem("Open scene..."))
			{
				auto selection = pfd::open_file("Select a file", ".",
												{"Scene files", "*.rvscene"},
												pfd::opt::none).result();

				std::cout << "Selected file: " << selection.at(0) << "\n";
				OpenScene(selection.at(0));
			}
			if (ImGui::MenuItem("Save", nullptr, false, !m_SavedScenePath.empty()))
			{
				m_SavedScenePath = SaveScene();
			}
			if (ImGui::MenuItem("Save as..."))
			{
				m_SavedScenePath = SaveSceneAs();
			}
			if (ImGui::MenuItem("Exit"))
			{
				glfwSetWindowShouldClose(GetWindow(), true);
			}
			ImGui::EndMenu();
		}
	}
	m_NewProjectModal.Render();

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

std::filesystem::path RVEditor::SaveScene()
{
	SceneSerializer serializer(m_ActiveScene);
	serializer.Serialize(m_SavedScenePath);
	return m_SavedScenePath;
}


void RVEditor::OpenScene(const std::filesystem::path& path)
{
	std::shared_ptr newScene = std::make_shared<Scene>();
	SceneSerializer serializer(newScene);
	if(serializer.Deserialize(path))
	{
		m_EditorScene = newScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_ActiveScene = m_EditorScene;
		m_SavedScenePath = path;

		UpdateWindowTitle();

	} else
	{
		std::cout << "Failed to deserialize scene" << std::endl;
	}
}

void RVEditor::OpenProject(const std::filesystem::path& path)
{
	m_ProjectSettings.Deserialize(path);
	UpdateWindowTitle();
	m_AssetsPanel.SetAssetDirectory(path.parent_path() /= m_ProjectSettings.ResourcesDirectory);
}

void RVEditor::OnScenePlay()
{
	if (m_SceneState == SceneState::Simulate)
		OnSceneStop();

	m_SceneState = SceneState::Play;

	m_ActiveScene = Scene::Copy(m_EditorScene);
	m_ActiveScene->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
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

bool RVEditor::IsAnyPopupOpen()
{
	return m_NewProjectModal.IsOpen() || m_AssetImporterModal.IsOpen();
}

void RVEditor::UpdateWindowTitle()
{
	SetTitle("RVEditor - " + (!m_ProjectSettings.ProjectName.empty() ? m_ProjectSettings.ProjectName : "No project") + " - " + m_ActiveScene->GetName());
}
