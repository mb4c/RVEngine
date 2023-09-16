#pragma once
#include <glm/glm.hpp>

namespace Math
{
	class Mat4
	{
	public:
		static void DecomposeTransform(const glm::mat4& mat, glm::vec3& pos, glm::quat& rot, glm::vec3& scale);
	};

}