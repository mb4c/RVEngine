#include <Entity.hpp>

Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
{
}

void Entity::SetParent(Entity entity)
{
	auto& rc = GetComponent<RelationshipComponent>();
	auto& prc = entity.GetComponent<RelationshipComponent>();

	// If already has a parent, adjust siblings
	if (rc.parent != entt::null)
	{
		auto& oldParentRC = m_Scene->m_Registry.get<RelationshipComponent>(rc.parent);
		if (oldParentRC.first == m_EntityHandle)
		{
			oldParentRC.first = rc.next;
		}
		if (rc.prev != entt::null)
		{
			auto& prevRC = m_Scene->m_Registry.get<RelationshipComponent>(rc.prev);
			prevRC.next = rc.next;
		}
		if (rc.next != entt::null)
		{
			auto& nextRC = m_Scene->m_Registry.get<RelationshipComponent>(rc.next);
			nextRC.prev = rc.prev;
		}
	}

	rc.parent = entity.m_EntityHandle;
	rc.prev = entt::null;
	rc.next = prc.first;

	if (prc.first != entt::null)
	{
		auto& firstRC = m_Scene->m_Registry.get<RelationshipComponent>(prc.first);
		firstRC.prev = m_EntityHandle;
	}

	prc.first = m_EntityHandle;
}

Entity Entity::GetParent()
{
	auto& rel = GetComponent<RelationshipComponent>();
	return {rel.parent, m_Scene};
}

Entity Entity::Find(std::string name)
{
	//TODO:
	return Entity();
}

Entity Entity::GetChild(uint32_t index) {
	std::vector<Entity> children = GetChildren();
	assert(index < children.size() && "Index out of bounds");

	if (index < children.size()) {
		return children[index];
	}

	return {};
}
std::vector<Entity> Entity::GetChildren()
{
	std::vector<Entity> children;

	auto &comp = m_Scene->m_Registry.get<RelationshipComponent>(m_EntityHandle);
	auto curr = comp.first;

	while(curr != entt::null) {
		children.push_back(Entity(curr,m_Scene));
		curr = m_Scene->m_Registry.get<RelationshipComponent>(curr).next;
	}

	return children;
}

bool Entity::IsChildOf(Entity entity)
{
	auto& rel = GetComponent<RelationshipComponent>();
	return rel.parent == entity.m_EntityHandle;
}
bool Entity::HasParent()
{
	auto& rel = GetComponent<RelationshipComponent>();
	return rel.parent != entt::null;
}

void Entity::UpdateGlobalTransform(Entity entity)
{
	auto& transform = entity.GetComponent<TransformComponent>();

//	if (transform.IsDirty)
//	{
		glm::mat4 localTransform = transform.GetLocalTransform();
		if (entity.HasParent())
		{
			auto& parentTransform = entity.GetParent().GetComponent<TransformComponent>();
			transform.GlobalTransform = parentTransform.GlobalTransform * localTransform;
		} else
		{
			transform.GlobalTransform = localTransform;
		}
		transform.IsDirty = false;
//		std::cout << "Updated transform" << std::endl;
		// Update the children recursively
		for (auto& child: entity.GetChildren())
		{
			UpdateGlobalTransform(child);
		}
//	}
}

bool Entity::IsColliding()
{
	bool isColliding = false;
	if(HasComponent<SphereColliderComponent>())
	{
		auto& sc = GetComponent<SphereColliderComponent>();
		isColliding = sc.userData.isColliding;
		sc.userData.isColliding = false;
	}
	if(HasComponent<BoxColliderComponent>())
	{
		auto& bc = GetComponent<BoxColliderComponent>();
		isColliding = bc.userData.isColliding;
		bc.userData.isColliding = false;
	}

	return isColliding;
}

Entity Entity::OnContactAdded()
{
	Entity other;
	if (IsColliding() && HasComponent<SphereColliderComponent>())
	{
		other = Entity(static_cast<entt::entity>(GetComponent<SphereColliderComponent>().userData.otherID), m_Scene);
		return other;
	}
	else if (IsColliding() && HasComponent<BoxColliderComponent>())
	{
		other = Entity(static_cast<entt::entity>(GetComponent<BoxColliderComponent>().userData.otherID), m_Scene);
		return other;
	}

	return Entity();
}

Entity Entity::Instantiate()
{
	auto entity = m_Scene->DuplicateEntity(*this);
	if (entity.HasComponent<BoxColliderComponent>())
	{
		auto& bc = entity.GetComponent<BoxColliderComponent>();
		BodyUserData bud;
		bc.userData = bud;
		bc.IndexSequence = 0;

	}
	return entity;
}
