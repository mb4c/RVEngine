#pragma once
#include "Application.hpp"
#include "Renderer/Model.hpp"
#include "Scene.hpp"
#include <Renderer/FrameBuffer.hpp>
#include <Entity.hpp>
#include "SceneSerializer.hpp"
#include <ImGuizmo.h>
#include <Math.hpp>
#include "ProjectSettings.hpp"
#include "Time.hpp"
#include <Widgets.hpp>

class Game : public Application
{
public:
	Game(const std::string &title, int width, int height);
private:
	void OnInit() override;
	void OnUpdate() override;
	void OnShutdown() override;
	void OnImGuiRender() override;
	void ProcessInput();
	void OnResize();
	void OpenScene(const std::filesystem::path& path);
	void OpenProject(const std::filesystem::path& path);
	std::shared_ptr<Scene> m_ActiveScene;
	std::shared_ptr<FrameBuffer> m_FrameBuffer;
	FrameBufferProperties m_FramebufferProps;
	std::filesystem::path m_SavedScenePath;
	glm::vec2 m_LastViewportSize{};
	glm::vec3 m_CursorWorldPos{};

	void LoadAssets();

	Entity m_Camera;
};
