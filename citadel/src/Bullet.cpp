#include "Bullet.hpp"


Bullet::Bullet(Entity entity, glm::vec3 pos, glm::vec3 velocity, int damage, float lifeTime, bool friendly)
{
	m_Entity = entity;
	m_Velocity = velocity;
	m_Damage = damage;
	m_LifeTime = lifeTime;
	m_RemainingLifeTime = m_LifeTime;
	m_Friendly = friendly;
	entity.GetComponent<TransformComponent>().SetPosition(pos);
}

void Bullet::OnUpdate(const float dt)
{
	m_RemainingLifeTime -= dt;

	m_Entity.GetComponent<TransformComponent>().Translate(m_Velocity * dt);

}

void Bullet::Destroy()
{
	m_Entity.Destroy();
}
bool Bullet::ShouldDie() const
{
	if (m_RemainingLifeTime <= 0)
		return true;
	return false;
}

float Bullet::GetLifeTime() const
{
	return m_LifeTime;
}

float Bullet::GetRemainingLifeTime() const
{
	return m_RemainingLifeTime;
}
