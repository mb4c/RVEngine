#pragma once
#include <Modal.hpp>
#include <string>
#include "../../RVEditor/src/portable-file-dialogs.h"
#include <yaml-cpp/yaml.h>

class NewProjectModal : Modal
{
public:
	explicit NewProjectModal() : Modal("New project"){}
	void Open()
	{
		Modal::Open();
		m_IsOpen = true;
	}

	void Render()
	{
		Modal::Render();

		if (ImGui::BeginPopupModal(m_Name.c_str(),  nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("Project name", &m_ProjectName);
			ImGui::InputText("Path", &m_Path);
			ImGui::SameLine();
			if(ImGui::Button("select..."))
			{
				std::filesystem::path selection = pfd::select_folder("Select a file", ".", pfd::opt::none).result();
				m_Path = selection;
			}

			ImGui::BeginDisabled(m_Path.empty() || m_ProjectName.empty());
			if (ImGui::Button("Create"))
			{
				m_CompletePath = std::filesystem::path(m_Path).append(m_ProjectName);
				std::filesystem::create_directory(m_CompletePath);
				std::filesystem::copy("res",m_CompletePath.string() + "/res", std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

				YAML::Emitter out;
				out << YAML::BeginMap;
				out << YAML::Key << "Project" << YAML::Value << m_ProjectName;
				out << YAML::Key << "ResourcesDirectory" << YAML::Value << "res";
				out << YAML::EndMap;
				std::ofstream fout(m_CompletePath.string() + "/" + m_ProjectName + ".rvproj");
				fout << out.c_str();

				std::cout << "Project " << m_ProjectName << " created at: " << m_CompletePath << std::endl;

				Close();
			}
			ImGui::EndDisabled();
			

			ImGui::SameLine();

			if (ImGui::Button("Close"))
			{
				Close();
			}

			ImGui::EndPopup();
		}
	}
	bool IsOpen()
	{
		return m_IsOpen;
	}
private:
	std::string m_Path;
	std::string m_ProjectName;
	std::filesystem::path m_CompletePath;
};