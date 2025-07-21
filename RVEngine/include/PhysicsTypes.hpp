#pragma once
#include <cstdint>


enum class CollisionLayer : uint16_t
{
	Static = 0,
	Dynamic = 1,
	Player = 2,
	Enemy = 3,
	Bullet = 4,
	BulletEnemy = 5,
};

enum class MotionType : uint8_t
{
	Static,
	Kinematic,
	Dynamic,
};