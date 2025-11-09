#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>
#include <Renderer/Model.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "Renderer/SceneCamera.hpp"

#include "BodyUserData.hpp"
#include "PhysicsTypes.hpp"
#include "Types.hpp"

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
	u32 EntityID = entt::null;

	IDComponent() = default;
	IDComponent(const UUID& id) : ID(id) {}
	IDComponent(const IDComponent&) = default;
	IDComponent(IDComponent&&) = default;
	IDComponent& operator=(const IDComponent&) = default;
	IDComponent& operator=(IDComponent&&) = default;

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
		RV_PROFILE_FUNCTION();
		glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

		return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
	}
	glm::mat4 GetTransform() const
	{
		RV_PROFILE_FUNCTION();
		return GlobalTransform;
	}

	void Translate(glm::vec3 translation)
	{
		RV_PROFILE_FUNCTION();
		Translation += translation;
		IsDirty = true;
	}

	// Rotate in degrees
	void Rotate(glm::vec3 rotation)
	{
		RV_PROFILE_FUNCTION();
		Rotation += glm::radians(rotation);
		IsDirty = true;
	}
	void SetScale(glm::vec3 scale)
	{
		RV_PROFILE_FUNCTION();
		Scale = scale;
		IsDirty = true;
	}
	void SetPosition(glm::vec3 position)
	{
		RV_PROFILE_FUNCTION();
		Translation = position;
		IsDirty = true;
	}
	glm::vec3 GetPosition()
	{
		RV_PROFILE_FUNCTION();
		return Translation;
	}
	void SetRotation(glm::vec3 rotation)
	{
		RV_PROFILE_FUNCTION();
		Rotation = glm::radians(rotation);
		IsDirty = true;
	}
	void SetRotationRad(glm::vec3 rotation)
	{
		RV_PROFILE_FUNCTION();
		Rotation = rotation;
		IsDirty = true;
	}
	glm::vec3 GetRotationRad()
	{
		RV_PROFILE_FUNCTION();
		return Rotation;
	}
	glm::quat GetRotationQuat()
	{
		RV_PROFILE_FUNCTION();
		return glm::quat(Rotation);
	}
	glm::vec3 GetRotation()
	{
		RV_PROFILE_FUNCTION();
		return glm::degrees(Rotation);
	}

	glm::vec3 GetForward()
	{
		glm::vec3 localForward = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 worldForward = glm::toMat3(GetRotationQuat()) * localForward;
		return glm::normalize(worldForward);
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
enum class LightType
{
	Point = 0,
	Directional,
};
struct LightComponent
{
	LightType type = LightType::Point;
	glm::vec3 color = {1, 1, 1};
	f32 intensity = {1.0};

	LightComponent() = default;
	LightComponent(const LightComponent&) = default;
	LightComponent(const glm::vec3 & newcolor, const f32 _intensity)
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
	RV::EMotionType MotionType = RV::EMotionType::Dynamic;
	RV::ECollisionLayer CollisionLayer = RV::ECollisionLayer::Dynamic;
	f32 Mass = 10;
	f32 Restitution = 0.5f;
	f32 Friction = 0.2f;
	u32 IndexSequence = 0xffffffff;
	bool IsDestroyed = false;

	BoxColliderComponent() = default;
	BoxColliderComponent(const BoxColliderComponent&) = default;
};

struct SphereColliderComponent
{
	f32 Radius = 0.5;
	bool Dynamic = true;
	f32 Mass = 10;
	f32 Restitution = 0.5f;
	f32 Friction = 0.2f;
	u32 IndexSequence;
	BodyUserData userData;

	SphereColliderComponent() = default;
	SphereColliderComponent(const SphereColliderComponent&) = default;
};

struct SkyboxComponent
{
	std::string envMap;
	u32 envCubemap;
	u32 irradianceMap;
	u32 prefilterMap;
	u32 brdfLUTTexture;

	SkyboxComponent() = default;
	SkyboxComponent(const SkyboxComponent&) = default;
};

struct RelationshipComponent
{
	entt::entity first{entt::null};
	entt::entity prev{entt::null};
	entt::entity next{entt::null};
	entt::entity parent{entt::null};

	u64 uuidFirst{};
	u64 uuidPrev{};
	u64 uuidNext{};
	u64 uuidParent{};
};


enum class EBulletPattern
{
	Line = 0,
	one,
	None
};

enum class EEnemyMovementType
{
	Horizontally = 0,
	Sinusoidal,
	None
};

struct EnemyComponent
{
	i32 Health = 100;
	i32 Damage = 10;
	f32 Speed = 5;
	i32 Armor = 10;
	f32 AttackCooldown = 0.5f;
	EBulletPattern BulletPattern = EBulletPattern::Line;
	EEnemyMovementType EnemyMovementType = EEnemyMovementType::Horizontally;
	f32 MovementTimer = 0.0;
	bool MoveRight = true;
	i32 MoneyDropMin = 2;
	i32 MoneyDropMax = 5;
	i32 MoneyDropAmount = 0; // needs to be specified when spawning enemy
};

struct PlayerComponent
{
	i32 Health = 100;
	i32 Money = 0;
	u32 EnginesLevel = 0;
	u32 MainGunLevel = 0;
	u32 ArmorLevel = 0;

	f32 MovementSpeedLevel0 = 24;
	f32 MovementSpeedLevel1 = 28;
	f32 MovementSpeedLevel2 = 32;
	f32 MovementSpeedLevel3 = 38;
	f32 MovementSpeedLevel4 = 42;
	f32 MovementSpeedLevel5 = 48;
};

struct BulletComponent
{
	glm::vec3 Velocity = {0,0,0};
	glm::vec3 Position = {0,0,0};

	i32 Damage = 10;
	f32 LifeTime = 2.0f;
	f32 RemainingLifeTime = 2.0f;
	bool Friendly = true;
	bool Initialized = false;
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
#if defined(__clang__) || defined(__GNUC__)
	std::string prettyFunction = __PRETTY_FUNCTION__;
	std::string key = "Component = ";
	size_t start = prettyFunction.find(key) + key.size();
	size_t end = prettyFunction.find(']', start);
	return prettyFunction.substr(start, end - start);
#elif defined(_MSC_VER)
	std::string funcSig = __FUNCSIG__;
	std::string key = "GetComponentName<";
	size_t start = funcSig.find(key) + key.size();
	size_t end = funcSig.find(">", start);
	return funcSig.substr(start, end - start);
#else
	return "Unsupported compiler";
#endif
}



template<typename Component>
void PrintComponentName()
{
	auto componentName = GetComponentName<Component>();
	std::cout << componentName << std::endl;
}

template<typename... Components>
void PrintAllComponentNames(ComponentGroup<Components...>)
{
	(PrintComponentName<Components>(), ...);
}
