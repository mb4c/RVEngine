#pragma once
#include <string>
#include <glm/vec4.hpp>

class Texture2D
{
public:
	Texture2D() = default;
	explicit Texture2D(const std::string& path, bool normalMap = false);
	explicit Texture2D(uint32_t width, uint32_t height, glm::vec4 color, bool normalMap = false);

	static uint32_t FromFile(const std::string& path, bool normalMap = false);
	void SetTexture(uint32_t id);
	[[nodiscard]] uint32_t GetTexture() const;
	std::string GetPath() {return m_Path;};
	std::string GetName() {return m_Name;};
	void SetName(std::string name) {m_Name = name;};
private:
	uint32_t m_ID{};
	std::string m_Path;
	std::string m_Name;
};
