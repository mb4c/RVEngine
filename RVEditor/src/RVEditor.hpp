#pragma once

#include "Application.hpp"
#include "Model.hpp"
#include "Scene.hpp"
#include <FrameBuffer.hpp>
#include <Entity.hpp>
#include "SceneHierarchyPanel.hpp"
#include "SceneCamera.hpp"
#include "EditorCamera.hpp"
#include "SceneSerializer.hpp"
#include <ImGuizmo.h>
#include <Math.hpp>
#include "ContentBrowserPanel.hpp"

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

	bool ClickedInViewPort();

	void NewScene();
	std::filesystem::path SaveSceneAs();

	bool m_CursorEnabled = true;
	bool m_ViewportFocused = false;

	SceneHierarchyPanel m_SceneHierarchyPanel;
	ContentBrowserPanel m_ContentBrowserPanel;
	std::shared_ptr<Scene> m_ActiveScene;


	std::shared_ptr<Model> model;
	std::shared_ptr<Model> cerberus;
	std::shared_ptr<Model> light;

	EditorCamera m_Camera;

	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  1.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	glm::vec3 direction;

	float pitch = 0;
	float yaw = -90;

	glm::vec3 lightPosition = glm::vec3(0.5f, 0.0f,  0.0f);
	glm::vec3 lightRotation = glm::vec3(2.0f, 0.0f,  3.0f);
	glm::vec3 lightColor  	= glm::vec3(1.0f, 1.0f,  1.0f);
	float lightIntensity = 1.0f;


	glm::vec3 albedo = glm::vec3(1.0f, 1.0f,  1.0f);
	float metallic = 1.0f;
	float roughness = 1.0f;
	float ao = 1.0f;


	uint32_t m_MeshesCount = 0;
	float mousedelta[2];

	ImVec2 m_ViewportSize {};
	ImVec2 m_LastViewportSize {};
	glm::ivec2 m_LastWindowSize {};
	ImVec2 m_MouseVieportPos {0,0};
	uint32_t m_HoveredEntity {};
	uint32_t m_ClickedEntity {};

	std::shared_ptr<Shader> flatShader;
	std::shared_ptr<Shader> mainShader;

	std::shared_ptr<FrameBuffer> frameBuffer;

	int m_GizmoType = -1;
	SceneSerializer m_Serializer;

	unsigned int m_DisplayType = 0;
};
