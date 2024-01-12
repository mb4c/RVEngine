#include <filesystem>
#include <iostream>
#include <utility>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "AssetImporter.hpp"
#include <YAMLUtils.hpp>

void AssetImporter::Open(AppData* appdata, ProjectSettings* projectSettings, std::filesystem::path currentDirectory)
{
	Modal::Open();
	m_IsOpen = true;
	m_AppData = appdata;
	m_ProjectSettings = projectSettings;
	m_CurrentDirectory = std::move(currentDirectory);
}

void AssetImporter::Render()
{
	Modal::Render();

	if (ImGui::BeginPopupModal(m_Name.c_str(),  nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{

		if (m_AppData->dropPaths.empty())
			Close();

		ImGui::Text("Assets left: %zu", m_AppData->dropPaths.size());

		if (ImGui::BeginTable("table1", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableSetupColumn("File");
			ImGui::TableSetupColumn("Type");
			ImGui::TableSetupColumn("Path");
			ImGui::TableHeadersRow();
			for (int row = 0; row < m_AppData->dropPaths.size(); row++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); // File
				ImGui::Selectable(std::filesystem::path(m_AppData->dropPaths.at(row)).filename().c_str(), row	== 0, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns);
				ImGui::TableSetColumnIndex(1); // Type
				ImGui::Selectable(AssetTypeToString(AssetTypeFromExtension(m_AppData->dropPaths.at(row).extension())).c_str(), row	== 0, ImGuiSelectableFlags_DontClosePopups);
				ImGui::TableSetColumnIndex(2); // Path
				ImGui::Selectable(m_AppData->dropPaths.at(row).c_str(), row	== 0, ImGuiSelectableFlags_DontClosePopups );

			}
			ImGui::EndTable();
		}


		if (AssetTypeFromExtension(m_AppData->dropPaths.at(0).extension()) == AssetType::IMAGE)
		{
			// TODO: Display metadata
			// TODO: Display Image preview
			// TODO: Display import settings. TextureType, etc
		}

		if (ImGui::Button("Cancel"))
		{
			EraseFirstElement();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel all"))
		{
			Close();
		}
		ImGui::SameLine();
		if (ImGui::Button("Import"))
		{
			auto copyPath = m_ProjectSettings->ProjectPath.parent_path();
			copyPath /= m_CurrentDirectory;
			copyPath /= m_AppData->dropPaths.at(0).filename();
			std::cout << m_CurrentDirectory << std::endl;

			bool copied = std::filesystem::copy_file(m_AppData->dropPaths.at(0), copyPath, std::filesystem::copy_options::overwrite_existing); //TODO: ask user if he wants to overwrite
			if (AssetTypeFromExtension(m_AppData->dropPaths.at(0).extension()) == AssetType::MODEL)
			{
				ImportMesh(copyPath);
			}

			if (copied)
				EraseFirstElement();


		}

		ImGui::EndPopup();
	}
}

bool AssetImporter::IsOpen()
{
	return m_IsOpen;
}

AssetImporter::AssetType AssetImporter::AssetTypeFromExtension(const std::string& extension)
{
	if (std::find(m_ImageExtensions.begin(), m_ImageExtensions.end(), ToLower(extension)) != m_ImageExtensions.end())
		return AssetType::IMAGE;
	if (std::find(m_ModelExtensions.begin(), m_ModelExtensions.end(), ToLower(extension)) != m_ModelExtensions.end())
		return AssetType::MODEL;

	return AssetImporter::AssetType::UNKNOWN;
}

std::string AssetImporter::AssetTypeToString(AssetImporter::AssetType assetType)
{
	switch (assetType)
	{
		case AssetType::IMAGE:
			return "Image";
		case AssetType::UNKNOWN:
			return "Unknown";
		case AssetType::MODEL:
			return "Model";
		case AssetType::SOUND:
			return "Sound";
	}
}

void AssetImporter::EraseFirstElement()
{
	m_AppData->dropPaths.erase(m_AppData->dropPaths.begin());
	if (m_AppData->dropPaths.empty())
		Close();
}

void AssetImporter::ImportMesh(const std::filesystem::path& path)
{
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path,  aiProcess_RemoveRedundantMaterials);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	if (!scene->HasMaterials())
	{
		std::cout << "No materials found." << std::endl;
		return;
	}

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* material = scene->mMaterials[i];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);
//		m_MaterialNames.push_back(materialName.C_Str());
		std::cout << "Name: " << materialName.C_Str() << std::endl;
		//TODO: Generate .rvmat material
		CreateRVMat(materialName.C_Str());

	}
}

void AssetImporter::CreateRVMat(const std::string& materialName)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Material" << YAML::Value << materialName;
	out << YAML::Key << "Shader" << YAML::Value << "default_pbr";

	out << YAML::Key << "Textures";
	out << YAML::BeginMap;
		out << YAML::Key << "Albedo" << YAML::Value << "default_albedo";
		out << YAML::Key << "Normal" << YAML::Value << "default_normal";
		out << YAML::Key << "OcclusionRoughnessMetallic" << YAML::Value << "default_albedo";
	out << YAML::EndMap;

	out << YAML::Key << "Properties";
	out << YAML::BeginMap;
		out << YAML::Key << "AlbedoColor" << YAML::Value << glm::vec4(1,1,1,1);
		out << YAML::Key << "MetallicValue" << YAML::Value << 0.0f;
	out << YAML::EndMap;


	out << YAML::EndMap;
	std::ofstream fout(m_CurrentDirectory.string() + "/" + materialName + ".rvmat");
	fout << out.c_str();
}
