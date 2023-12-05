#include "ContentBrowserPanel.hpp"

ContentBrowserPanel::ContentBrowserPanel()
	: m_CurrentDirectory(m_AssetsDirectory)
{

}

void ContentBrowserPanel::OnRender()
{
	ImGui::Begin("Content Browser");

	if (m_CurrentDirectory != std::filesystem::path(m_AssetsDirectory))
	{
		if (ImGui::Button(".."))
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
	}

	for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
	{
		auto relativePath = std::filesystem::relative(it.path(), m_AssetsDirectory);

		if (it.is_directory())
		{
			if (ImGui::Button(it.path().c_str()))
			{
				m_CurrentDirectory /= it.path().filename();
			}
		} else
		{
			if (ImGui::Button(relativePath.filename().c_str()))
			{

			}
		}
	}

	ImGui::End();
}
