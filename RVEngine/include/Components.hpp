#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>
#include "Model.hpp"

struct TagComponent
{
	std::string Tag;

	TagComponent() = default;
	TagComponent(const TagComponent&) = default;
	TagComponent(const std::string& tag)
			: Tag(tag) {}
};

struct TransformComponent
{
	glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3 & translation)
			: Translation(translation) {}

	glm::mat4 GetTransform() const
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, {1,0,0})
						   * glm::rotate(glm::mat4(1.0f), Rotation.y, {0,1,0})
						   * glm::rotate(glm::mat4(1.0f), Rotation.z, {0,0,1});

		return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
	}

	// Rotate in degrees
	void Rotate(glm::vec3 rotation)
	{
		Rotation += glm::radians(rotation);
	}
	void SetScale(glm::vec3 scale)
	{
		Scale = scale;
	}
	void SetPosition(glm::vec3 position)
	{
		Translation = position;
	}


};

struct MeshRendererComponent
{
	std::shared_ptr<Model> model;
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Shader> outlineShader;

	MeshRendererComponent() = default;
	MeshRendererComponent(const MeshRendererComponent&) = default;
	MeshRendererComponent(std::shared_ptr<Model> mdl, std::shared_ptr<Shader> shdr, std::shared_ptr<Shader> outShdr)
			: model(std::move(mdl)), shader(std::move(shdr)), outlineShader(std::move(outShdr)) {}
};

//template<typename Component>
//static void Display(Component& component)
//{
//
//	// Could we do it the everything'sfinae way instead?
//
//	static_assert(false && "Missing Display() implementation for component");
//
//};
//
//template<>
//static void Display(TagComponent& component)
//{
//
//	ImGui::Text("TestStruct print: %s", component.Tag.c_str());
//
//}
