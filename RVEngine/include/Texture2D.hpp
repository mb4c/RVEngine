#pragma once
#include <string>
#include <glm/vec4.hpp>
#include <filesystem>
#include "UUID.hpp"

class Texture2D
{
public:
	Texture2D() = default;
	explicit Texture2D(const std::string& path, bool normalMap = false, bool nearestFiltering = false);
	explicit Texture2D(uint32_t width, uint32_t height, glm::vec4 color, bool normalMap = false, bool nearestFiltering = false);

	static uint32_t FromFile(const std::string& path, bool normalMap = false, bool nearestFiltering = false);
	void SetTexture(uint32_t id);
	[[nodiscard]] uint32_t GetTexture() const;
	std::string GetPath() {return m_Path;};
	std::string GetName() {return m_Name;};
	void SetName(std::string name) {m_Name = name;};

	void SetRelPath(std::string path) {m_RelativePath = path;};

	void Serialize(const std::filesystem::path& file);
	void Deserialize(const std::filesystem::path& file);
private:
	uint32_t m_ID{};
	std::string m_Path;
	std::string m_Name;
	UUID m_UUID;
	bool m_IsNormalMap = false;
	std::string m_RelativePath;
};
