#include "RVEditor.hpp"
#include "Renderer.hpp"
#include "Macros.hpp"
#include "Components.hpp"

RVEditor::RVEditor(const std::string &title, int width, int height) : Application(title, width, height)
{

}

void RVEditor::OnInit()
{
	model = std::make_shared<Model>("/home/lolman/Downloads/backpack/backpack.obj");
	light = std::make_shared<Model>("/home/lolman/monke.fbx");

	flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
	mainShader = std::make_shared<Shader>("res/shaders/vert.glsl", "res/shaders/frag.glsl");

	frameBuffer = std::make_shared<FrameBuffer>(GetWindowSize().x,GetWindowSize().y);
	m_Scene = std::make_shared<Scene>();

	m_SceneHierarchyPanel.SetContext(m_Scene);
	m_BackpackEntity = m_Scene->CreateEntity("Dupa romana");
	m_BackpackEntity.AddComponent<MeshRendererComponent>(model, mainShader);

}

void RVEditor::OnUpdate()
{
	RV_PROFILE_FUNCTION();
	Renderer::BeginScene(camera);
	ProcessInput();

	frameBuffer->Bind();
	Renderer::Clear();

	m_Scene->OnUpdate(GetDeltaTime());

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
	camera.SetViewMatrix(view);
	mainShader->Bind();

	mainShader->SetVec3("u_CamPos", cameraPos);
//		mainShader->SetVec3("u_Light.position", lightPosition);
	mainShader->SetVec3("u_Light.direction", lightRotation);
	mainShader->SetVec3("u_Light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	mainShader->SetVec3("u_Light.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	mainShader->SetVec3("u_Light.specular", glm::vec3(0.5f, 0.5f, 0.5f));

	mainShader->SetFloat("u_Material.shininess", 32.0f);

//	auto meshes = model->GetMeshes();
//	m_MeshesCount = meshes->size();
//	{
//		RV_PROFILE_SCOPE("MeshLoop");
//		for (int i = 0; i < m_MeshesCount; ++i)
//		{
////			 bind diffuse map
//			glActiveTexture(GL_TEXTURE0);
//			glBindTexture(GL_TEXTURE_2D, meshes->at(i).m_Textures.at(0).id);
//			// bind specular map
//			glActiveTexture(GL_TEXTURE1);
//			glBindTexture(GL_TEXTURE_2D, meshes->at(i).m_Textures.at(1).id);
//			Renderer::Submit(mainShader, meshes->at(i).m_VertexArray);
//		}
//	}
//
//	glm::mat4 transform = glm::mat4(1.0f);
//	transform = glm::translate(transform, glm::vec3(lightPosition));
//	flatShader->Bind();
//	flatShader->SetVec4("u_Color", glm::vec4(lightColor.x,lightColor.y,lightColor.z,1));
//	Renderer::Submit(flatShader, light->GetMeshes()->at(0).m_VertexArray, transform);

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

	if (m_BackpackEntity)
	{
		ImGui::Separator();
		auto& tag = m_BackpackEntity.GetComponent<TagComponent>().Tag;
		ImGui::Text("%s", tag.c_str());

		ImGui::Separator();
	}
	ImGui::End();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // Big padding used for clarity.	// Get the size of the child (i.e. the whole draw size of the windows).

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
		float cameraSpeed = 2.0f * GetDeltaTime();



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