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
	return CreateEntityWithUUID(UUID(), name);
}

void Scene::OnStart()
{
	PrintAllComponentNames(AllComponents{});
}

void Scene::OnUpdateEditor(float ts, EditorCamera& editorCamera)
{
	RenderScene();
}

void Scene::OnUpdateRuntime(float ts)
{
	m_PhysicsManager->OnUpdate(ts);

	{
		auto view = m_Registry.view<TransformComponent, BoxColliderComponent>();
		for (auto entity : view)
		{
			auto [transform, boxCollider] = view.get<TransformComponent, BoxColliderComponent>(entity);

			for (auto body : m_PhysicsManager->GetBodies())
			{
				const JPH::BodyLockInterface* lock_interface;
				lock_interface = &m_PhysicsManager->GetPhysicsSystem().GetBodyLockInterface(); // Or GetBodyLockInterfaceNoLock

// Scoped lock
				{
					JPH::BodyLockRead lock(*lock_interface, body);
					if (lock.Succeeded()) // body_id may no longer be valid
					{
						const JPH::Body &bodyobj = lock.GetBody();
						auto entityId = static_cast<entt::entity>(bodyobj.GetUserData());
						if (entityId == entity)
						{
							auto pos = bodyobj.GetPosition();
							transform.SetPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
							auto rot = bodyobj.GetRotation();
							transform.SetRotationRad(glm::eulerAngles(glm::quat(rot.GetW(),rot.GetX(),rot.GetY(), rot.GetZ())));

						}

					}
				}

			}

		}
	}



	Camera* mainCamera = nullptr;
	glm::vec3 cameraPosition;
	glm::mat4 cameraTransform;
	{
		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view)
		{
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

			if (camera.Primary)
			{
				mainCamera = &camera.Camera;
				auto scenecam = (SceneCamera*)mainCamera;
				scenecam->SetViewportSize(1280,720); // TODO: scene should have viewport size
				cameraTransform = transform.GetTransform();
				cameraPosition = transform.GetPosition();

				break;
			}
		}
	}

	if (mainCamera)
	{
		Renderer::BeginScene(*mainCamera, cameraTransform);
		ResourceManager& rm = ResourceManager::instance();
		auto shader = rm.GetShader("pbr");
		shader->Bind();
		shader->SetVec3("u_CamPos", cameraPosition);
		shader->SetUInt("u_DisplayType", 0);

		RenderScene();
	}

}

void Scene::RenderScene()
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
	Entity newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
	CopyComponentIfExists(AllComponents{}, newEntity, entity);
}

void Scene::OnRuntimeStart()
{
	m_IsRunning = true;
	m_PhysicsManager = new PhysicsManager();

	{
		auto view = m_Registry.view<TransformComponent, BoxColliderComponent>();
		for (auto entity : view)
		{
			auto [transform, boxCollider] = view.get<TransformComponent, BoxColliderComponent>(entity);
			auto pos = transform.GetPosition();
			auto scale = boxCollider.Size;
			entt::entity entityId = entity;
			auto rot = JPH::Quat::sEulerAngles(Vec3(transform.GetRotationRad().x,transform.GetRotationRad().y,transform.GetRotationRad().z));
//			auto rot = transform.GetRotationQuat();
			auto id = m_PhysicsManager->CreateBox(Vec3(pos.x, pos.y, pos.z), Vec3(scale.x, scale.y, scale.z), rot, (uint64_t)entityId, boxCollider.Dynamic, boxCollider.Mass, boxCollider.Restitution, boxCollider.Friction);
//			auto id = m_PhysicsManager->CreateBox(Vec3(pos.x, pos.y, pos.z), Vec3(scale.x, scale.y, scale.z), Quat(rot.x,rot.y,rot.z,rot.w), (uint64_t)entityId, boxCollider.Dynamic, boxCollider.Mass);
//			m_PhysicsManager->SetMass(id->GetID(), boxCollider.Mass);
		}
	}
}

void Scene::OnRuntimeStop()
{
	m_IsRunning = false;
	delete m_PhysicsManager;
}

std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> other)
{
	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

//		newScene->m_ViewportWidth = other->m_ViewportWidth;
//		newScene->m_ViewportHeight = other->m_ViewportHeight;

	auto& srcSceneRegistry = other->m_Registry;
	auto& dstSceneRegistry = newScene->m_Registry;
	std::unordered_map<UUID, entt::entity> enttMap;

	// Create entities in new scene
	auto idView = srcSceneRegistry.view<IDComponent>();
	for (auto e : idView)
	{
		UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
		const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
		entt::entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
		enttMap[uuid] = (entt::entity)newEntity;
	}

	// Copy components (except IDComponent and TagComponent)
	CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

	return newScene;
}

Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
{
	Entity entity = { m_Registry.create(), this };
	entity.AddComponent<IDComponent>(uuid);
	entity.AddComponent<TransformComponent>();
	auto& tag = entity.AddComponent<TagComponent>();
	tag.Tag = name.empty() ? "Entity" : name;

	m_EntityMap[uuid] = entity;

	return entity;
}

template<typename... Component>
void Scene::CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src,
						  const std::unordered_map<UUID, entt::entity>& enttMap)
{
	CopyComponent<Component...>(dst, src, enttMap);
}

template<typename... Component>
void Scene::CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
{
	CopyComponentIfExists<Component...>(dst, src);
}

template<typename... Component>
void Scene::CopyComponentIfExists(Entity dst, Entity src)
{
	([&]()
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}(), ...);
}

template<typename... Component>
void
Scene::CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
{
	([&]()
	{
		auto view = src.view<Component>();
		for (auto srcEntity : view)
		{
			entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = src.get<Component>(srcEntity);
			dst.emplace_or_replace<Component>(dstEntity, srcComponent);
		}
	}(), ...);
}
