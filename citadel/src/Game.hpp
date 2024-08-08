#pragma once
#include "Application.hpp"
#include "Model.hpp"
#include "Scene.hpp"
#include <FrameBuffer.hpp>
#include <Entity.hpp>
#include "SceneCamera.hpp"
#include "EditorCamera.hpp"
#include "SceneSerializer.hpp"
#include <ImGuizmo.h>
#include <Math.hpp>
#include "ProjectSettings.hpp"
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
	void OpenScene(const std::filesystem::path& path);
	void OpenProject(const std::filesystem::path& path);
	std::shared_ptr<Scene> m_ActiveScene;
	std::shared_ptr<FrameBuffer> frameBuffer;
	std::filesystem::path m_SavedScenePath;
};
