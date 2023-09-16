#pragma once

#include "Application.hpp"
#include "Model.hpp"
#include "PerspectiveCamera.hpp"
#include "Scene.hpp"
#include <FrameBuffer.hpp>
#include <Entity.hpp>
#include <Math.hpp>
#include "SceneHierarchyPanel.hpp"

class RVEditor : public Application
{
public:
	RVEditor(const std::string &title, int width, int height);
private:
	void OnInit() override;
	void OnUpdate() override;
	void OnShutdown() override;
	void OnImGuiRender() override;
	void DrawImGui();
	void ProcessInput();
	void Dockspace();


	bool m_CursorEnabled = true;
	bool m_ViewportFocused = false;

	SceneHierarchyPanel m_SceneHierarchyPanel;
	std::shared_ptr<Scene> m_Scene;
	Entity m_BackpackEntity;


	std::shared_ptr<Model> model;
	std::shared_ptr<Model> light;

//	Model light {"/home/lolman/monke.fbx"};
	PerspectiveCamera camera;


	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	glm::vec3 direction;

	float pitch = 0;
	float yaw = -90;

	glm::vec3 lightPosition = glm::vec3(2.0f, 0.0f,  3.0f);
	glm::vec3 lightRotation = glm::vec3(2.0f, 0.0f,  3.0f);
	glm::vec3 lightColor  	= glm::vec3(1.0f, 1.0f,  1.0f);
	uint32_t m_MeshesCount = 0;
	float mousedelta[2];

	ImVec2 m_ViewportSize {};
	ImVec2 m_LastViewportSize {};
	glm::ivec2 m_LastWindowSize {};

	std::shared_ptr<Shader> flatShader;
	std::shared_ptr<Shader> mainShader;

	std::shared_ptr<FrameBuffer> frameBuffer;
};
