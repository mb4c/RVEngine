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
	void DuplicateEntity(Entity entity);

	void OnStart();
	void OnUpdateEditor(float ts, EditorCamera& editorCamera);
	void OnUpdateRuntime(float ts);
	void RenderScene();

	void OnRuntimeStart();
	void OnRuntimeStop();

	void SetSelectedEntity(uint32_t entity);
private:
	entt::registry m_Registry;
	uint32_t m_SelectedEntity;
	bool m_IsRunning = false;
	std::unordered_map<UUID, entt::entity> m_EntityMap;
	PhysicsManager* m_PhysicsManager = nullptr;


	friend class Entity;
	friend class SceneHierarchyPanel;
	friend class SceneSerializer;
};


