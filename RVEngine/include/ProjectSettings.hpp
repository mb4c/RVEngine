#pragma once

#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <fstream>

class ProjectSettings
{
public:
	std::string ProjectName;
	std::filesystem::path ProjectPath; //not serialized
	std::filesystem::path ResourcesDirectory; // project resources relative to project path

	bool Serialize(const std::filesystem::path& file)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << ProjectName;
		out << YAML::Key << "ResourcesDirectory" << YAML::Value << ResourcesDirectory;

		out << YAML::EndMap;

		std::ofstream fout(file);
		fout << out.c_str();
		return true;
	}

	bool Deserialize(const std::filesystem::path& file)
	{
		ProjectPath = file;
		std::ifstream stream(file);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		ProjectName = data["Project"].as<std::string>();
		ResourcesDirectory = data["ResourcesDirectory"].as<std::string>();
		return true;
	}
};
