#include <Material.hpp>
#include <YAMLUtils.hpp>
#include "ResourceManager.hpp"

Material::Material()
{
}

void Material::Serialize(std::filesystem::path file)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Material" << YAML::Value << materialName;
	out << YAML::Key << "Shader" << YAML::Value << "default_pbr";

	out << YAML::Key << "Textures";
	out << YAML::BeginMap;
	out << YAML::Key << "Albedo" << YAML::Value << albedo->GetName();
	out << YAML::Key << "Normal" << YAML::Value << normal->GetName();
	out << YAML::Key << "OcclusionRoughnessMetallic" << YAML::Value << albedo->GetName();
	out << YAML::EndMap;

	out << YAML::Key << "Properties";
	out << YAML::BeginMap;
	out << YAML::Key << "AlbedoColor" << YAML::Value << albedoColor;
	out << YAML::Key << "MetallicValue" << YAML::Value << metallicValue;
	out << YAML::EndMap;
}

void Material::Deserialize(const std::filesystem::path& file)
{
	ResourceManager& rm = ResourceManager::instance();

//	std::ifstream stream(file);
//	std::stringstream strStream;
//	strStream << stream.rdbuf();

	std::ifstream stream(file);
	std::stringstream strStream;
	strStream << stream.rdbuf();


	YAML::Node data = YAML::Load(strStream.str());
	if (!data["Material"])
		std::cout << "No data!" << std::endl;

	std::cout << "str: " << strStream.str() << std::endl;
	materialName = data["Material"].as<std::string>();
	shader = rm.GetShader(data["Shader"].as<std::string>());
	albedo = rm.GetTexture(data["Textures"]["Albedo"].as<std::string>());
	normal = rm.GetTexture(data["Textures"]["Normal"].as<std::string>());
	occlusionRoughnessMetallic = rm.GetTexture(data["Textures"]["OcclusionRoughnessMetallic"].as<std::string>());

	albedoColor = data["Properties"]["AlbedoColor"].as<glm::vec4>();
	metallicValue = data["Properties"]["MetallicValue"].as<float>();

}

