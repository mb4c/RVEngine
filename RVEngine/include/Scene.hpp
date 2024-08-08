#pragma once
#include <entt/entt.hpp>
#include <ResourceManager.hpp>
#include "EditorCamera.hpp"
#include "UUID.hpp"
#include "Components.hpp"
#include "PhysicsManager.hpp"

class Entity;

class Scene
{
public:
	Scene();
	~Scene();

	Entity CreateEntityWithUUID(UUID uuid, const std::string& name);

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap);

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap);

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src);

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src);

	static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> other);

	Entity CreateEntity(const std::string& name = std::string());
	void RemoveEntity(Entity entity);
	Entity DuplicateEntity(Entity entity);
	uint32_t GetEntityCount();

	void OnStart();
	void OnUpdateEditor(float ts, EditorCamera& editorCamera);
	void OnUpdateRuntime(float ts);
	void RenderScene();

	void OnRuntimeStart();
	void OnRuntimeStop();

	void SetSelectedEntity(uint32_t entity);
	void SetViewportSize(uint32_t width, uint32_t height);

	std::string GetName() {  return m_SceneName;};
	void SetName(const std::string& name) {m_SceneName = name;};

	void SetPhysicsPosition(Entity entity, glm::vec3 pos);
	glm::vec3 GetPhysicsPosition(Entity entity);
	void SetGravityFactor(Entity entity, float gravityFactor);
	void SetVelocity(Entity entity, glm::vec3 velocity);
	void AddVelocity(Entity entity, glm::vec3 velocity);
	void SetAngularVelocity(Entity entity, glm::vec3 velocity);

	glm::vec3 GetVelocity(Entity entity);
	glm::vec3 GetAngularVelocity(Entity entity);

private:
	entt::registry m_Registry;
	uint32_t m_SelectedEntity;
	bool m_IsRunning = false;
	std::unordered_map<UUID, entt::entity> m_EntityMap;
	PhysicsManager* m_PhysicsManager = nullptr;
	uint32_t m_ViewportWidth, m_ViewportHeight;
	std::string m_SceneName = "Untitled";

	friend class Entity;
	friend class SceneHierarchyPanel;
	friend class SceneSerializer;
};


