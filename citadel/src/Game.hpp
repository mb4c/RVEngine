#pragma once
#include "Application.hpp"
#include "../../RVEngine/include/Renderer/Model.hpp"
#include "Scene.hpp"
#include <../../RVEngine/include/Renderer/FrameBuffer.hpp>
#include <Entity.hpp>
#include "../../RVEngine/include/Renderer/SceneCamera.hpp"
#include "../../RVEngine/include/Renderer/EditorCamera.hpp"
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

	Entity SpawnBullet();

	void LoadAssets();

	void SpawnEnemyWave(int enemies, float yPos = 5, bool centered = true);
	void CheckPlayerUpgrades();
	Entity m_Player;
	Entity m_Camera;
	float m_MoveSpeed = 0;
	float m_CameraMoveSpeed = 2;
	TimeUntil playerShoot = 0.25;
	glm::vec3 targetPos{};
	// Entity m_BulletPrefab;
	float m_BulletSpeed = 24;

	std::vector<Entity> m_Enemies;
};
