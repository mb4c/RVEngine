#include "RVEditor.hpp"
#include "Renderer.hpp"
#include "Macros.hpp"
#include "Components.hpp"

RVEditor::RVEditor(const std::string &title, int width, int height) : Application(title, width, height)
{

}

void RVEditor::OnInit()
{
	model = std::make_shared<Model>("/home/lolman/git/horror-assets/Models/ServiceBell/Export/ServiceBell_low.fbx");
	light = std::make_shared<Model>("/home/lolman/monke.fbx");

	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
	mainShader = std::make_shared<Shader>("res/shaders/PBR_vert.glsl", "res/shaders/PBR_frag.glsl");

	frameBuffer = std::make_shared<FrameBuffer>(GetWindowSize().x,GetWindowSize().y);
	m_Scene = std::make_shared<Scene>();

	m_SceneHierarchyPanel.SetContext(m_Scene);

	auto dzwonek = m_Scene->CreateEntity("dzwonek");
	dzwonek.AddComponent<MeshRendererComponent>(model, mainShader, flatShader);
	dzwonek.GetComponent<TransformComponent>().Rotate({-90,0,0});

	auto dzwonek2 = m_Scene->CreateEntity("dzwonek2");
	dzwonek2.AddComponent<MeshRendererComponent>(model, mainShader, flatShader);
	dzwonek2.GetComponent<TransformComponent>().Rotate({-90,0,0});
	dzwonek2.GetComponent<TransformComponent>().Translation = {0,0,0.2};

	auto dzwonek3 = m_Scene->CreateEntity("dzwonek3");
	dzwonek3.AddComponent<MeshRendererComponent>(model, mainShader, flatShader);
	dzwonek3.GetComponent<TransformComponent>().Rotate({-90,0,0});
	dzwonek3.GetComponent<TransformComponent>().Translation = {0,0,0.4};

	auto swiatlo = m_Scene->CreateEntity("swiatelko");
	swiatlo.AddComponent<MeshRendererComponent>(light, flatShader, flatShader);
	swiatlo.GetComponent<TransformComponent>().Rotate({-90,0,0});
	swiatlo.GetComponent<TransformComponent>().SetScale({0.1,0.1,0.1});
	swiatlo.GetComponent<TransformComponent>().SetPosition({lightPosition});

	std::shared_ptr<Material> material = std::make_shared<Material>() ;
	material->albedo.id = Texture::TextureFromFile("/home/lolman/git/horror-assets/Models/ServiceBell/Export/ServiceBell_low_ServiceBell_material_BaseColor.png");;
	material->occlusionRoughnessMetallic.id = Texture::TextureFromFile("/home/lolman/git/horror-assets/Models/ServiceBell/Export/ServiceBell_low_ServiceBell_material_OcclusionRoughnessMetallic.png");;
	model->m_Material = material;
	light->m_Material = material;

}

void RVEditor::OnUpdate()
{
	RV_PROFILE_FUNCTION();
	Renderer::BeginScene(camera);
	ProcessInput();

	m_HoveredEntity = frameBuffer->GetEntityID({m_MouseVieportPos.x, m_MouseVieportPos.y});

	if (ClickedInViewPort() && !ImGuizmo::IsUsing())
	{
		if(m_HoveredEntity < 4294967295)
		{

			m_ClickedEntity = m_HoveredEntity;
			m_SceneHierarchyPanel.SetSelectedEntity(Entity((entt::entity)m_ClickedEntity, m_Scene.get()));
			m_Scene->SetSelectedEntity(m_ClickedEntity);

		}
		else
		{
			m_ClickedEntity = entt::null;
			m_SceneHierarchyPanel.SetSelectedEntity(Entity((entt::entity)entt::null, m_Scene.get()));
			m_Scene->SetSelectedEntity(entt::null);
		}
	}



	frameBuffer->Bind();

	Renderer::SetClearColor({0, 0, 0, 1});
	Renderer::Clear();

	m_Scene->OnUpdate(GetDeltaTime());

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
	camera.SetViewMatrix(view);

	mainShader->Bind();
	mainShader->SetVec3("u_CamPos", cameraPos);
//	mainShader->SetVec3("lightPositions[0]", lightPosition);
//	mainShader->SetVec3("lightColors[0]", lightColor * lightIntensity);




	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(lightPosition));
	transform = glm::scale(transform, glm::vec3(0.1,0.1,0.1));

	flatShader->Bind();
	flatShader->SetVec4("u_Color", glm::vec4(lightColor.x,lightColor.y,lightColor.z,1));
	Renderer::Submit(flatShader, light->GetMeshes()->at(0).m_VertexArray, transform);
	frameBuffer->Unbind();
}

void RVEditor::OnImGuiRender()
{
	DrawImGui();
	m_SceneHierarchyPanel.OnRender();
}

void RVEditor::OnShutdown()
{

}

void RVEditor::DrawImGui()
{
	RV_PROFILE_FUNCTION();
	Dockspace();

	ImGui::Begin("Content browser");

	ImGui::End();


	ImGui::Begin("Settings");
	ImGui::Text("FPS: %f", GetFPS());
	mousedelta[0] = m_Input.GetMouseDelta().x;
	mousedelta[1] = m_Input.GetMouseDelta().y;
	ImGui::InputFloat2("Mouse Delta", mousedelta);
	ImGui::InputFloat("Pitch", &pitch);
	ImGui::InputFloat("Yaw", &yaw);

	ImGui::DragFloat3("LightPos", glm::value_ptr(lightPosition), 0.1);
	ImGui::ColorEdit3("LightColor", glm::value_ptr(lightColor));
	ImGui::DragFloat3("LightDir", glm::value_ptr(lightRotation), 0.1);
	ImGui::DragFloat("LightIntensity", &lightIntensity, 0.1);

	ImGui::ColorEdit3("Albedo", glm::value_ptr(albedo));
	ImGui::DragFloat("Metallic", &metallic, 0.1);
	ImGui::DragFloat("Roughness", &roughness, 0.1);
	ImGui::DragFloat("AO", &ao, 0.1);
	ImGui::InputFloat2("Viewport cursor pos", &m_MouseVieportPos[0]);
	ImGui::Text("Hovered entity: %u", m_HoveredEntity);

	ImGui::End();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Viewport");
	m_ViewportFocused = ImGui::IsWindowFocused();

	m_ViewportSize = ImGui::GetContentRegionAvail();

	if (m_ViewportSize.x != m_LastViewportSize.x || m_ViewportSize.y != m_LastViewportSize.y || m_LastWindowSize != GetWindowSize())
	{
		Renderer::SetViewport(0,0,m_ViewportSize.x,m_ViewportSize.y);
		camera.Resize(m_ViewportSize.x, m_ViewportSize.y);
//		frameBuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
		frameBuffer = std::make_shared<FrameBuffer>(m_ViewportSize.x,m_ViewportSize.y);

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

		glm::mat4 cameraView = camera.m_ViewMatrix;
		glm::mat4 cameraProjection = camera.GetProjectionMatrix();

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

	if (m_ViewportFocused)
	{
		float cameraSpeed = 0.5f * GetDeltaTime();



		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
		{
			SetCursorState(GLFW_CURSOR_DISABLED);
			glm::vec2 mouseDelta = m_Input.GetMouseDelta();
			mouseDelta *= 0.12f;
			yaw += mouseDelta.x;
			pitch += mouseDelta.y;

			if(pitch > 89.0f)
				pitch = 89.0f;
			if(pitch < -89.0f)
				pitch = -89.0f;

			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
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
	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspace_id);
	ImGui::End();

}

bool RVEditor::ClickedInViewPort()
{
	if (m_Input.GetMouseDown(0) && m_ViewportFocused && m_MouseVieportPos.x > 0 && m_MouseVieportPos.x < m_ViewportSize.x && m_MouseVieportPos.y > 0 && m_MouseVieportPos.y < m_ViewportSize.y)
		return true;
	else
		return false;
}
