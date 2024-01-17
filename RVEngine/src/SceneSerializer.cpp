#include <SceneSerializer.hpp>
#include <Entity.hpp>
#include <fstream>
#include <Components.hpp>
#include <YAMLUtils.hpp>

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
	: m_Scene(scene)
{

}

static void SerializeEntity(YAML::Emitter& out, Entity entity)
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

		out << YAML::Key << "ModelPath" << YAML::Value << meshRendererComponent.model->GetPath();
		out << YAML::Key << "FragmentShaderPath" << YAML::Value << meshRendererComponent.shader->m_FragmentPath;
		out << YAML::Key << "VertexShaderPath" << YAML::Value << meshRendererComponent.shader->m_VertexPath;

		//textures
		out << YAML::Key << "Albedo" << YAML::Value << meshRendererComponent.model->m_Material->albedo->GetPath();
		out << YAML::Key << "Normal" << YAML::Value << meshRendererComponent.model->m_Material->normal->GetPath();
		out << YAML::Key << "OcclusionRoughnessMetallic" << YAML::Value << meshRendererComponent.model->m_Material->occlusionRoughnessMetallic->GetPath();


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

	out << YAML::EndMap;

}

void SceneSerializer::Serialize(const std::filesystem::path& path)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << path.stem();
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
	m_Scene->m_Registry.each([&](auto entityID)
							 {
								 Entity entity = { entityID, m_Scene.get() };
								 if (!entity)
									 return;

								 SerializeEntity(out, entity);
							 });
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
				std::shared_ptr<Material> mat = std::make_shared<Material>();

				if (!meshRendererComponent["Albedo"].as<std::string>().empty())
					mat->albedo = std::make_shared<Texture>(Texture(meshRendererComponent["Albedo"].as<std::string>()));
				else
					mat->albedo = rm.GetTexture("default_albedo");
				if (!meshRendererComponent["Normal"].as<std::string>().empty())
					mat->normal = std::make_shared<Texture>(Texture(meshRendererComponent["Normal"].as<std::string>()));
				else
					mat->normal = rm.GetTexture("default_normal");
				if (!meshRendererComponent["OcclusionRoughnessMetallic"].as<std::string>().empty())
					mat->occlusionRoughnessMetallic = std::make_shared<Texture>(Texture(meshRendererComponent["OcclusionRoughnessMetallic"].as<std::string>()));
				else
					mat->normal = rm.GetTexture("default_albedo");

				mrc.model->m_Material = mat;
 				mrc.shader = rm.GetShader("pbr");
			}

			auto lightComponent = entity["LightComponent"];
			if (lightComponent)
			{
				auto& lc = deserializedEntity.AddComponent<LightComponent>();
				lc.color = lightComponent["Color"].as<glm::vec3>();
				lc.intensity = lightComponent["Intensity"].as<float>();

			}
		}
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
