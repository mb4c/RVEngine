#pragma once


#include <vector>
#include <Renderer/Shader.hpp>
#include <Renderer/Mesh.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Renderer/Texture2D.hpp>
#include <Macros.hpp>
#include <Renderer/Material.hpp>

class Model
{
public:
	Model() = default;
	Model(const std::string& path);
	std::shared_ptr<std::vector<Mesh>> GetMeshes(){ RV_PROFILE_FUNCTION(); return m_Meshes;};
	std::string GetPath(){ return m_Path; };
	void AddMesh(const Mesh& mesh){m_Meshes->push_back(mesh);}
	void ClearMeshes() {m_Meshes->clear();}
	void SetMaterial(const std::shared_ptr<Material>& material);

private:
	std::shared_ptr<std::vector<Mesh>> m_Meshes = std::make_shared<std::vector<Mesh>>();
	std::string m_Directory;
	std::string m_Path;
	void LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
