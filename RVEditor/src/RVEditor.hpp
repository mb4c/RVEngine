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
#include "AssetsPanel.hpp"
#include "NewProjectModal.hpp"
#include "ProjectSettings.hpp"
#include "AssetImporter.hpp"
#include <Widgets.hpp>

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

	bool ClickedInViewport();
	bool LeftClickedInViewport();
	bool RightClickedInViewport();
	bool CursorInViewport() const;

	void NewScene();
	std::filesystem::path SaveSceneAs();
	std::filesystem::path SaveScene();
	void OpenScene(const std::filesystem::path& path);
	void OpenProject(const std::filesystem::path& path);

	void OnScenePlay();
	void OnSceneStop();

	bool IsAnyPopupOpen();

	void UpdateWindowTitle();

	bool m_ViewportFocused = false;

	SceneHierarchyPanel m_SceneHierarchyPanel;
	AssetsPanel m_AssetsPanel;
	std::shared_ptr<Scene> m_ActiveScene;
	std::shared_ptr<Scene> m_EditorScene, m_RuntimeScene;


	std::shared_ptr<Model> model;
	std::shared_ptr<Model> light;

	EditorCamera m_Camera;

	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  1.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	glm::vec3 direction;

	float m_CameraPitch = 0;
	float m_CameraYaw = -90;
	float m_CameraSpeed = 2;

	ImVec2 m_ViewportSize {};
	ImVec2 m_LastViewportSize {};
	glm::ivec2 m_LastWindowSize {};
	ImVec2 m_MouseVieportPos {0,0};
	uint32_t m_HoveredEntity {};
	uint32_t m_ClickedEntity {};

	std::shared_ptr<Shader> flatShader;
	std::shared_ptr<Shader> mainShader;

	std::shared_ptr<FrameBuffer> frameBuffer;

	int m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	int32_t m_SelectedGizmo = 0;

	unsigned int m_DisplayType = 0;

	enum class SceneState
	{
		Edit = 0, Play = 1, Simulate = 2
	};
	SceneState m_SceneState = SceneState::Edit;
	std::filesystem::path m_SavedScenePath;

	NewProjectModal m_NewProjectModal;
	ProjectSettings m_ProjectSettings;

	AssetImporter m_AssetImporterModal;
};
