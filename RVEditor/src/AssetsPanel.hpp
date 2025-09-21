#pragma once
#include <imgui.h>
#include <string>
#include <filesystem>
#include "Entity.hpp"

class AssetsPanel
{
public:
	AssetsPanel();
	void OnRender();
	std::filesystem::path GetCurrentDirectory();
	void SetAssetDirectory(std::filesystem::path dir);
	void SetContext(const std::shared_ptr<Scene>& scene);
	 std::filesystem::path m_CurrentDirectory;

private:
	std::filesystem::path m_AssetsDirectory = "res";
	float m_ThumbnailSize = 96;


	std::shared_ptr<Scene> m_Scene;
	 Material m_SelectedMaterial;
};
