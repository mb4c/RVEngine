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
		if (ImGui::BeginMenu("Create 3D..."))
		{
			if (ImGui::MenuItem("Create Plane"))
			{
				auto model = std::make_shared<Model>("res/plane.fbx");
				auto shader = std::make_shared<Shader>("res/shaders/PBR_vert.glsl", "res/shaders/PBR_frag.glsl");
				auto flatShader = std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag");
				auto material = std::make_shared<Material>();
				material->albedo = Texture("res/brickwall.jpg");
				material->normal= Texture("res/brickwall_normal.jpg");
				material->occlusionRoughnessMetallic = Texture("res/brickwall_ORM.png");
				model->m_Material = material;
				auto plane = m_Context->CreateEntity("Plane");
				plane.AddComponent<MeshRendererComponent>(model,shader, flatShader);
				plane.GetComponent<TransformComponent>().SetScale({0.1,0.1,0.1});
				plane.GetComponent<TransformComponent>().SetPosition({0,0,-0.035});
			}
			ImGui::EndMenu();
		}

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
	if (entity.HasComponent<MeshRendererComponent>())
	{
		if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "MeshRenderer"))
		{
			auto& mrc = entity.GetComponent<MeshRendererComponent>();


			ImGui::TreePop();
		}
	}

	if (entity.HasComponent<CameraComponent>())
	{
		if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
		{
			auto& cc = entity.GetComponent<CameraComponent>();
			ImGui::Checkbox("Primary", &cc.Primary);
			ImGui::Checkbox("Fixed aspect ratio", &cc.FixedAspectRatio);

			auto nc = cc.Camera.GetPerspectiveNearClip();
			if(ImGui::InputFloat("Near clip", &nc))
				cc.Camera.SetPerspectiveNearClip(nc);

			auto fc = cc.Camera.GetPerspectiveFarClip();
			if(ImGui::InputFloat("Far clip", &fc))
				cc.Camera.SetPerspectiveFarClip(fc);
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


