#pragma once
#include <imgui.h>
#include <string>
#include <filesystem>

class ContentBrowserPanel
{
public:
	ContentBrowserPanel();
	void OnRender();
private:
	 std::filesystem::path m_AssetsDirectory = "res";
	 std::filesystem::path m_CurrentDirectory;
};
