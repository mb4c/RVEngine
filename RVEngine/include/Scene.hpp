#pragma once
#include <entt/entt.hpp>
#include <ResourceManager.hpp>

class Entity;

class Scene
{
public:
	Scene();
	~Scene();

	Entity CreateEntity(const std::string& name = std::string());
	void RemoveEntity(Entity entity);
	void DuplicateEntity(Entity entity);

	void OnUpdate(float ts);
	void SetSelectedEntity(uint32_t entity);
private:
	entt::registry m_Registry;
	uint32_t m_SelectedEntity;

	friend class Entity;
	friend class SceneHierarchyPanel;
	friend class SceneSerializer;
};


