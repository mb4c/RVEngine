#pragma once
#include <vector>

#include "Entity.hpp"

class Enemy
{
	public:
	Enemy(Entity entity);
	void Update(float dt);
	int GetRemainingEnemies();
	void TakeDamage(int damage);
	void Die();

private:
	std::vector<Entity> m_Enemies;
	Entity m_Entity;
};
