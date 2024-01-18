#include "AssetsPanel.hpp"
#include "ResourceManager.hpp"
#include "Widgets.hpp"

AssetsPanel::AssetsPanel()
	: m_CurrentDirectory(m_AssetsDirectory)
{

}

void AssetsPanel::OnRender()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		ImGui::PushItemWidth(128);
		ImGui::DragFloat("Thumbnail size", &m_ThumbnailSize, 1, 32, 128);
		ImGui::EndMenuBar();
	}

	{
		ResourceManager& rm = ResourceManager::instance();

		float padding = 16;
		float cellSize = m_ThumbnailSize + padding;

		int columnCount = (int)(ImGui::GetContentRegionAvail().x / cellSize);
		if ( columnCount < 1)
			columnCount = 1;



		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), window_flags);

		if (m_CurrentDirectory != std::filesystem::path(m_AssetsDirectory))
		{
			if (ImGui::Button(".."))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		ImGui::Columns(columnCount, nullptr, false);

		for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			auto relativePath = std::filesystem::relative(it.path(), m_AssetsDirectory);

			if (it.is_directory())
			{
				ImGui::PushStyleColor(ImGuiCol_Button, {0,0,0,0});
				if (ImGui::ImageButton(it.path().c_str(), reinterpret_cast<ImTextureID>(rm.GetTexture("icon_folder")->GetTexture()), {m_ThumbnailSize, m_ThumbnailSize}))
				{
					m_CurrentDirectory /= it.path().filename();
				}
				ImGui::PopStyleColor();
				ImGui::TextWrapped("%s", relativePath.c_str());
			} else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, {0,0,0,0});
				if (ImGui::ImageButton(relativePath.filename().c_str(), reinterpret_cast<ImTextureID>(rm.GetTexture("icon_file")->GetTexture()), {m_ThumbnailSize, m_ThumbnailSize}))
				{
					if (relativePath.extension() == ".rvmat")
					{
						std::filesystem::path matPath = m_CurrentDirectory;
						matPath /= relativePath;
						m_SelectedMaterial.Deserialize(absolute(matPath));
					}

				}
				ImGui::PopStyleColor();
				ImGui::TextWrapped("%s", relativePath.filename().c_str());

			}
			ImGui::NextColumn();
		}

		ImGui::EndChild();
	}



	ImGui::End();
	ImGui::PopStyleVar(1);
}

std::filesystem::path AssetsPanel::GetCurrentDirectory()
{
	return m_CurrentDirectory;
}

void AssetsPanel::SetAssetDirectory(std::filesystem::path dir)
{
	m_AssetsDirectory = dir;
	m_CurrentDirectory = m_AssetsDirectory;
}