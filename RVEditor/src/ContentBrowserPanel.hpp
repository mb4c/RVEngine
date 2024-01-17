#pragma once
#include <imgui.h>
#include <string>
#include <filesystem>
#include "Entity.hpp"

class ContentBrowserPanel
{
public:
	ContentBrowserPanel();
	void OnRender();
	std::filesystem::path GetCurrentDirectory();
	void SetAssetDirectory(std::filesystem::path dir);
private:
	 std::filesystem::path m_AssetsDirectory = "res";
	 std::filesystem::path m_CurrentDirectory;
	 float m_ThumbnailSize = 96;


	 Material m_SelectedMaterial;
};
