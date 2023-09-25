#include "Scene.hpp"
#include "Renderer.hpp"
#include "Stencil.hpp"
#include <Components.hpp>
#include <Entity.hpp>

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
