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
	glm::mat4 Transform{ 1.0f };

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::mat4 & transform)
			: Transform(transform) {}

	operator glm::mat4& () { return Transform; }
	operator const glm::mat4& () const { return Transform; }

	void Translate(glm::vec3 pos)
	{
		glm::translate(Transform, pos);
	}

};

struct MeshRendererComponent
{
	std::shared_ptr<Model> model;
	std::shared_ptr<Shader> shader;

	MeshRendererComponent() = default;
	MeshRendererComponent(const MeshRendererComponent&) = default;
	MeshRendererComponent(std::shared_ptr<Model> mdl, std::shared_ptr<Shader> shdr)
			: model(std::move(mdl)), shader(std::move(shdr)) {}
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
