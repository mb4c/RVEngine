#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "SceneHierarchyPanel.hpp"
#include "Components.hpp"

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &context)
{
	RV_PROFILE_FUNCTION();
	SetContext(context);
}

void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene> &context)
{
	RV_PROFILE_FUNCTION();
	m_Context = context;
}

void SceneHierarchyPanel::OnRender()
{
	RV_PROFILE_FUNCTION();
	ImGui::Begin("Scene hierarchy");
	m_Context->m_Registry.each([this](auto entityID)
	{
		Entity entity {entityID, m_Context.get()};
		DrawEntityNode(entity);
	});
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("New Light"))
		{
			auto light = m_Context->CreateEntity("Point light");
			light.AddComponent<LightComponent>();
			m_SelectionContext = light;
			m_Context->SetSelectedEntity(light);
		}
		if (GetSelectedEntity())
		{
			if (ImGui::MenuItem("Remove entity"))
			{
				m_Context->RemoveEntity(GetSelectedEntity());
				m_SelectionContext = Entity(entt::null, m_Context.get());
			}
		}
		if (GetSelectedEntity())
		{
			if (ImGui::MenuItem("Duplicate entity"))
			{
				m_Context->DuplicateEntity(GetSelectedEntity());
			}
		}

		ImGui::EndPopup();
	}

	ImGui::End();

	ImGui::Begin("Inspector");

//	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
//		m_SelectionContext = {};

	if (m_SelectionContext)
		DrawComponents(m_SelectionContext);
	ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity)
{
	RV_PROFILE_FUNCTION();
	auto& tag = entity.GetComponent<TagComponent>().Tag;

	ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, "%s", tag.c_str());
	if (ImGui::IsItemClicked() || ImGui::IsItemClicked(1))
	{
		m_SelectionContext = entity;
		m_Context->SetSelectedEntity(entity);
	}
	if (opened)
	{
		ImGui::TreePop();
	}
}

void SceneHierarchyPanel::DrawComponents(Entity entity)
{
	RV_PROFILE_FUNCTION();
	if (entity.HasComponent<TagComponent>())
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGui::InputText("Tag", &tag);
	}

	if (entity.HasComponent<TransformComponent>())
	{
		if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			ImGui::DragFloat3("Position", glm::value_ptr(transform.Translation),0.01f);
			auto rot = glm::degrees(transform.Rotation);
			ImGui::DragFloat3("Rotation", glm::value_ptr(rot),0.1f);
			transform.Rotation = glm::radians(rot);
			ImGui::DragFloat3("Scale", glm::value_ptr(transform.Scale),0.1f);
			ImGui::TreePop();
		}
	}
	if (entity.HasComponent<LightComponent>())
	{
		if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Light"))
		{
			auto& lc = entity.GetComponent<LightComponent>();
			ImGui::ColorEdit3("Color", glm::value_ptr(lc.color));
			ImGui::DragFloat("Intensity", &lc.intensity, 0.01f);

			ImGui::TreePop();
		}
	}

}

void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
{
	m_SelectionContext = entity;
}

Entity SceneHierarchyPanel::GetSelectedEntity()
{
	return m_SelectionContext;
}


