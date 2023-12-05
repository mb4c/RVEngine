#pragma once
#include <string>
#include <glm/vec4.hpp>

class Texture
{
public:
	Texture() = default;
	explicit Texture(const std::string& path, bool normalMap = false);
	explicit Texture(uint32_t width, uint32_t height, glm::vec4 color, bool normalMap = false);

	static uint32_t FromFile(const std::string& path, bool normalMap = false);
	void SetTexture(uint32_t id);
	[[nodiscard]] uint32_t GetTexture() const;
	std::string GetPath() {return m_Path;};
private:
	uint32_t m_ID{};
	std::string m_Path;
};
