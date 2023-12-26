#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "SceneHierarchyPanel.hpp"
#include "Components.hpp"
#include <Widgets.hpp>

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &context)
{
	RV_PROFILE_FUNCTION();
	SetContext(context);
}

void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene> &context)
{
	RV_PROFILE_FUNCTION();
	m_Context = context;
	m_SelectionContext = {};
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
		if (ImGui::BeginMenu("Create..."))
		{
			if (ImGui::MenuItem("Create empty entity"))
			{
				m_Context->CreateEntity("Empty entity");
			}
			if (ImGui::MenuItem("Create Plane"))
			{
				ResourceManager& rm = ResourceManager::instance();
				auto model = rm.GetModel("plane");
				auto shader = rm.GetShader("pbr");
				auto flatShader = rm.GetShader("flat");

				model->m_Material = rm.GetMaterial("default_pbr");
				auto plane = m_Context->CreateEntity("Plane");
				plane.AddComponent<MeshRendererComponent>(model, shader, flatShader);
				plane.GetComponent<TransformComponent>().SetScale({1, 1, 1});
				plane.GetComponent<TransformComponent>().SetPosition({0, 0, 0});
			}
			if (ImGui::MenuItem("Create Cube"))
			{
				ResourceManager& rm = ResourceManager::instance();
				auto model = rm.GetModel("cube");
				auto shader = rm.GetShader("pbr");
				auto flatShader = rm.GetShader("flat");

				model->m_Material = rm.GetMaterial("default_pbr");
				auto plane = m_Context->CreateEntity("Cube");
				plane.AddComponent<MeshRendererComponent>(model, shader, flatShader);
				plane.GetComponent<TransformComponent>().SetScale({1, 1, 1});
				plane.GetComponent<TransformComponent>().SetPosition({0, 0, 0});
			}

			if (ImGui::MenuItem("Create Sphere"))
			{
				ResourceManager& rm = ResourceManager::instance();
				auto model = rm.GetModel("sphere05");
				auto shader = rm.GetShader("pbr");
				auto flatShader = rm.GetShader("flat");

				model->m_Material = rm.GetMaterial("default_pbr");
				auto plane = m_Context->CreateEntity("Sphere");
				plane.AddComponent<MeshRendererComponent>(model, shader, flatShader);
				plane.GetComponent<TransformComponent>().SetScale({1, 1, 1});
				plane.GetComponent<TransformComponent>().SetPosition({0, 0, 0});
			}

			if (ImGui::MenuItem("Create Point light"))
			{
				auto light = m_Context->CreateEntity("Point light");
				light.AddComponent<LightComponent>();
				m_SelectionContext = light;
				m_Context->SetSelectedEntity(light);
			}

			ImGui::EndMenu();
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
	{
		DrawComponents(m_SelectionContext);
		if(ImGui::Button("Add component"))
		{
			ImGui::OpenPopup("add_component_popup");
		}
		ImGui::AddComponentPopup(m_SelectionContext);
	}



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

	if (entity.HasComponent<IDComponent>())
	{
		auto& id = entity.GetComponent<IDComponent>().ID;

		ImGui::Text("ID: %lu", (uint64_t) id);
	}

	if (entity.HasComponent<TagComponent>())
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGui::InputText("Tag", &tag);
	}

	if (entity.HasComponent<TransformComponent>())
	{
		if (ImGui::TreeNodeEx((void*) typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen,
							  "Transform"))
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			ImGui::DragFloat3("Position", glm::value_ptr(transform.Translation), 0.01f);
			auto rot = glm::degrees(transform.Rotation);
			ImGui::DragFloat3("Rotation", glm::value_ptr(rot), 0.1f);
			transform.Rotation = glm::radians(rot);
			ImGui::DragFloat3("Scale", glm::value_ptr(transform.Scale), 0.1f);
			ImGui::TreePop();
		}
	}

	if (ImGui::ComponentTreeNode<LightComponent>(entity))
	{
		auto& lc = entity.GetComponent<LightComponent>();
		ImGui::ColorEdit3("Color", glm::value_ptr(lc.color));
		ImGui::DragFloat("Intensity", &lc.intensity, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::ComponentTreeNode<MeshRendererComponent>(entity))
	{
		auto& mrc = entity.GetComponent<MeshRendererComponent>();
		ImGui::TreePop();
	}

	if (ImGui::ComponentTreeNode<SpriteRendererComponent>(entity))
	{
		auto& src = entity.GetComponent<SpriteRendererComponent>();
		ImGui::TextureEditColor("Albedo", src.Tex, glm::value_ptr(src.Color));

		ImGui::Checkbox("Billboard", &src.Billboard);

		ImGui::TreePop();
	}

	if (ImGui::ComponentTreeNode<CameraComponent>(entity))
	{
		auto& cc = entity.GetComponent<CameraComponent>();
		auto& tc = entity.GetComponent<TransformComponent>();
		ImGui::Checkbox("Primary", &cc.Primary);
		ImGui::Checkbox("Fixed aspect ratio", &cc.FixedAspectRatio);

		auto nc = cc.Camera.GetPerspectiveNearClip();
		if (ImGui::InputFloat("Near clip", &nc))
			cc.Camera.SetPerspectiveNearClip(nc);

		auto fc = cc.Camera.GetPerspectiveFarClip();
		if (ImGui::InputFloat("Far clip", &fc))
			cc.Camera.SetPerspectiveFarClip(fc);

		auto fov = cc.Camera.GetPerspectiveVerticalFOV();
		fov *= 100;
		if (ImGui::DragFloat("Vertical FOV", &fov, 0.05f, 0.01, 360))
		{
			fov /= 100;
			cc.Camera.SetPerspectiveVerticalFOV(fov);
		}

		if(ImGui::Button("TODO: Copy editor camera transform"))
		{
			//TODO: Copy editor camera transform
		}

		ImGui::TreePop();
	}

	if (ImGui::ComponentTreeNode<BoxColliderComponent>(entity))
	{
		auto& bcc = entity.GetComponent<BoxColliderComponent>();
		ImGui::DragFloat3("Scale##2", glm::value_ptr(bcc.Size), 0.01f);
		ImGui::Checkbox("Dynamic", &bcc.Dynamic);
		ImGui::DragFloat("Mass", &bcc.Mass);

		ImGui::TreePop();
	}

	if (ImGui::ComponentTreeNode<SphereColliderComponent>(entity))
	{
		auto& scc = entity.GetComponent<SphereColliderComponent>();
		ImGui::DragFloat("Radius##2", &scc.Radius, 0.01f);
		ImGui::Checkbox("Dynamic", &scc.Dynamic);
		ImGui::DragFloat("Mass", &scc.Mass);

		ImGui::TreePop();
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
