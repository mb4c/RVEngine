#pragma once
#include <memory>
#include <imgui_stdlib.h>
#include <ResourceManager.hpp>
class Scene;
#include "Entity.hpp"

class SceneHierarchyPanel
{
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(const std::shared_ptr<Scene>& context);
	void SetContext(const std::shared_ptr<Scene>& context);
	void OnRender();
	void SetSelectedEntity(Entity entity);
	Entity GetSelectedEntity();
private:
	void DrawEntityNode(Entity entity);
	void DrawComponents(Entity entity);
	std::shared_ptr<Scene> m_Context;
	Entity m_SelectionContext;
};
