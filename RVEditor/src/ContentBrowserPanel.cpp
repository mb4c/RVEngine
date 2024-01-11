#include "ContentBrowserPanel.hpp"
#include "ResourceManager.hpp"

ContentBrowserPanel::ContentBrowserPanel()
	: m_CurrentDirectory(m_AssetsDirectory)
{

}

void ContentBrowserPanel::OnRender()
{
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		ImGui::PushItemWidth(128);
		ImGui::DragFloat("Thumbnail size", &m_ThumbnailSize, 1, 32, 128);
		ImGui::EndMenuBar();
	}

//	{
//		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
//		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.15f, ImGui::GetContentRegionAvail().y), window_flags);
//		ImGui::Text("TODO");
//		ImGui::EndChild();
//	}
//	ImGui::SameLine();



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

				}
				ImGui::PopStyleColor();
				ImGui::TextWrapped("%s", relativePath.filename().c_str());

			}
			ImGui::NextColumn();
		}

		ImGui::EndChild();
	}



	ImGui::End();
}

std::filesystem::path ContentBrowserPanel::GetCurrentDirectory()
{
	return m_CurrentDirectory;
}

void ContentBrowserPanel::SetAssetDirectory(std::filesystem::path dir)
{
	m_AssetsDirectory = dir;
	m_CurrentDirectory = m_AssetsDirectory;
}
