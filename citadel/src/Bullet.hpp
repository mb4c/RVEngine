#pragma once
#include <glm/vec3.hpp>
#include "Entity.hpp"

class Bullet
{
	public:
	Bullet(Entity entity, glm::vec3 pos, glm::vec3 velocity, int damage, float lifeTime, bool friendly);
	void OnUpdate(float dt);
	void Destroy();
	bool ShouldDie() const;
	float GetLifeTime() const;
	float GetRemainingLifeTime() const;

	Entity m_Entity;
	glm::vec3 m_Velocity;
	int m_Damage;
	float m_LifeTime = 2.0f;
	float m_RemainingLifeTime = 2.0f;
	bool m_Friendly = false;
};



