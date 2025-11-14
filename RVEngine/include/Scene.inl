#pragma once

#include "Entity.hpp"

template<typename... Component>
std::vector<Entity> Scene::GetEntitiesWithComponent()
{
    std::vector<Entity> entities;

    auto view = m_Registry.view<Component...>();

    for (entt::entity entity : view)
    {
        entities.emplace_back(entity, this);
    }

    return entities;
}