#include <Model.hpp>

Model::Model(const std::string& path)
{
	RV_PROFILE_FUNCTION();
	LoadModel(path);
	m_Path = path;
}

void Model::LoadModel(const std::string& path)
{
	RV_PROFILE_FUNCTION();
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path,  aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	m_Directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);

	if (!scene->HasMaterials())
	{
		std::cout << "No materials found." << std::endl;
		return;
	}

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* material = scene->mMaterials[i];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);
		m_MaterialNames.push_back(materialName.C_Str());
	}

}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	RV_PROFILE_FUNCTION();
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes->push_back(ProcessMesh(mesh, scene));
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	RV_PROFILE_FUNCTION();
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};
		// process vertex positions, normals and texture coordinates
		glm::vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		if(mesh->HasTangentsAndBitangents())
		{
			vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
			vertex.Bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
		}

		vertices.push_back(vertex);
	}
	// process indices
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return {vertices, indices, textures};

}
