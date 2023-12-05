#include <Scene.hpp>
#include <Renderer.hpp>
#include <Stencil.hpp>
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
			// bind albedo
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.model->GetMaterial()->albedo->GetTexture());

			// bind normal map
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mesh.model->GetMaterial()->normal->GetTexture());

			// bind orm map
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mesh.model->GetMaterial()->occlusionRoughnessMetallic->GetTexture());
			Stencil::DisableStencil();

			mesh.shader->Bind();

			for (int k = 0; k < 32; ++k)
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

	auto spriteView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
	for (auto entity : spriteView)
	{
		auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entity);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite.Tex->GetTexture());

		Stencil::DisableStencil();
		ResourceManager& rm = ResourceManager::instance();


		auto model = rm.GetModel("plane");
		auto shader = rm.GetShader("sprite");
		auto outlineShader = rm.GetShader("flat");

		shader->Bind();
		shader->SetVec4("u_Color", sprite.Color);
		shader->SetBool("u_Billboard", sprite.Billboard);

		Renderer::Submit(shader, model->GetMeshes()->at(0).m_VertexArray, transform.GetTransform(), (unsigned int)entity);

		outlineShader->Bind();
		outlineShader->SetVec4("u_Color", glm::vec4(1, 0.35, 0, 1));
		if((uint32_t)entity == m_SelectedEntity)
		{
			Stencil::EnableStencil();
			auto outlineTransform = transform.GetTransform();
			outlineTransform = glm::scale(outlineTransform, {1.1, 1.1, 1.1});

			Renderer::Submit(outlineShader, model->GetMeshes()->at(0).m_VertexArray, outlineTransform, (unsigned int)entity);
			Stencil::DefaultStencil();
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

void Scene::DuplicateEntity(Entity entity)
{
	auto newEntity = m_Registry.create();

	// create a copy of an entity component by component
	for(auto &&curr: m_Registry.storage()) {
		if(auto &storage = curr.second; storage.contains(entity.GetHandle())) {
			storage.push(newEntity, storage.value(entity.GetHandle()));
		}
	}

}
