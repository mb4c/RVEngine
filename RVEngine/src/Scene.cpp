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
//	PrintAllComponentNames(AllComponents{});
}

void Scene::OnUpdateEditor(float ts, EditorCamera& editorCamera)
{
	RV_PROFILE_FUNCTION();
	RenderScene();
}

void Scene::OnUpdateRuntime(float ts)
{
	RV_PROFILE_FUNCTION();
	m_PhysicsManager->OnUpdate(ts);

	{
		auto view = m_Registry.view<TransformComponent, BoxColliderComponent>();
		for (auto entity : view)
		{
			auto [transform, boxCollider] = view.get<TransformComponent, BoxColliderComponent>(entity);
			if (boxCollider.IndexSequence == 0)
			{
				auto pos = transform.GetPosition();
				auto size = boxCollider.Size * transform.Scale;
				entt::entity entityId = entity;
				auto rot = JPH::Quat::sEulerAngles(Vec3(transform.GetRotationRad().x,transform.GetRotationRad().y,transform.GetRotationRad().z));
				auto body = m_PhysicsManager->CreateBox(Vec3(pos.x, pos.y, pos.z), Vec3(size.x, size.y, size.z), rot, (uint64_t)entityId, &boxCollider.userData, boxCollider.Dynamic, boxCollider.Mass, boxCollider.Restitution, boxCollider.Friction);
//			auto body = m_PhysicsManager->CreateBox((uint64_t)entityId, transform, boxCollider);
				boxCollider.IndexSequence = body->GetID().GetIndexAndSequenceNumber();
			}
		}
	}


	{
		RV_PROFILE_SCOPE("UpdatePhysicsTransform");

		for (auto body : m_PhysicsManager->GetBodies())
		{
			const JPH::BodyLockInterface* lock_interface;
			lock_interface = &m_PhysicsManager->GetPhysicsSystem().GetBodyLockInterface();
			// Scoped lock
			{
				JPH::BodyLockRead lock(*lock_interface, body);
				if (lock.Succeeded()) // body_id may no longer be valid
				{
					const JPH::Body &bodyobj = lock.GetBody();
					BodyUserData* bud = reinterpret_cast<BodyUserData*>(bodyobj.GetUserData());

					auto entity = Entity(static_cast<entt::entity>(bud->entityID), this);
					if (entity.HasComponent<BoxColliderComponent>() || entity.HasComponent<SphereColliderComponent>())
					{
						auto& transform = entity.GetComponent<TransformComponent>();
						auto pos = bodyobj.GetPosition();
						transform.SetPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
						auto rot = bodyobj.GetRotation();
						transform.SetRotationRad(glm::eulerAngles(glm::quat(rot.GetW(),rot.GetX(),rot.GetY(), rot.GetZ())));

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
				scenecam->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
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

		RenderScene();
		Renderer::EndScene();
	}

}

void Scene::RenderScene()
{
	RV_PROFILE_FUNCTION();
	m_Registry.each([this](auto entityID)
							   {
								   Entity entity{entityID, this};
								   if (entity.GetComponent<RelationshipComponent>().parent == entt::null)
								   {
									   Entity::UpdateGlobalTransform(entity);
								   }
							   });

	auto lightGroup = m_Registry.group<>(entt::get<TransformComponent ,LightComponent>);
	unsigned int irrMap;
	unsigned int prefMap;
	unsigned int brdfLUT;

	auto skyView = m_Registry.view<TransformComponent, SkyboxComponent>();
	for (auto entity : skyView)
	{
		auto [transform, sky] = skyView.get<TransformComponent, SkyboxComponent>(entity);
		irrMap = sky.irradianceMap;
		prefMap = sky.prefilterMap;
		brdfLUT = sky.brdfLUTTexture;
	}

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



			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irrMap);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefMap);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, brdfLUT);

			Stencil::DisableStencil();

			mesh.shader->Bind();
			std::shared_ptr<Material> mat = mesh.model->GetMaterial();
			mesh.shader->SetBool("u_UseAlbedo", mat->useAlbedo);
			mesh.shader->SetBool("u_UseNormal", mat->useNormal);
			mesh.shader->SetBool("u_UseORM", mat->useORM);
			mesh.shader->SetVec4("u_AlbedoColor", mat->albedoColor);
			mesh.shader->SetFloat("u_RoughnessVal", mat->roughnessValue);
			mesh.shader->SetFloat("u_MetallicVal", mat->metallicValue);

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
			ResourceManager& rm = ResourceManager::instance();

			auto outlineShader = rm.GetShader("flat");
			outlineShader->Bind();
			outlineShader->SetVec4("u_Color", glm::vec4(1, 0.35, 0, 1));
			if((uint32_t)entity == m_SelectedEntity)
			{
				Stencil::EnableStencil();
				auto outlineTransform = transform.GetTransform();
				outlineTransform = glm::scale(outlineTransform, {1.1, 1.1, 1.1});

				Renderer::Submit(outlineShader, mesh.model->GetMeshes()->at(i).m_VertexArray, outlineTransform, (unsigned int)entity);
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

	auto skyboxView = m_Registry.view<TransformComponent, SkyboxComponent>();
	for (auto entity : skyboxView)
	{
		auto [transform, skybox] = skyboxView.get<TransformComponent, SkyboxComponent>(entity);
		// render skybox (render as last to prevent overdraw)
		ResourceManager& rm = ResourceManager::instance();
		auto model = rm.GetModel("cube");
		auto backgroundShader = rm.GetShader("skybox");
		backgroundShader->Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.envCubemap);
		Renderer::Submit(backgroundShader, model->GetMeshes()->at(0).m_VertexArray, transform.GetTransform(), (unsigned int)entity);
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

Entity Scene::DuplicateEntity(Entity entity)
{
	Entity newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
	CopyComponentIfExists(AllComponents{}, newEntity, entity);
	return newEntity;
}

void Scene::OnRuntimeStart()
{
	m_IsRunning = true;
	m_PhysicsManager = new PhysicsManager();


	{
		auto view = m_Registry.view<TransformComponent, SphereColliderComponent>();
		for (auto entity : view)
		{
			auto [transform, sphereCollider] = view.get<TransformComponent, SphereColliderComponent>(entity);
			auto pos = transform.GetPosition();

			float radius = sphereCollider.Radius * glm::compMax(transform.Scale);
//			std::cout << "radius " << radius << std::endl;
			entt::entity entityId = entity;

			auto rot = JPH::Quat::sEulerAngles(Vec3(transform.GetRotationRad().x,transform.GetRotationRad().y,transform.GetRotationRad().z));
			auto body = m_PhysicsManager->CreateSphere(Vec3(pos.x, pos.y, pos.z), radius, rot, (uint64_t)entityId, &sphereCollider.userData, sphereCollider.Dynamic, sphereCollider.Mass, sphereCollider.Restitution, sphereCollider.Friction);
			sphereCollider.IndexSequence = body->GetID().GetIndexAndSequenceNumber();

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
	entity.AddComponent<RelationshipComponent>();
	auto& tag = entity.AddComponent<TagComponent>();
	tag.Tag = name.empty() ? "Entity" : name;

	m_EntityMap[uuid] = entity;

	return entity;
}

void Scene::SetViewportSize(uint32_t width, uint32_t height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;
}

void Scene::SetPhysicsPosition(Entity entity, glm::vec3 pos)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	m_PhysicsManager->GetBodyInterface()->SetPosition(bodyID, RVec3Arg(pos.x, pos.y, pos.z), EActivation::Activate);
}

glm::vec3 Scene::GetPhysicsPosition(Entity entity)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	auto pos = m_PhysicsManager->GetBodyInterface()->GetPosition(bodyID);
	return {pos.GetX(), pos.GetY(), pos.GetZ()};
}

void Scene::SetGravityFactor(Entity entity, float gravityFactor)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	m_PhysicsManager->GetBodyInterface()->SetGravityFactor(bodyID, gravityFactor);
}

void Scene::SetVelocity(Entity entity, glm::vec3 velocity)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	m_PhysicsManager->GetBodyInterface()->SetLinearVelocity(bodyID, RVec3Arg(velocity.x, velocity.y, velocity.z));
}

void Scene::AddVelocity(Entity entity, glm::vec3 velocity)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	m_PhysicsManager->GetBodyInterface()->AddLinearVelocity(bodyID, RVec3Arg(velocity.x, velocity.y, velocity.z));
}

void Scene::SetAngularVelocity(Entity entity, glm::vec3 velocity)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	m_PhysicsManager->GetBodyInterface()->SetAngularVelocity(bodyID, RVec3Arg(velocity.x, velocity.y, velocity.z));
}

glm::vec3 Scene::GetVelocity(Entity entity)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	Vec3 vel = m_PhysicsManager->GetBodyInterface()->GetLinearVelocity(bodyID);
	return {vel.GetX(),vel.GetY(),vel.GetZ()};
}

glm::vec3 Scene::GetAngularVelocity(Entity entity)
{
	BodyID bodyID;
	if (entity.HasComponent<BoxColliderComponent>())
		bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
	if (entity.HasComponent<SphereColliderComponent>())
		bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

	Vec3 vel = m_PhysicsManager->GetBodyInterface()->GetAngularVelocity(bodyID);
	return {vel.GetX(),vel.GetY(),vel.GetZ()};
}

uint32_t Scene::GetEntityCount()
{
	return m_Registry.size();
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
