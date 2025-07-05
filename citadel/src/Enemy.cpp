#include "Enemy.hpp"

Enemy::Enemy(const Entity entity)
{
	m_Entity = entity;
}

void Enemy::Update(float dt)
{
	auto& ec = m_Entity.GetComponent<EnemyComponent>();

	if (ec.Health <= 0)
	{
		Die();
	}
}

int Enemy::GetRemainingEnemies()
{
	return m_Enemies.size();
}

void Enemy::TakeDamage(int damage)
{
	auto& ec = m_Entity.GetComponent<EnemyComponent>();
	ec.Health -= damage;
}

void Enemy::Die()
{
	m_Entity.Destroy();
}
