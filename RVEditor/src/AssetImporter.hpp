#pragma once
#include "Modal.hpp"
#include "AppData.hpp"
#include "ProjectSettings.hpp"
#include <algorithm>


class AssetImporter : Modal
{
public:
	explicit AssetImporter() : Modal("Asset importer"){}
	void Open(AppData* appdata, ProjectSettings* projectSettings, std::filesystem::path currentDirectory);
	void Render();
	bool IsOpen();
private:
	AppData* m_AppData = nullptr;
	ProjectSettings* m_ProjectSettings = nullptr;
	std::filesystem::path m_CurrentDirectory;

	enum class AssetType
	{
		UNKNOWN,
		IMAGE,
		MODEL,
		SOUND
	};

	std::vector<std::string> m_ImageExtensions {".jpg", ".png", ".tga", ".bmp", ".hdr"};
	std::vector<std::string> m_ModelExtensions {".fbx", ".obj", ".gltf", ".glb"};

	AssetType AssetTypeFromExtension(const std::string& extension);
	static std::string AssetTypeToString(AssetType assetType);
	void EraseFirstElement();

	static std::string ToLower(const std::string& str)
	{
		std::string data = str;
		std::transform(data.begin(), data.end(), data.begin(),
					   [](unsigned char c)
					   { return std::tolower(c); });
		return data;
	}
};
