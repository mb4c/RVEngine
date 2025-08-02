#pragma once
#include <cstdint>

namespace RV
{

enum class ECollisionLayer : uint16_t
{
	Static = 0,
	Dynamic = 1,
	Player = 2,
	Enemy = 3,
	Bullet = 4,
	BulletEnemy = 5,
};

enum class EMotionType : uint8_t
{
	Static,
	Kinematic,
	Dynamic,
};
}
