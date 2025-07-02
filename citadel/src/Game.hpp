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
#include "Time.hpp"
#include "Bullet.hpp"
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
	std::shared_ptr<FrameBuffer> frameBuffer;
	FrameBufferProperties m_FramebufferProps;
	std::filesystem::path m_SavedScenePath;
	glm::vec2 m_LastViewportSize{};
	glm::vec3 m_CursorWorldPos{};


	void LoadAssets();

	Entity m_Player;
	Entity m_Camera;
	float m_MoveSpeed = 24;
	float m_CameraMoveSpeed = 2;
	TimeUntil playerShoot = 0.25;
	glm::vec3 targetPos{};

	Entity m_BulletPrefab;
	std::vector<Bullet> m_Bullets;
	float m_BulletSpeed = 24;
};
