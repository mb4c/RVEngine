#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>
#include <Model.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "SceneCamera.hpp"

struct TagComponent
{
	std::string Tag;

	TagComponent() = default;
	TagComponent(const TagComponent&) = default;
	TagComponent(const std::string& tag)
			: Tag(tag) {}
};

struct IDComponent
{
	UUID ID;

	IDComponent() = default;
	IDComponent(const IDComponent&) = default;
};

struct TransformComponent
{
	glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f }; // Radians
	glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };
	glm::mat4 GlobalTransform = glm::mat4(1);

	bool IsDirty = true;

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3 & translation)
			: Translation(translation) {}

	glm::mat4 GetLocalTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

		return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
	}
	glm::mat4 GetTransform() const
	{
		return GlobalTransform;
	}

	void Translate(glm::vec3 translation)
	{
		Translation += translation;
		IsDirty = true;
	}

	// Rotate in degrees
	void Rotate(glm::vec3 rotation)
	{
		Rotation += glm::radians(rotation);
		IsDirty = true;
	}
	void SetScale(glm::vec3 scale)
	{
		Scale = scale;
		IsDirty = true;
	}
	void SetPosition(glm::vec3 position)
	{
		Translation = position;
		IsDirty = true;
	}
	glm::vec3 GetPosition()
	{
		return Translation;
	}
	void SetRotation(glm::vec3 rotation)
	{
		Rotation = glm::radians(rotation);
		IsDirty = true;
	}
	void SetRotationRad(glm::vec3 rotation)
	{
		Rotation = rotation;
		IsDirty = true;
	}
	glm::vec3 GetRotationRad()
	{
		return Rotation;
	}
	glm::quat GetRotationQuat()
	{
		return glm::quat(Translation);
	}
};

struct MeshRendererComponent
{
	std::shared_ptr<Model> model;
	std::shared_ptr<Shader> shader;

	MeshRendererComponent() = default;
	MeshRendererComponent(const MeshRendererComponent&) = default;
	MeshRendererComponent(std::shared_ptr<Model> mdl, std::shared_ptr<Shader> shdr, std::shared_ptr<Shader> outShdr)
			: model(std::move(mdl)), shader(std::move(shdr)) {}
};

struct SpriteRendererComponent
{
	glm::vec4 Color {1,1,1,1};
	bool Billboard = false;
	std::shared_ptr<Texture2D> Tex = ResourceManager::instance().GetTexture("default_albedo");

	SpriteRendererComponent() = default;
	SpriteRendererComponent(const SpriteRendererComponent&) = default;
	SpriteRendererComponent(glm::vec4 color) : Color(color){}
	SpriteRendererComponent(std::shared_ptr<Texture2D> texture) : Tex(texture) {}
};

struct LightComponent
{
	glm::vec3 color = {1, 1, 1};
	float intensity = {1.0};

	LightComponent() = default;
	LightComponent(const LightComponent&) = default;
	LightComponent(const glm::vec3 & newcolor, const float _intensity)
			: color(newcolor), intensity(_intensity) {}
};

struct CameraComponent
{
	SceneCamera Camera;
	bool Primary = true;
	bool FixedAspectRatio = false;

	CameraComponent() = default;
	CameraComponent(const CameraComponent&) = default;
};

struct BoxColliderComponent
{

	glm::vec3 Size = {1, 1, 1};
	bool Dynamic = true;
	float Mass = 10;
	float Restitution = 0.5f;
	float Friction = 0.2f;
	uint32_t IndexSequence;

	BoxColliderComponent() = default;
	BoxColliderComponent(const BoxColliderComponent&) = default;
};

struct SphereColliderComponent
{
	float Radius = 0.5;
	bool Dynamic = true;
	float Mass = 10;
	float Restitution = 0.5f;
	float Friction = 0.2f;
	uint32_t IndexSequence;

	SphereColliderComponent() = default;
	SphereColliderComponent(const SphereColliderComponent&) = default;
};

//TODO: serialization
struct SkyboxComponent
{
	unsigned int envCubemap;
	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUTTexture;

	SkyboxComponent() = default;
	SkyboxComponent(const SkyboxComponent&) = default;
};

//TODO: serialization
struct RelationshipComponent
{
	entt::entity first{entt::null};
	entt::entity prev{entt::null};
	entt::entity next{entt::null};
	entt::entity parent{entt::null};
};

template<typename... Component>
struct ComponentGroup
{
};

using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent,
		CameraComponent, MeshRendererComponent, LightComponent, BoxColliderComponent, SphereColliderComponent, SkyboxComponent, RelationshipComponent>;

template<typename Component>
std::string GetComponentName()
{
	std::string prettyFunction = __PRETTY_FUNCTION__;
	size_t start = prettyFunction.find("Component = ") + 12;
	size_t end = prettyFunction.find(';', start);
//	std::cout << prettyFunction << std::endl;
//	std::cout << prettyFunction.substr(start, end - start) << std::endl;

	return prettyFunction.substr(start, end - start);

}

template<typename Component>
void PrintComponentName()
{
	std::string prettyFunction = __PRETTY_FUNCTION__;
	size_t start = prettyFunction.find("Component = ") + 12;
	size_t end = prettyFunction.find(']', start);
	std::cout << prettyFunction << std::endl;
	std::cout << prettyFunction.substr(start, end - start) << std::endl;
}

template<typename... Components>
void PrintAllComponentNames(ComponentGroup<Components...>)
{
	(PrintComponentName<Components>(), ...);
}
