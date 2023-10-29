#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <Scene.hpp>

class Entity
{
public:
	Entity() = default;
	Entity(entt::entity handle, Scene* scene);
	Entity(const Entity& other) = default;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		assert(!HasComponent<T>()); // "Entity already has component!"
		return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent()
	{
		assert(HasComponent<T>()); // "Entity does not have component!"
		return m_Scene->m_Registry.get<T>(m_EntityHandle);
	}

	template<typename T>
	bool HasComponent()
	{
		return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		assert(HasComponent<T>());
		m_Scene->m_Registry.remove<T>(m_EntityHandle); // "Entity does not have component!"
	}

	operator bool() const { return m_EntityHandle != entt::null; }
	operator bool() { return m_EntityHandle != entt::null; }
	operator uint32_t () { return (uint32_t)m_EntityHandle;}
	bool operator==(const Entity& other) const
	{
		return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
	}

	bool operator!=(const Entity& other) const
	{
		return !(*this == other);
	}

	entt::entity GetHandle() { return m_EntityHandle;};

private:
	entt::entity m_EntityHandle{ entt::null };
	Scene* m_Scene = nullptr;
};