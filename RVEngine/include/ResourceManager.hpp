#pragma once

#include <Singleton.hpp>
#include <Shader.hpp>
#include <unordered_map>
#include <Model.hpp>
#include <Texture.hpp>

class ResourceManager : public Singleton<ResourceManager>
{
public:
	ResourceManager(token)
	{
		m_Models.emplace("plane", std::make_shared<Model>("res/plane.fbx"));

		m_Shaders.emplace("pbr", std::make_shared<Shader>("res/shaders/PBR_vert.glsl", "res/shaders/PBR_frag.glsl"));
		m_Shaders.emplace("flat", std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag"));
		m_Shaders.emplace("sprite", std::make_shared<Shader>("res/shaders/Sprite.vert", "res/shaders/Sprite.frag"));

		m_Textures.emplace("brickwall_albedo", std::make_shared<Texture>(Texture("res/brickwall.jpg")));
		m_Textures.emplace("brickwall_normal", std::make_shared<Texture>(Texture("res/brickwall_normal.jpg")));
		m_Textures.emplace("brickwall_orm", std::make_shared<Texture>(Texture("res/brickwall_ORM.png")));


		// Default textures
		m_Textures.emplace("default_normal", std::make_shared<Texture>(Texture(256,256,{0.5,0.5,1,1})));
		m_Textures.emplace("default_albedo", std::make_shared<Texture>(Texture(256,256,{1,1,1,1})));
		m_Textures.emplace("default_missing", std::make_shared<Texture>(Texture("res/missing.png")));



		auto brickwallMat = std::make_shared<Material>();
		brickwallMat->albedo = GetTexture("brickwall_albedo");
		brickwallMat->normal= GetTexture("brickwall_normal");
		brickwallMat->occlusionRoughnessMetallic = GetTexture("brickwall_orm");
		m_Materials.emplace("brickwall", brickwallMat);
	};

	template<class T>
	T Get(const std::string& name, std::unordered_map<std::string, T> map)
	{
		if (auto it = map.find(name); it != map.end())
		{
			return it->second;
		} else
		{
			return map.at("default_missing");
		}
	}

	std::shared_ptr<Model> GetModel(const std::string& name)
	{
		return m_Models.at(name);
	}

	std::shared_ptr<Shader> GetShader(const std::string& name)
	{
		return m_Shaders.at(name);
	}

	std::shared_ptr<Texture> GetTexture(const std::string& name)
	{
		return Get<std::shared_ptr<Texture>>(name, m_Textures);
	}

	std::shared_ptr<Material> GetMaterial(const std::string& name)
	{
		return m_Materials.at(name);
	}

	void AddModel(const std::string& name, const std::shared_ptr<Model>& model)
	{
		m_Models.emplace(name, model);
	}
	void AddShader(const std::string& name, const std::shared_ptr<Shader>& shader)
	{
		m_Shaders.emplace(name, shader);
	}

	void AddTexture(const std::string& name, const std::shared_ptr<Texture>& texture)
	{
		m_Textures.emplace(name, texture);
	}

	void AddMaterial(const std::string& name, const std::shared_ptr<Material>& material)
	{
		m_Materials.emplace(name, material);
	}

	std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
	std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
};
