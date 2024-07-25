#include "PhysicsManager.hpp"

PhysicsManager::PhysicsManager()
{
	StartSimulation();

//	ShutdownSimulation();
}

void PhysicsManager::StartSimulation()
{
// Register allocation hook
	RegisterDefaultAllocator();

	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a factory
	Factory::sInstance = new Factory();

	// Register all Jolt physics types
	RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const uint cMaxContactConstraints = 1024;

//	// Create mapping table from object layer to broadphase layer
//	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
//	BPLayerInterfaceImpl broad_phase_layer_interface;
//
//	// Create class that filters object vs broadphase layers
//	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
//	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
//
//	// Create class that filters object vs object layers
//	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
//	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// Now we can create the actual physics system.
	m_PhysicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
//	MyBodyActivationListener body_activation_listener;
	m_PhysicsSystem.SetBodyActivationListener(&body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
//	MyContactListener contact_listener;
	m_PhysicsSystem.SetContactListener(&contact_listener);
	m_BodyInterface = &m_PhysicsSystem.GetBodyInterface();
//	m_PhysicsSystem.SetGravity(Vec3(0,-9.8f,0));


//	CreateBox(Vec3(0, -1, 0), Vec3(100, 1, 100));
//	CreateSphere(0.5f, Vec3(0,5,0))->GetID();
//	auto sphere_id = CreateSphere(0.5f, Vec3(0,100,0))->GetID();
//	m_BodyInterface->ActivateBody(sphere);


	// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
	// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
//	m_BodyInterface->SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));


	// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	m_PhysicsSystem.OptimizeBroadPhase();
}

void PhysicsManager::ShutdownSimulation()
{
	// Remove bodies from physic system
	for (auto& body : m_Bodies)
	{
		m_BodyInterface->RemoveBody(body);
	}

	// Destroy bodies
	for (auto& body : m_Bodies)
	{
		m_BodyInterface->DestroyBody(body);
	}

	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

Body* PhysicsManager::CreateBox(Vec3 position, Vec3 size, Quat rotation, uint64_t entity, BodyUserData* bud, bool dynamic, float mass, float restitution, float friction)
{
	BoxShapeSettings bodyShapeSettings(size);

	// Create the shape
	ShapeSettings::ShapeResult bodyShapeResult = bodyShapeSettings.Create();

	ShapeRefC bodyShape = bodyShapeResult.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
	// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	BodyCreationSettings bodySettings(bodyShape, position, rotation, dynamic ? EMotionType::Dynamic : EMotionType::Static, Layers::MOVING);
	bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bodySettings.mMassPropertiesOverride.mMass = mass;
	bodySettings.mRestitution = restitution;
	bodySettings.mFriction = friction;
	// Create the actual rigid body
	Body* body = m_BodyInterface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
	bud->entityID = entity;
	void* ptr = bud;
	body->SetUserData(reinterpret_cast<uintptr_t>(ptr));

	// Add it to the world
	m_BodyInterface->AddBody(body->GetID(), EActivation::Activate);
	m_Bodies.push_back(body->GetID());
	return body;
}
//
//Body* PhysicsManager::CreateBox(uint64_t entity, TransformComponent tc, BoxColliderComponent bcc)
//{
//	auto shapeSize = bcc.Size * tc.Scale;
//	auto pos = tc.GetPosition();
//	Vec3 position(pos.x,pos.y,pos.z);
////	entt::entity entityId = entity;
//	auto rotation = JPH::Quat::sEulerAngles(Vec3(tc.GetRotationRad().x,tc.GetRotationRad().y,tc.GetRotationRad().z));
//
//	BoxShapeSettings bodyShapeSettings(Vec3(shapeSize.x,shapeSize.y,shapeSize.z));
//
//	// Create the shape
//	ShapeSettings::ShapeResult bodyShapeResult = bodyShapeSettings.Create();
//
//	ShapeRefC bodyShape = bodyShapeResult.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
//	// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
//
//	BodyCreationSettings bodySettings(bodyShape, position, rotation, bcc.Dynamic ? EMotionType::Dynamic : EMotionType::Static, Layers::MOVING);
//	bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
//	bodySettings.mMassPropertiesOverride.mMass = bcc.Mass;
//	bodySettings.mRestitution = bcc.Restitution;
//	bodySettings.mFriction = bcc.Friction;
//	// Create the actual rigid body
//	Body* body = m_BodyInterface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
//	body->SetUserData(entity);
//
//	// Add it to the world
//	m_BodyInterface->AddBody(body->GetID(), EActivation::Activate);
//	m_Bodies.push_back(body->GetID());
//	return body;
//}

Body* PhysicsManager::CreateSphere(Vec3 position, float radius, Quat rotation, uint64_t entity, BodyUserData* bud, bool dynamic, float mass, float restitution, float friction)
{
	BodyCreationSettings bodySettings(new SphereShape(radius), position, rotation, dynamic ? EMotionType::Dynamic : EMotionType::Static, Layers::MOVING);
	bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bodySettings.mMassPropertiesOverride.mMass = mass;
	bodySettings.mRestitution = restitution;
	bodySettings.mFriction = friction;

	Body* body = m_BodyInterface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
	bud->entityID = entity;
	void* ptr = bud;
	body->SetUserData(reinterpret_cast<uintptr_t>(ptr));
	// Add it to the world
	m_BodyInterface->AddBody(body->GetID(), EActivation::Activate);
	m_Bodies.push_back(body->GetID());
	return body;
}

void PhysicsManager::OnUpdate(float dt)
{
//	std::cout << m_Bodies.size() << std::endl;
	for (auto& body : m_Bodies)
	{
		if(m_BodyInterface->IsActive(body))
		{
			// Next step
			++m_SimulationStep;
			// Output current position and velocity of the sphere
//			RVec3 position = m_BodyInterface->GetCenterOfMassPosition(body);
//			Vec3 velocity = m_BodyInterface->GetLinearVelocity(body);
//			std::cout << GetMass(body) << std::endl;
//			cout << "Step " << m_SimulationStep << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << endl;
		}
	}
	// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
	// Step the world
	const int cCollisionSteps = 1;
	m_PhysicsSystem.Update(dt, cCollisionSteps, temp_allocator, job_system);
}

void PhysicsManager::SetMass(BodyID body_id, float mass)
{
	lock_interface = &m_PhysicsSystem.GetBodyLockInterface(); // Or GetBodyLockInterfaceNoLock

// Scoped lock
	{
		JPH::BodyLockRead lock(*lock_interface, body_id);
		if (lock.Succeeded()) // body_id may no longer be valid
		{
			const JPH::Body &body = lock.GetBody();
			if (!body.IsStatic())
			{
				auto* pMotionProperties = const_cast<MotionProperties*>(body.GetMotionProperties());
				JPH::MassProperties massProperties = body.GetShape()->GetMassProperties();
				massProperties.ScaleToMass( mass );
				massProperties.mInertia( 3, 3 ) = 1.0f;
				pMotionProperties->SetMassProperties( JPH::EAllowedDOFs::All, massProperties );
//			pMotionProperties->SetInverseMass( 1/ mass );
				std::cout <<"Mass set: " << pMotionProperties->GetInverseMass() << "kg" << std::endl;
			}

		}
	}
}

float PhysicsManager::GetMass(BodyID body_id)
{
	lock_interface = &m_PhysicsSystem.GetBodyLockInterface(); // Or GetBodyLockInterfaceNoLock

// Scoped lock
	{
		JPH::BodyLockRead lock(*lock_interface, body_id);
		if (lock.Succeeded()) // body_id may no longer be valid
		{
			const JPH::Body &body = lock.GetBody();

			return 1 / body.GetMotionProperties()->GetInverseMass();
		}
	}
	return -1; // Return -1 if failed to get mass
}
