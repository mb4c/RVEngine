#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

class Shader
{
public:
	// the program ID
	unsigned int ID;

	// constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath);
	// use/activate the shader
	void Bind();
	// utility uniform functions
	void SetBool(const std::string &name, bool value) const;
	void SetInt(const std::string &name, int value) const;
	void SetUInt(const std::string &name, unsigned int value) const;
	void SetFloat(const std::string &name, float value) const;
	void SetVec3(const std::string &name, glm::vec3 value) const;
	void SetVec4(const std::string &name, glm::vec4 value) const;
	void SetMat4(const std::string &name, glm::mat4 value) const;
	void SetMat3(const std::string &name, glm::mat3 value) const;
	void CheckCompileErrors(unsigned int shader, std::string type);
	std::string m_VertexPath;
	std::string m_FragmentPath;
};
