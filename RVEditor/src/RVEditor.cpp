#include <imgui_internal.h>
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
	SetupImGuiTheme();

	ResourceManager& rm = ResourceManager::instance();

	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
	mainShader = rm.GetShader("pbr");

	frameBuffer = std::make_shared<FrameBuffer>(GetWindowSize().x,GetWindowSize().y);
	m_EditorScene = std::make_shared<Scene>();
	m_ActiveScene = m_EditorScene;

	EnvironmentMap envMap("res/buikslotermeerplein_4k.hdr");
	envMap.Capture();


	m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	m_AssetsPanel.SetContext(m_ActiveScene);


	auto skybox = m_ActiveScene->CreateEntity("skybox");
	skybox.AddComponent<SkyboxComponent>().envCubemap = envMap.envCubemap;
	skybox.GetComponent<SkyboxComponent>().irradianceMap = envMap.irradianceMap;
	skybox.GetComponent<SkyboxComponent>().prefilterMap = envMap.prefilterMap;
	skybox.GetComponent<SkyboxComponent>().brdfLUTTexture = envMap.brdfLUTTexture;
	skybox.GetComponent<SkyboxComponent>().envMap = "res/buikslotermeerplein_4k.hdr";

	auto shader = rm.GetShader("pbr");

//	auto cube = m_ActiveScene->CreateEntity("cube");
//	cube.AddComponent<MeshRendererComponent>();
//	cube.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
//	cube.GetComponent<MeshRendererComponent>().model = rm.GetModel("player_ship");
//	cube.GetComponent<TransformComponent>().SetPosition({0, 0, 0});
//	rm.GetModel("cube")->m_Material = rm.GetMaterial("brickwall");
//
//	auto sphere = m_ActiveScene->CreateEntity("sphere");
//	sphere.AddComponent<MeshRendererComponent>();
//	sphere.GetComponent<MeshRendererComponent>().shader = shader;
//	sphere.GetComponent<MeshRendererComponent>().model = rm.GetModel("sphere05");
//	sphere.GetComponent<TransformComponent>().SetPosition({2, 0, 0});
//	rm.GetModel("sphere05")->m_Material = rm.GetMaterial("brickwall");
//
//	sphere.SetParent(cube);
//	skybox.SetParent(cube);
//
//	std::cout << "Children: "  << cube.GetChildren().size() << std::endl;
//	std::cout << "Dupa: "  << cube.GetChild(0).GetComponent<TagComponent>().Tag << std::endl;


//	auto helmet = m_ActiveScene->CreateEntity("helmet");
//	helmet.AddComponent<MeshRendererComponent>();
//	helmet.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
//	helmet.GetComponent<MeshRendererComponent>().model = rm.GetModel("helmet");
//	helmet.GetComponent<TransformComponent>().SetPosition({0, 0, -5});
//	helmet.GetComponent<TransformComponent>().SetRotation({90, 0, 0});
//
//	auto boombox = m_ActiveScene->CreateEntity("boombox");
//	boombox.AddComponent<MeshRendererComponent>();
//	boombox.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
//	boombox.GetComponent<MeshRendererComponent>().model = rm.GetModel("boombox");
//	boombox.GetComponent<TransformComponent>().SetPosition({-2, 0, -5});
//	boombox.GetComponent<TransformComponent>().SetRotation({0, 180, 0});
//	boombox.GetComponent<TransformComponent>().SetScale({25, 25, 25});
//
//	auto sas = m_ActiveScene->CreateEntity("maxwell");
//	sas.AddComponent<MeshRendererComponent>();
//	sas.GetComponent<MeshRendererComponent>().shader = rm.GetShader("pbr");
//	sas.GetComponent<MeshRendererComponent>().model = rm.GetModel("maxwell");
//	sas.GetComponent<TransformComponent>().SetPosition({2, 0, -5});
//	sas.GetComponent<TransformComponent>().SetRotation({-90, -90, 0});
//	sas.GetComponent<TransformComponent>().SetScale({0.05, 0.05, 0.05});
//	sas.SetParent(cube);

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
		float avail = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

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
			glm::mat4 deltaTransform = glm::mat4(1);

			//TODO: Rotation is broken
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), glm::value_ptr(deltaTransform));

			if(ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
//				glm::mat4 t = tc.GetLocalTransform();
//				t = t * deltaTransform;
				Math::DecomposeTransform(transform, translation, rotation, scale);
//				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(t), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
//				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation = rotation;
				tc.Scale = scale;
				tc.IsDirty = true;

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
				if(!selection.empty())
				{
					std::cout << "Selected file: " << selection.at(0) << "\n";
					OpenProject(selection.at(0));
				}
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
				if(!selection.empty())
				{
					std::cout << "Selected file: " << selection.at(0) << "\n";
					OpenScene(selection.at(0));
				}
			}
			if (ImGui::MenuItem("Save", nullptr, false, !m_SavedScenePath.empty()))
			{
				m_SavedScenePath = SaveScene();
			}
			if (ImGui::MenuItem("Save as..."))
			{
				auto path = SaveSceneAs();
				if (!path.empty())
					m_SavedScenePath = path;
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

	if (m_FirstFrame)
	{
		ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

		auto dockViewport = ImGui::DockBuilderAddNode(dockspace_id);
		auto dockInspector = ImGui::DockBuilderSplitNode(dockViewport, ImGuiDir_Right, 0.20f, nullptr, &dockspace_id);
		auto dockAssets = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockViewport);
//		auto dockSettings = ImGui::DockBuilderAddNode(dockInspector);
		auto dockHierarchy = ImGui::DockBuilderSplitNode(dockViewport, ImGuiDir_Left, 0.15f, nullptr, &dockViewport);

		ImGui::DockBuilderDockWindow("Viewport", dockViewport);
		ImGui::DockBuilderDockWindow("Assets", dockAssets);
		ImGui::DockBuilderDockWindow("Inspector", dockInspector);
		ImGui::DockBuilderDockWindow("Scene hierarchy", dockHierarchy);
//		ImGui::DockBuilderDockWindow("Settings", dockSettings);

		ImGui::DockBuilderFinish(dockspace_id);
		m_FirstFrame = false;
	}



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
	m_AssetsPanel.SetContext(m_ActiveScene);
//	m_Serializer.SetContext(m_ActiveScene);
}

std::filesystem::path RVEditor::SaveSceneAs()
{
	std::filesystem::path selection = pfd::save_file("Select a file", ".",
									{"Scene files", "*.rvscene"},
									pfd::opt::none).result();

	if (selection.extension() != ".rvscene")
	{
		selection.replace_extension(".rvscene");
	}

	if (!selection.stem().empty() && selection != ".rvscene")
	{
		std::cout << "Saved file: " << selection << "\n";

		SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(selection);
	}
	else
	{
		return "";
	}

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
		m_AssetsPanel.SetContext(m_ActiveScene);
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

void RVEditor::SetupImGuiTheme()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(4.0f, 8.0f);
	style.WindowRounding = 0.0f;
//	style.WindowRounding = 7.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
//	style.ChildRounding = 4.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding =0.0f;
//	style.PopupRounding = 4.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(10.0f, 4.0f);
	style.FrameRounding = 0.0f;
//	style.FrameRounding = 3.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
	style.CellPadding = ImVec2(6.0f, 6.0f);
	style.IndentSpacing = 25.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 0.0f;
//	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.9200000166893005f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.6266094446182251f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.239999994635582f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3372549116611481f, 0.3372549116611481f, 0.3372549116611481f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4000000059604645f, 0.4000000059604645f, 0.4000000059604645f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5568627715110779f, 0.5568627715110779f, 0.5568627715110779f, 0.5400000214576721f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.3294117748737335f, 0.6666666865348816f, 0.8588235378265381f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3372549116611481f, 0.3372549116611481f, 0.3372549116611481f, 0.5400000214576721f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5568627715110779f, 0.5568627715110779f, 0.5568627715110779f, 0.5400000214576721f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.3600000143051147f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 0.3300000131130219f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.2899999916553497f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4000000059604645f, 0.4392156898975372f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.2899999916553497f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4000000059604645f, 0.4392156898975372f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 0.3600000143051147f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.3294117748737335f, 0.6666666865348816f, 0.8588235378265381f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.3499999940395355f);
}
