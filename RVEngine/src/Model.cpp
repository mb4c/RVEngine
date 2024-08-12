#include <Model.hpp>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/filesystem.h>

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
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
												 aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_RemoveRedundantMaterials);
	m_Path = path;

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
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
//	std::cout << "Model "<< path <<" has: "<< scene->mNumMaterials << " materials" << std::endl;

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* material = scene->mMaterials[i];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);

		m_Material = std::make_shared<Material>();
		m_Material->materialName = materialName.C_Str();
//		std::cout <<"Material name: " << materialName.C_Str() << std::endl;

		unsigned int textureCount = material->GetTextureCount(static_cast<aiTextureType>(aiTextureType_BASE_COLOR));

		for (unsigned int type = aiTextureType_DIFFUSE; type <= aiTextureType_AMBIENT_OCCLUSION; ++type)
		{
			for (int j = 0; j < textureCount; ++j)
			{
				aiString path;
//				std::cout << path.C_Str() << std::endl;
				if (material->GetTexture(static_cast<aiTextureType>(type), j, &path) == AI_SUCCESS)
				{
//					std::cout << "Texture type: " << aiTextureTypeToString((aiTextureType) type) << std::endl;

					if (path.length > 0 && path.data[0] == '*')
					{
						// Embedded texture found
						const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());

						if (texture)
						{
							// Process the texture data
//							std::cout << "Embedded texture found: " << path.C_Str() << std::endl;
//							std::cout << "Texture width: " << texture->mWidth << std::endl;
//							std::cout << "Texture height: " << texture->mHeight << std::endl;

							if (texture->mHeight == 0)
							{
								// Compressed texture, texture->mWidth is the size of the texture in bytes
//								std::cout << "Compressed texture data size: " << texture->mWidth << " bytes"
//										  << std::endl;
//								std::cout << "Compressed format: " << (texture->CheckFormat("jpg") ? "jpg" : "png")
//										  << std::endl;


								OIIO::Filesystem::IOMemReader memreader(texture->pcData,
																		texture->mWidth);  // I/O proxy object

								auto in = OIIO::ImageInput::open((texture->CheckFormat("jpg") ? "in.jpg" : "in.png"), nullptr, &memreader);
								auto pixels = std::unique_ptr<unsigned char[]>(
										new unsigned char[in->spec().width * in->spec().height * in->spec().nchannels]);
								in->read_image(0, 0, 0, in->spec().nchannels, OIIO::TypeDesc::UINT8, &pixels[0]);
								const OIIO::ImageSpec& spec = in->spec();

								//NOTE: this commented out code writes the texture to disk for debugging purpose, do not remove
//								std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(std::string(materialName.C_Str()).append(aiTextureTypeToString((aiTextureType) type)).append(".png"));
//								if (!out)
//									return;  // error
//								out->open(std::string(materialName.C_Str()).append(aiTextureTypeToString((aiTextureType) type)).append(".png"), spec);
//								out->write_image(OIIO::TypeDesc::UINT8, &pixels[0]);
//								out->close();


								switch (type)
								{
									case aiTextureType_BASE_COLOR:
									{
										m_Material->albedo = std::make_shared<Texture2D>(spec.width, spec.height,
																						 spec.nchannels, pixels.get());
//										std::cout << "Added albedo" << std::endl;
										break;
									}

									case aiTextureType_NORMALS:
									{
										m_Material->normal = std::make_shared<Texture2D>(spec.width, spec.height,
																						 spec.nchannels, pixels.get());
//										std::cout << "Added normal" << std::endl;
										break;
									}

									case aiTextureType_METALNESS:
									{
										m_Material->metallic = std::make_shared<Texture2D>(spec.width, spec.height,
																						   spec.nchannels,
																						   pixels.get());
//										std::cout << "Added metallnes" << std::endl;
										break;
									}

									case aiTextureType_DIFFUSE_ROUGHNESS:
									{
										m_Material->roughness = std::make_shared<Texture2D>(spec.width, spec.height,
																							spec.nchannels,
																							pixels.get());
//										std::cout << "Added roughness" << std::endl;
										break;
									}

									case aiTextureType_AMBIENT_OCCLUSION:
									{
										m_Material->occlusion = std::make_shared<Texture2D>(spec.width, spec.height,
																							spec.nchannels,
																							pixels.get());
//										std::cout << "Added ao" << std::endl;
										break;
									}
									case aiTextureType_EMISSIVE:
									{
										m_Material->emission = std::make_shared<Texture2D>(spec.width, spec.height,
																							spec.nchannels,
																							pixels.get());
//										std::cout << "Added ao" << std::endl;
										break;
									}

								}

//								std::cout << "Decoded JPEG image: " << spec.width << "x" << spec.height << ", "
//										  << spec.nchannels << " channels" << std::endl;
								in->close();

							} else
							{
								// Uncompressed texture

//								std::cout << "Uncompressed texture with dimensions: " << texture->mWidth << " x "
//										  << texture->mHeight << std::endl;
							}
						} else
						{
//							std::cerr << "Failed to retrieve embedded texture:  " << path.C_Str() << std::endl;
						}
					}
				}
			}
		}
		if (m_Material->albedo == nullptr)
		{
			m_Material->albedo = std::make_shared<Texture2D>(Texture2D(256, 256, {1, 0, 1, 1}));
//			std::cout << "	Albedo missing using fallback " << std::endl;
		}
		if (m_Material->normal == nullptr)
		{
			m_Material->normal = std::make_shared<Texture2D>(Texture2D(256, 256, {0.5, 0.5, 1, 1}));
//			std::cout << "	Normal missing using fallback" << std::endl;
		}
		if (m_Material->roughness == nullptr)
		{
			m_Material->roughness = std::make_shared<Texture2D>(Texture2D(256, 256, {1, 1, 1, 1}));
//			std::cout << "	Roughness missing using fallback" << std::endl;
		}
		if (m_Material->metallic == nullptr)
		{
			m_Material->metallic = std::make_shared<Texture2D>(Texture2D(256, 256, {0, 0, 0, 0}));
//			std::cout << "	Metalness missing using fallback" << std::endl;
		}
		if (m_Material->occlusion == nullptr)
		{
			m_Material->occlusion = std::make_shared<Texture2D>(Texture2D(256, 256, {1, 1, 1, 1}));
//			std::cout << "	Ambient occlusion missing using fallback" << std::endl;
		}
		if (m_Material->emission == nullptr)
		{
			m_Material->emission = std::make_shared<Texture2D>(Texture2D(256, 256, {0, 0, 0, 0}));
//			std::cout << "	Emission missing using fallback" << std::endl;
		}
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
	std::vector<Texture2D> textures;

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
		else
		{
			std::cout << m_Path << " HasTangentsAndBitangents() == false" <<  std::endl;
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
