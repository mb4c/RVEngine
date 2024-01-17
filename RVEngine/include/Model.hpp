#pragma once


#include <vector>
#include <Shader.hpp>
#include <Mesh.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Texture2D.hpp>
#include <Macros.hpp>
#include <Material.hpp>

class Model
{
public:

	Model(const std::string& path);
	std::shared_ptr<std::vector<Mesh>> GetMeshes(){ RV_PROFILE_FUNCTION(); return m_Meshes;};
	std::shared_ptr<Material> GetMaterial() { RV_PROFILE_FUNCTION(); return m_Material;};
	std::shared_ptr<Material> m_Material;
	std::string GetPath(){ return m_Path; };

private:
	std::shared_ptr<std::vector<Mesh>> m_Meshes = std::make_shared<std::vector<Mesh>>();
	std::string m_Directory;
	std::vector<Texture2D> m_TexturesLoaded;
	std::string m_Path;
	std::vector<std::string> m_MaterialNames; // mesh material names used for generating engine materials
	void LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
