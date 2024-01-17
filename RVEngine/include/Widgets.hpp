#pragma once

#include <memory>
#include <Texture2D.hpp>

namespace ImGui
{

	inline std::string TexturePickerPopup()
	{
		if (ImGui::BeginPopup("texture_picker_popup"))
		{
			ImGui::SeparatorText("Texture picker");

			ResourceManager& rm = ResourceManager::instance();

			for (auto& it: rm.m_Textures)
			{
				ImGui::Image((void*) (intptr_t) it.second->GetTexture(), ImVec2(16, 16));
				ImGui::SameLine();
				if (ImGui::Selectable(it.first.c_str()))
				{
					ImGui::EndPopup();
					return it.first;
				}
			}

			ImGui::EndPopup();
		}
		return {};
	}

	inline void AddComponentPopup(Entity& entity)
	{
		ResourceManager& rm = ResourceManager::instance();

		if (ImGui::BeginPopup("add_component_popup"))
		{
			ImGui::SeparatorText("Add component");

			if (ImGui::Selectable("Sprite renderer"))
				entity.AddComponent<SpriteRendererComponent>();
			if (ImGui::Selectable("Camera"))
				entity.AddComponent<CameraComponent>();
			if (ImGui::Selectable("Mesh renderer"))
				entity.AddComponent<MeshRendererComponent>(rm.GetModel("cube"), rm.GetShader("pbr"), rm.GetShader("flat"));
			if (ImGui::Selectable("Light"))
				entity.AddComponent<LightComponent>();
			if (ImGui::Selectable("Box collider"))
				entity.AddComponent<BoxColliderComponent>();
			if (ImGui::Selectable("Sphere collider"))
				entity.AddComponent<SphereColliderComponent>();

			ImGui::EndPopup();
		}
	}

	inline void TextureEditColor(const char* label, std::shared_ptr<Texture2D>& src, float color[4])
	{
		if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_None))
		{
			if (ImGui::ImageButton((void*) (intptr_t) src->GetTexture(), ImVec2(48, 48)))
			{
				ImGui::OpenPopup("texture_picker_popup");
			}

			ResourceManager& rm = ResourceManager::instance();
			std::string newTex = ImGui::TexturePickerPopup();

			if (!newTex.empty())
			{
				std::shared_ptr<Texture2D> tex = rm.GetTexture(newTex);
				src = tex;
			}

			ImGui::SameLine();
			ImGui::Text("%s", label);
			ImGui::SameLine();
			ImGui::ColorEdit4("Color", color, ImGuiColorEditFlags_NoInputs);
		}
	}

	inline void TextureEdit(const char* label, std::shared_ptr<Texture2D>& src)
	{
		if (ImGui::ImageButton((void*) (intptr_t) src->GetTexture(), ImVec2(48, 48)))
		{
			ImGui::OpenPopup("texture_picker_popup");
		}

		ResourceManager& rm = ResourceManager::instance();
		std::string newTex = ImGui::TexturePickerPopup();

		if (!newTex.empty())
		{
			std::shared_ptr<Texture2D> tex = rm.GetTexture(newTex);
			src = tex;
		}

		ImGui::SameLine();
		ImGui::Text("%s", label);

	}

	template <typename Component>
	inline bool ComponentTreeNode(Entity entity)
	{
		if (entity.HasComponent<Component>())
		{

			std::size_t nodeId = (std::size_t)entity.GetHandle() ^ typeid(Component).hash_code(); // xor to get unique node id

			bool treeOpen = ImGui::TreeNodeEx((void*)nodeId,
											  ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap,
											  GetComponentName<Component>().c_str());
			ImGui::SameLine();
			if (ImGui::Button("x"))
			{
				entity.RemoveComponent<Component>();
				ImGui::TreePop();
			}
			if (treeOpen && entity.HasComponent<Component>())
			{
				return true;
			}
			return false;
		}
		return false;
	}

}