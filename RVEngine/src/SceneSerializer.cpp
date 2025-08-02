#include <SceneSerializer.hpp>
#include <Entity.hpp>
#include <fstream>
#include <Components.hpp>
#include <YAMLUtils.hpp>
#include "../include/Renderer/EnvironmentMap.hpp"

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
	: m_Scene(scene)
{

}

static void SerializeEntity(YAML::Emitter& out, Entity entity, Scene* scene)
{
	out << YAML::BeginMap;
	out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();


	if (entity.HasComponent<TagComponent>())
	{
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		out << YAML::Key << "Tag" << YAML::Value << tag;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<TransformComponent>())
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap; // TransformComponent

		auto& tc = entity.GetComponent<TransformComponent>();
		out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

		out << YAML::EndMap; // TransformComponent
	}

	if (entity.HasComponent<CameraComponent>())
	{
		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap; // CameraComponent

		auto& cameraComponent = entity.GetComponent<CameraComponent>();
		auto& camera = cameraComponent.Camera;

		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap; // Camera
		out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
		out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
		out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
		out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
		out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
		out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
		out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
		out << YAML::EndMap; // Camera

		out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
		out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

		out << YAML::EndMap; // CameraComponent
	}

	if (entity.HasComponent<MeshRendererComponent>())
	{
		out << YAML::Key << "MeshRendererComponent";
		out << YAML::BeginMap; // MeshRendererComponent

		auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
		ResourceManager& rm = ResourceManager::instance();

		out << YAML::Key << "ModelPath" << YAML::Value << meshRendererComponent.model->GetPath();
		out << YAML::Key << "Shader" << YAML::Value << rm.GetShaderName(meshRendererComponent.shader->ID);

		out << YAML::EndMap; // MeshRendererComponent
	}

	if (entity.HasComponent<LightComponent>())
	{
		out << YAML::Key << "LightComponent";
		out << YAML::BeginMap;

		auto& lc = entity.GetComponent<LightComponent>();
		out << YAML::Key << "Color" << YAML::Value << lc.color;
		out << YAML::Key << "Intensity" << YAML::Value << lc.intensity;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<BoxColliderComponent>())
	{
		out << YAML::Key << "BoxColliderComponent";
		out << YAML::BeginMap;

		auto& bcc = entity.GetComponent<BoxColliderComponent>();
		out << YAML::Key << "Size" << YAML::Value << bcc.Size;
		out << YAML::Key << "MotionType" << YAML::Value << (uint8_t)bcc.MotionType;
		out << YAML::Key << "Mass" << YAML::Value << bcc.Mass;
		out << YAML::Key << "Restitution" << YAML::Value << bcc.Restitution;
		out << YAML::Key << "Friction" << YAML::Value << bcc.Friction;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<SphereColliderComponent>())
	{
		out << YAML::Key << "SphereColliderComponent";
		out << YAML::BeginMap;

		auto& scc = entity.GetComponent<SphereColliderComponent>();
		out << YAML::Key << "Radius" << YAML::Value << scc.Radius;
		out << YAML::Key << "Dynamic" << YAML::Value << scc.Dynamic;
		out << YAML::Key << "Mass" << YAML::Value << scc.Mass;
		out << YAML::Key << "Restitution" << YAML::Value << scc.Restitution;
		out << YAML::Key << "Friction" << YAML::Value << scc.Friction;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<RelationshipComponent>())
	{
		out << YAML::Key << "RelationshipComponent";
		out << YAML::BeginMap;

		auto& rc = entity.GetComponent<RelationshipComponent>();

		if (auto first = Entity(rc.first, scene))
		{
			out << YAML::Key << "first" << YAML::Value << Entity(rc.first, scene).GetUUID();
		}
		else
			out << YAML::Key << "first" << YAML::Value << (uint64_t) entt::null;

		if (auto next = Entity(rc.next, scene))
		{
			out << YAML::Key << "next" << YAML::Value << Entity(rc.next, scene).GetUUID();
		}
		else
			out << YAML::Key << "next" << YAML::Value << (uint64_t) entt::null;

		if (auto prev = Entity(rc.prev, scene))
		{
			out << YAML::Key << "prev" << YAML::Value << Entity(rc.prev, scene).GetUUID();
		}
		else
			out << YAML::Key << "prev" << YAML::Value << (uint64_t) entt::null;

		if (auto parent = Entity(rc.parent, scene))
		{
			out << YAML::Key << "parent" << YAML::Value << entity.GetParent().GetUUID();
		}
		else
			out << YAML::Key << "parent" << YAML::Value << (uint64_t) entt::null;


		out << YAML::EndMap;
	}

	if (entity.HasComponent<SkyboxComponent>())
	{
		out << YAML::Key << "SkyboxComponent";
		out << YAML::BeginMap;

		auto& sc = entity.GetComponent<SkyboxComponent>();
		out << YAML::Key << "envMap" << YAML::Value << sc.envMap;

		out << YAML::EndMap;
	}

	out << YAML::EndMap;

}

void SceneSerializer::Serialize(const std::filesystem::path& path)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << path.stem().u8string();
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
	for (auto entityID: m_Scene->m_Registry.view<entt::entity>())
	{
		Entity entity = {entityID, m_Scene.get()};
		if (!entity)
			return;

		SerializeEntity(out, entity, m_Scene.get());
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::filesystem::path directory = std::filesystem::path(path).parent_path();

	if (!std::filesystem::exists(directory)) {
		std::filesystem::create_directories(directory);
	}


	std::ofstream fout(path);
	fout << out.c_str();
}

bool SceneSerializer::Deserialize(const std::filesystem::path& path)
{
	std::ifstream stream(path);
	std::stringstream strStream;
	strStream << stream.rdbuf();

	YAML::Node data = YAML::Load(strStream.str());
	if (!data["Scene"])
		return false;

	m_Scene->m_SceneName = data["Scene"].as<std::string>();
	ResourceManager& rm = ResourceManager::instance();

	auto entities = data["Entities"];
	if (entities)
	{
		for (auto entity : entities)
		{
			uint64_t uuid = entity["Entity"].as<uint64_t>();


			std::string name;
			auto tagComponent = entity["TagComponent"];
			if (tagComponent)
				name = tagComponent["Tag"].as<std::string>();


			Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

			auto transformComponent = entity["TransformComponent"];
			if (transformComponent)
			{
				// Entities always have transforms
				auto& tc = deserializedEntity.GetComponent<TransformComponent>();
				tc.Translation = transformComponent["Translation"].as<glm::vec3>();
				tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
				tc.Scale = transformComponent["Scale"].as<glm::vec3>();
			}

			auto cameraComponent = entity["CameraComponent"];
			if (cameraComponent)
			{
				auto& cc = deserializedEntity.AddComponent<CameraComponent>();

				YAML::Node cameraProps = cameraComponent["Camera"];
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

				cc.Primary = cameraComponent["Primary"].as<bool>();
				cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
			}

			auto meshRendererComponent = entity["MeshRendererComponent"];
			if (meshRendererComponent)
			{
				auto& mrc = deserializedEntity.AddComponent<MeshRendererComponent>();
				mrc.model = std::make_shared<Model>(meshRendererComponent["ModelPath"].as<std::string>());
				mrc.shader = rm.GetShader(meshRendererComponent["Shader"].as<std::string>());
			}

			auto lightComponent = entity["LightComponent"];
			if (lightComponent)
			{
				auto& lc = deserializedEntity.AddComponent<LightComponent>();
				lc.color = lightComponent["Color"].as<glm::vec3>();
				lc.intensity = lightComponent["Intensity"].as<float>();

			}

			auto boxCollider = entity["BoxColliderComponent"];
			if (boxCollider)
			{
				auto& bcc = deserializedEntity.AddComponent<BoxColliderComponent>();
				bcc.Size = boxCollider["Size"].as<glm::vec3>();
				bcc.MotionType = (RV::EMotionType)boxCollider["Dynamic"].as<uint8_t>();
				bcc.Mass = boxCollider["Mass"].as<float>();
				bcc.Restitution = boxCollider["Restitution"].as<float>();
				bcc.Friction = boxCollider["Friction"].as<float>();
			}

			auto sphereCollider = entity["SphereColliderComponent"];
			if (sphereCollider)
			{
				auto& scc = deserializedEntity.AddComponent<SphereColliderComponent>();
				scc.Radius = sphereCollider["Radius"].as<float>();
				scc.Dynamic = sphereCollider["Dynamic"].as<bool>();
				scc.Mass = sphereCollider["Mass"].as<float>();
				scc.Restitution = sphereCollider["Restitution"].as<float>();
				scc.Friction = sphereCollider["Friction"].as<float>();
			}

			auto relationshipComponent = entity["RelationshipComponent"];
			if (relationshipComponent)
			{
				auto& rc = deserializedEntity.GetComponent<RelationshipComponent>();

				 rc.uuidFirst = relationshipComponent["first"].as<uint64_t>();
				 rc.uuidPrev = relationshipComponent["prev"].as<uint64_t>();
				 rc.uuidNext = relationshipComponent["next"].as<uint64_t>();
				 rc.uuidParent = relationshipComponent["parent"].as<uint64_t>();
			}

			auto skyboxComponent = entity["SkyboxComponent"];
			if (skyboxComponent)
			{
				auto& sc = deserializedEntity.AddComponent<SkyboxComponent>();

				sc.envMap = skyboxComponent["envMap"].as<std::string>();
				EnvironmentMap envMap(sc.envMap);
				envMap.Capture();
				sc.envCubemap = envMap.envCubemap;
				sc.irradianceMap = envMap.irradianceMap;
				sc.prefilterMap = envMap.prefilterMap;
				sc.brdfLUTTexture = envMap.brdfLUTTexture;
			}

		}

		RelationshipDeserialization();
	}
	return true;
}

void SceneSerializer::SerializeRuntime(const std::filesystem::path& path)
{

}

void SceneSerializer::DeserializeRuntime(const std::filesystem::path& path)
{

}

void SceneSerializer::SetContext(const std::shared_ptr<Scene>& scene)
{
	m_Scene = scene;
}

void SceneSerializer::RelationshipDeserialization()
{
	for (auto entityID: m_Scene->m_Registry.view<entt::entity>())
	{
		Entity entity = {entityID, m_Scene.get()};
		if (!entity)
			return;

//		std::cout << "EntityMap size: "<< m_Scene->m_EntityMap.size() << std::endl;
		for(auto const& entityPair: m_Scene->m_EntityMap)
		{
//			std::cout << "UUID: " << entityPair.first << " entity: " << (uint32_t)entityPair.second << std::endl;
		}

		auto& rc = entity.GetComponent<RelationshipComponent>();
		uint64_t nullEntity = 18446744073709551615;

		if (rc.uuidFirst != nullEntity)
			rc.first = m_Scene->m_EntityMap.at(rc.uuidFirst);

		if (rc.uuidPrev != nullEntity)
			rc.prev = m_Scene->m_EntityMap.at(rc.uuidPrev);

		if (rc.uuidNext != nullEntity)
			rc.next = m_Scene->m_EntityMap.at(rc.uuidNext);

		if (rc.uuidParent != nullEntity)
			rc.parent = m_Scene->m_EntityMap.at(rc.uuidParent);
	}
}
