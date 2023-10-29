#include "Scene.hpp"
#include "Renderer.hpp"
#include "Stencil.hpp"
#include <Components.hpp>
#include <Entity.hpp>
#include <glm/gtc/type_ptr.hpp>

Scene::Scene()
{

}

Scene::~Scene()
{
}

Entity Scene::CreateEntity(const std::string& name)
{
	RV_PROFILE_FUNCTION();
	Entity entity = { m_Registry.create(), this };
	entity.AddComponent<TransformComponent>();
	auto& tag = entity.AddComponent<TagComponent>();
	tag.Tag = name.empty() ? "Entity" : name;
	return entity;
}

void Scene::OnUpdate(float ts)
{
	RV_PROFILE_FUNCTION();

	auto lightGroup = m_Registry.group<>(entt::get<TransformComponent ,LightComponent>);

	std::vector<std::tuple<TransformComponent, LightComponent>> lights;
	for (auto entity : lightGroup)
	{
		lights.emplace_back(lightGroup.get<TransformComponent>(entity), lightGroup.get<LightComponent>(entity));
	}

	auto group = m_Registry.group<TransformComponent>(entt::get<MeshRendererComponent>);
	for (auto entity : group)
	{
		auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);

		for (int i = 0; i < mesh.model->GetMeshes()->size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.model->GetMaterial()->albedo.id);
			// bind orm map
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mesh.model->GetMaterial()->occlusionRoughnessMetallic.id);

			Stencil::DisableStencil();

			mesh.shader->Bind();

			for (int k = 0; k < 32; ++k) //HACK: i want to die when i see this
			{
				glm::vec3 zero {0,0,0};
				mesh.shader->SetVec3("lightPositions[" + std::to_string(k) + "]", zero);
				mesh.shader->SetVec3("lightColors[" + std::to_string(k) + "]", zero);
			}

			for (int j = 0; j < lights.size(); ++j)
			{
				mesh.shader->SetVec3("lightPositions[" + std::to_string(j) + "]", std::get<0>(lights.at(j)).GetPosition());
				mesh.shader->SetVec3("lightColors[" + std::to_string(j) + "]", std::get<1>(lights.at(j)).color * std::get<1>(lights.at(j)).intensity);
			}


			Renderer::Submit(mesh.shader, mesh.model->GetMeshes()->at(i).m_VertexArray, transform.GetTransform(), (unsigned int)entity);
			mesh.outlineShader->Bind();
			mesh.outlineShader->SetVec4("u_Color", glm::vec4(1, 0.35, 0, 1));
			if((uint32_t)entity == m_SelectedEntity)
			{
				Stencil::EnableStencil();
				auto outlineTransform = transform.GetTransform();
				outlineTransform = glm::scale(outlineTransform, {1.1, 1.1, 1.1});

				Renderer::Submit(mesh.outlineShader, mesh.model->GetMeshes()->at(i).m_VertexArray, outlineTransform, (unsigned int)entity);
				Stencil::DefaultStencil();
			}
		}

	}



}

void Scene::SetSelectedEntity(uint32_t entity)
{
	m_SelectedEntity = entity;
}

void Scene::RemoveEntity(Entity entity)
{
	if (m_Registry.valid(entity.GetHandle()))
	{
		m_Registry.destroy(entity.GetHandle());
		m_SelectedEntity = entt::null;
	} else
	{
		std::cout << "Entity does not exist" << std::endl;
	}
}
