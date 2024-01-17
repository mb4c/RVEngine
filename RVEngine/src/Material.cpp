#include <Material.hpp>
#include <YAMLUtils.hpp>
#include "ResourceManager.hpp"

Material::Material()
{
}

void Material::Serialize(const std::filesystem::path& file)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Material" << YAML::Value << materialName;
	out << YAML::Key << "Shader" << YAML::Value << "pbr";

	out << YAML::Key << "Textures";
	out << YAML::BeginMap;
	out << YAML::Key << "Albedo" << YAML::Value << (albedo.get() == nullptr ? "" : albedo->GetName());
	out << YAML::Key << "Normal" << YAML::Value << (normal.get() == nullptr ? "" : normal->GetName());
	out << YAML::Key << "OcclusionRoughnessMetallic" << YAML::Value << (occlusionRoughnessMetallic.get() == nullptr ? "" : normal->GetName());
	out << YAML::EndMap;

	out << YAML::Key << "Properties";
	out << YAML::BeginMap;
	out << YAML::Key << "UseAlbedo" << YAML::Value << useAlbedo;
	out << YAML::Key << "UseNormal" << YAML::Value << useNormal;
	out << YAML::Key << "UseORM" << YAML::Value << useORM;
	out << YAML::Key << "AlbedoColor" << YAML::Value << albedoColor;
	out << YAML::Key << "MetallicValue" << YAML::Value << metallicValue;
	out << YAML::Key << "RoughnessValue" << YAML::Value << roughnessValue;
	out << YAML::EndMap;
	out << YAML::EndMap;

	std::ofstream fout(file);
	fout << out.c_str();
}

void Material::Deserialize(const std::filesystem::path& file)
{
	ResourceManager& rm = ResourceManager::instance();

	std::ifstream stream(file);
	std::stringstream strStream;
	strStream << stream.rdbuf();

//	std::cout << file << std::endl;

	YAML::Node data = YAML::Load(strStream.str());
	if (!data["Material"])
		std::cout << "No data!" << std::endl;

	std::cout << "str: " << strStream.str() << std::endl;
	materialName = data["Material"].as<std::string>();
	shader = rm.GetShader(data["Shader"].as<std::string>());

	auto albedoStr = data["Textures"]["Albedo"].as<std::string>();
	auto normalStr = data["Textures"]["Normal"].as<std::string>();
	auto ormStr = data["Textures"]["OcclusionRoughnessMetallic"].as<std::string>();

	albedo = rm.GetTexture(albedoStr.empty() ? "default_albedo" : albedoStr);
	normal = rm.GetTexture(normalStr.empty() ? "default_normal" : normalStr);
	occlusionRoughnessMetallic = rm.GetTexture(ormStr.empty() ? "default_albedo" : ormStr);

	useAlbedo = data["Properties"]["UseAlbedo"].as<bool>();
	useNormal = data["Properties"]["UseNormal"].as<bool>();
	useORM = data["Properties"]["UseORM"].as<bool>();
	albedoColor = data["Properties"]["AlbedoColor"].as<glm::vec4>();
	metallicValue = data["Properties"]["MetallicValue"].as<float>();
	roughnessValue = data["Properties"]["RoughnessValue"].as<float>();

}

