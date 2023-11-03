#pragma once
#include <string>

class Texture
{
public:
	static unsigned int TextureFromFile(const std::string& path, const std::string &directory, bool normalMap = false);
	static unsigned int TextureFromFile(const std::string& path, bool normalMap = false);
	unsigned int id;
	std::string type;
	std::string path;
};
