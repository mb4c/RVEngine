#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Material.hpp"
#include "VertexArray.hpp"
#include "Texture2D.hpp"
#include "Types.hpp"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;

};



class Mesh
{
public:
	Mesh() = default;
	Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture2D> textures);
	void SetupBuffers();
	void RecalculateNormals();
	std::shared_ptr<Material> GetMaterial() const { return m_Material; }
	void SetMaterial(std::shared_ptr<Material> material) { m_Material = material; }
	unsigned int GetMaterialIndex() const { return m_MaterialIndex; }
	void SetMaterialIndex(unsigned int index) { m_MaterialIndex = index; }

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<Texture2D> m_Textures;
	std::shared_ptr<VertexArray> m_VertexArray;
	std::shared_ptr<Material> m_Material;
	u32 m_MaterialIndex = 0;

};

