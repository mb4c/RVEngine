#include <Scene.hpp>
#include <../include/Renderer/Renderer.hpp>
#include <../include/Renderer/Stencil.hpp>
#include <Components.hpp>
#include <Entity.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer/MaterialUBO.hpp"

Scene::Scene()
{
    std::cout << "Scene created" << std::endl;
}

Scene::~Scene()
{
    std::cout << "Scene destroyed: " << m_SceneName << std::endl;
}

Entity Scene::CreateEntity(const std::string& name)
{
    RV_PROFILE_FUNCTION();
    return CreateEntityWithUUID(UUID(), name);
}

void Scene::OnStart()
{
    //	PrintAllComponentNames(AllComponents{});
    std::cout << "Scene Started" << std::endl;

    if (GetEntitiesWithComponent<SkyboxComponent>().empty())
    {
        std::cout << "No skybox found, critical error aaaaa" << std::endl;
    }

    auto& sc = GetEntitiesWithComponent<SkyboxComponent>().at(0).GetComponent<SkyboxComponent>();
    m_EnvironmentMap = EnvironmentMap(sc.envMap);
    m_EnvironmentMap.Capture();

    sc.envCubemap = m_EnvironmentMap.envCubemap;
    sc.irradianceMap = m_EnvironmentMap.irradianceMap;
    sc.prefilterMap = m_EnvironmentMap.prefilterMap;
    sc.brdfLUTTexture = m_EnvironmentMap.brdfLUTTexture;
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
            if (boxCollider.IndexSequence == static_cast<uint32_t>(BodyID::cInvalidBodyID) && !boxCollider.IsDestroyed)
            {
                auto pos = transform.GetPosition();
                auto size = boxCollider.Size * transform.Scale;
                entt::entity entityId = entity;
                auto rot = JPH::Quat::sEulerAngles(Vec3(transform.GetRotationRad().x, transform.GetRotationRad().y,
                                                        transform.GetRotationRad().z));
                auto body = m_PhysicsManager->CreateBox(Vec3(pos.x, pos.y, pos.z), Vec3(size.x, size.y, size.z), rot,
                                                        (uint32_t)entityId, nullptr, boxCollider.MotionType,
                                                        boxCollider.CollisionLayer, boxCollider.Mass,
                                                        boxCollider.Restitution, boxCollider.Friction);
                //			auto body = m_PhysicsManager->CreateBox((uint64_t)entityId, transform, boxCollider);
                boxCollider.IndexSequence = body->GetID().GetIndexAndSequenceNumber();
                std::cout << "Created BodyID: " << boxCollider.IndexSequence << std::endl;
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
                JPH::BodyLockRead lock(*lock_interface, body.first);
                if (lock.Succeeded()) // body_id may no longer be valid
                {
                    const JPH::Body& bodyobj = lock.GetBody();

                    auto entity = Entity(static_cast<entt::entity>(body.second), this);
                    if (entity.HasComponent<BoxColliderComponent>() || entity.HasComponent<SphereColliderComponent>())
                    {
                        auto& transform = entity.GetComponent<TransformComponent>();
                        auto pos = bodyobj.GetPosition();
                        transform.SetPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
                        auto rot = bodyobj.GetRotation();
                        transform.SetRotationRad(glm::eulerAngles(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ())));
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

    DestroyEntities();
}

void Scene::RenderScene()
{
    RV_PROFILE_SCOPE("Rendering");
	RV_PROFILE_GPU_ZONE("Render Scene");

    for (auto entityID : m_Registry.view<entt::entity>())
    {
        Entity entity{entityID, this};
        if (entity.GetComponent<RelationshipComponent>().parent == entt::null)
        {
            Entity::UpdateGlobalTransform(entity);
        }
    }

    auto lightGroup = m_Registry.group<>(entt::get<TransformComponent, LightComponent>);
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

        assert(mesh.model && "No model assigned to the model, what the fuck?!");

        for (int i = 0; i < mesh.model->GetMeshes()->size(); ++i)
        {
            RV_PROFILE_SCOPE("Render mesh");
            auto& currentMesh = mesh.model->GetMeshes()->at(i);
            std::shared_ptr<Material> mat = currentMesh.GetMaterial();

            assert(mat && "No material assigned to mesh");
            assert(mesh.shader && "No shader assigned to model");

            // bind albedo
            {
                RV_PROFILE_SCOPE("Bind textures");
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mat->albedo->GetTexture());

                // bind normal map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mat->normal->GetTexture());

                // bind orm map
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, mat->occlusion->GetTexture());

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, mat->roughness->GetTexture());

                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, mat->metallic->GetTexture());

                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, mat->emission->GetTexture());

                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_CUBE_MAP, irrMap);

                glActiveTexture(GL_TEXTURE7);
                glBindTexture(GL_TEXTURE_CUBE_MAP, prefMap);

                glActiveTexture(GL_TEXTURE8);
                glBindTexture(GL_TEXTURE_2D, brdfLUT);
            }

            Stencil::DisableStencil();

            mesh.shader->Bind();

            MaterialUBO uboData = {};
            uboData.albedoColor = mat->albedoColor;
            uboData.emissionColor = mat->emissionColor;
            uboData.parameters.x = mat->roughnessValue;
            uboData.parameters.y = mat->metallicValue;
            uboData.parameters.z = mat->emissionStrength;

            // pack flags into bits
            uint32_t flags = 0;
            {
                RV_PROFILE_SCOPE("Pack materialUBO flags");
                if (mat->useAlbedo)    flags |= 1 << 0;
                if (mat->useNormal)    flags |= 1 << 1;
                if (mat->useOcclusion) flags |= 1 << 2;
                if (mat->useRoughness) flags |= 1 << 3;
                if (mat->useMetallic)  flags |= 1 << 4;
                if (mat->useEmission)  flags |= 1 << 5;
                if (mat->useIBL)       flags |= 1 << 6;
            }

            uboData.parameters.w = *reinterpret_cast<float*>(&flags); // reinterpret flags as float

            {
                RV_PROFILE_SCOPE("Send materialUBO");
                glBindBuffer(GL_UNIFORM_BUFFER, Renderer::GetMaterialUBO());
                glBindBufferBase(GL_UNIFORM_BUFFER, 9, Renderer::GetMaterialUBO());
                glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uboData), &uboData);
            }

            {
                RV_PROFILE_SCOPE("Send light positions");

                constexpr int MAX_LIGHTS = 32;

                std::vector<glm::vec3> positions(MAX_LIGHTS, glm::vec3(0.0f));
                std::vector<glm::vec3> directions(MAX_LIGHTS, glm::vec3(0.0f));
                std::vector<glm::vec3> colors(MAX_LIGHTS, glm::vec3(0.0f));
                std::vector<int> lightTypes(MAX_LIGHTS, 0); // default to point

                int activeLights = std::min(static_cast<int>(lights.size()), MAX_LIGHTS);
                for (int i = 0; i < activeLights; ++i)
                {
                    positions[i] = std::get<0>(lights[i]).GetPosition();
                    colors[i] = std::get<1>(lights[i]).color * std::get<1>(lights[i]).intensity;
                    lightTypes[i] = static_cast<int>(std::get<1>(lights[i]).type);
                    directions[i] = std::get<0>(lights[i]).GetForward();
                }

                mesh.shader->SetVec3Array("lightPositions", positions);
                mesh.shader->SetVec3Array("lightDirections", directions);
                mesh.shader->SetVec3Array("lightColors", colors);
                mesh.shader->SetInt("numLights", activeLights);
                mesh.shader->SetInt3Array("lightTypes", lightTypes);
            }

            Renderer::Submit(mesh.shader, currentMesh.m_VertexArray, transform.GetTransform(), (unsigned int)entity);
            ResourceManager& rm = ResourceManager::instance();

            auto outlineShader = rm.GetShader("flat");
            outlineShader->Bind();
            outlineShader->SetVec4("u_Color", glm::vec4(1, 0.35, 0, 1));
            if ((uint32_t)entity == m_SelectedEntity)
            {
                Stencil::EnableStencil();
                auto outlineTransform = transform.GetTransform();
                outlineTransform = glm::scale(outlineTransform, {1.1, 1.1, 1.1});

                Renderer::Submit(outlineShader, currentMesh.m_VertexArray, outlineTransform, (unsigned int)entity);
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

        Renderer::Submit(shader, model->GetMeshes()->at(0).m_VertexArray, transform.GetTransform(),
                         (unsigned int)entity);

        outlineShader->Bind();
        outlineShader->SetVec4("u_Color", glm::vec4(1, 0.35, 0, 1));
        if ((uint32_t)entity == m_SelectedEntity)
        {
            Stencil::EnableStencil();
            auto outlineTransform = transform.GetTransform();
            outlineTransform = glm::scale(outlineTransform, {1.1, 1.1, 1.1});

            Renderer::Submit(outlineShader, model->GetMeshes()->at(0).m_VertexArray, outlineTransform,
                             (unsigned int)entity);
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
        Renderer::Submit(backgroundShader, model->GetMeshes()->at(0).m_VertexArray, transform.GetTransform(),
                         (unsigned int)entity);
    }

    if (!Renderer::GetDebugGeometry().Lines.empty())
    {
        const auto& lines = Renderer::GetDebugGeometry().Lines;

        struct RenderBatch
        {
            std::vector<float> vertices;
            float thickness;
            bool depthTest;
        };

        std::vector<RenderBatch> batches;

        for (const auto& line : lines)
        {
            auto it = std::find_if(batches.begin(), batches.end(),
                                   [&](const RenderBatch& batch)
                                   {
                                       return batch.thickness == line.Thickness && batch.depthTest == line.DepthTest;
                                   });

            if (it == batches.end())
            {
                batches.push_back({std::vector<float>(), line.Thickness, line.DepthTest});
                it = batches.end() - 1;
            }

            auto& verts = it->vertices;
            verts.insert(verts.end(), {
                             line.Start.x, line.Start.y, line.Start.z,
                             line.Color.r, line.Color.g, line.Color.b, line.Color.a,

                             line.End.x, line.End.y, line.End.z,
                             line.Color.r, line.Color.g, line.Color.b, line.Color.a
                         });
        }

        auto& rm = ResourceManager::instance();
        auto lineShader = rm.GetShader("DebugLine");
        lineShader->Bind();
        lineShader->SetMat4("u_ViewProjection", Renderer::GetViewProjection());
        lineShader->SetMat4("u_Transform", glm::mat4(1.0f));

        for (auto& batch : batches)
        {
            if (batch.vertices.empty()) continue;

            if (batch.depthTest)
            {
                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }

            auto vertexArray = std::make_shared<VertexArray>();
            auto vertexBuffer = std::make_shared<VertexBuffer>(
                batch.vertices.data(),
                batch.vertices.size() * sizeof(float)
            );

            BufferLayout layout = {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float4, "a_Color"}
            };
            vertexBuffer->SetLayout(layout);
            vertexArray->AddVertexBuffer(vertexBuffer);

            Renderer::SetLineWidth(batch.thickness);
            Renderer::DrawLines(vertexArray, static_cast<int>(batch.vertices.size() / 7));
        }

        glEnable(GL_DEPTH_TEST);

        Renderer::GetDebugGeometry().Clear();
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
        if (entity.HasComponent<BoxColliderComponent>())
        {
            auto& collider = entity.GetComponent<BoxColliderComponent>();

            m_PhysicsManager->RemoveBody(collider.IndexSequence);
            collider.IndexSequence = BodyID::cInvalidBodyID;
            collider.IsDestroyed = true;
        }
        m_Registry.destroy(entity.GetHandle());
        m_SelectedEntity = entt::null;
    }
    else
    {
        std::cout << "Entity does not exist" << std::endl;
    }
}

void Scene::DestroyEntities()
{
    for (auto entity : m_EntityDeletionQueue)
    {
        RemoveEntity(entity);
    }
    m_Registry.compact();
    m_EntityDeletionQueue.clear();
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
            entt::entity entityId = entity;

            auto rot = JPH::Quat::sEulerAngles(Vec3(transform.GetRotationRad().x, transform.GetRotationRad().y, transform.GetRotationRad().z));
            auto body = m_PhysicsManager->CreateSphere(Vec3(pos.x, pos.y, pos.z), radius, rot, (uint64_t)entityId,
                                                       &sphereCollider.userData, sphereCollider.Dynamic,
                                                       sphereCollider.Mass, sphereCollider.Restitution,
                                                       sphereCollider.Friction);
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
    Entity entity = {m_Registry.create(), this};
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
    if (!bodyID.IsInvalid())
    {
        m_PhysicsManager->GetBodyInterface()->SetPosition(bodyID, RVec3Arg(pos.x, pos.y, pos.z), EActivation::Activate);
    }
    else
    {
        std::cerr << "[SetPhysicsPosition] Entity has no valid collider BodyID! BodyID: " << bodyID. GetIndexAndSequenceNumber() << " Entity: " << (uint32_t)entity.GetHandle() << std::endl;
    }
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

    if (bodyID.IsInvalid())
    {
        std::cerr << "[SetGravityFactor] Entity has no valid collider BodyID! ID: " << bodyID. GetIndexAndSequenceNumber() << std::endl;
    }

    m_PhysicsManager->GetBodyInterface()->SetGravityFactor(bodyID, gravityFactor);
}

void Scene::SetVelocity(Entity entity, glm::vec3 velocity)
{
    BodyID bodyID;
    if (entity.HasComponent<BoxColliderComponent>())
        bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
    if (entity.HasComponent<SphereColliderComponent>())
        bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

    if (!bodyID.IsInvalid())
    {
        m_PhysicsManager->GetBodyInterface()->SetLinearVelocity(bodyID, RVec3Arg(velocity.x, velocity.y, velocity.z));
    }
    else
    {
        std::cerr << "[SetVelocity] Entity has no valid collider BodyID!\n";
    }
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
    return {vel.GetX(), vel.GetY(), vel.GetZ()};
}

glm::vec3 Scene::GetAngularVelocity(Entity entity)
{
    BodyID bodyID;
    if (entity.HasComponent<BoxColliderComponent>())
        bodyID = BodyID(entity.GetComponent<BoxColliderComponent>().IndexSequence);
    if (entity.HasComponent<SphereColliderComponent>())
        bodyID = BodyID(entity.GetComponent<SphereColliderComponent>().IndexSequence);

    Vec3 vel = m_PhysicsManager->GetBodyInterface()->GetAngularVelocity(bodyID);
    return {vel.GetX(), vel.GetY(), vel.GetZ()};
}

uint32_t Scene::GetEntityCount()
{
    return m_Registry.view<IDComponent>().size();
}

void Scene::RenderPicking()
{
    RV_PROFILE_SCOPE("Picking");
    auto group = m_Registry.group<TransformComponent>(entt::get<MeshRendererComponent>);
    for (auto entity : group)
    {
        auto [transform, mesh] = group.get<TransformComponent, MeshRendererComponent>(entity);

        assert(mesh.model && "No model assigned to the model, what the fuck?!");
        ResourceManager& rm = ResourceManager::instance();

        for (int i = 0; i < mesh.model->GetMeshes()->size(); ++i)
        {
            assert(mesh.model->GetMaterial() && "No material assigned to model ");
            assert(mesh.shader && "No shader assigned to model");

            auto picking = rm.GetShader("picking");
            picking->Bind();

            Renderer::Submit(picking, mesh.model->GetMeshes()->at(i).m_VertexArray, transform.GetTransform(), (unsigned int)entity);
        }
    }
}

//FIXME: this is so fucking wrong...
glm::vec3 Scene::ScreenToWorld(glm::vec2 screenPos, glm::vec3 origin, glm::vec3 direction)
{
    // Get viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Get the view and projection matrices
    auto view = Renderer::GetView();
    auto proj = Renderer::GetProjection();

    // Convert screen coordinates to normalized device coordinates
    glm::vec4 nearPoint;
    nearPoint.x = (2.0f * screenPos.x) / viewport[2] - 1.0f;
    nearPoint.y = 1.0f - (2.0f * screenPos.y) / viewport[3];
    nearPoint.z = -1.0f; // Near plane
    nearPoint.w = 1.0f;

    glm::vec4 farPoint;
    farPoint.x = nearPoint.x;
    farPoint.y = nearPoint.y;
    farPoint.z = 1.0f; // Far plane
    farPoint.w = 1.0f;

    // Create inverse view-projection matrix
    glm::mat4 invViewProj = glm::inverse(proj * view);

    // Transform from NDC to world space
    glm::vec4 nearWorld = invViewProj * nearPoint;
    glm::vec4 farWorld = invViewProj * farPoint;

    // Perspective division
    nearWorld /= nearWorld.w;
    farWorld /= farWorld.w;

    // Calculate ray direction
    glm::vec3 rayDirection = glm::normalize(glm::vec3(farWorld) - glm::vec3(nearWorld));

    // If no specific ray origin/direction are provided, use the calculated ones
    if (glm::length(origin) < 0.0001f && glm::length(direction) < 0.0001f)
    {
        origin = glm::vec3(nearWorld);
        direction = rayDirection;
    }

    glm::vec3 planeNormal = glm::normalize(direction);
    glm::vec3 planePoint = origin + glm::vec3{0, 0, 40};

    float denominator = glm::dot(planeNormal, rayDirection);

    // Check if ray is parallel to the plane (or nearly so)
    if (fabs(denominator) < 0.0001f)
    {
        return origin;
    }

    float t = glm::dot(planePoint - glm::vec3(nearWorld), planeNormal) / denominator;

    // Calculate the intersection point
    glm::vec3 intersectionPoint = glm::vec3(nearWorld) + rayDirection * t;

    return intersectionPoint;
}

template <typename... Component>
void Scene::CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src,
                          const std::unordered_map<UUID, entt::entity>& enttMap)
{
    CopyComponent<Component...>(dst, src, enttMap);
}

template <typename... Component>
void Scene::CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
{
    CopyComponentIfExists<Component...>(dst, src);
}

template <typename... Component>
void Scene::CopyComponentIfExists(Entity dst, Entity src)
{
    ([&]()
    {
        if (src.HasComponent<Component>())
            dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
    }(), ...);
}

template <typename... Component>
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
