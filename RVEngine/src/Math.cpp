#include <glm/gtc/quaternion.hpp>
#include "Math.hpp"

void Math::Mat4::DecomposeTransform(const glm::mat4 &mat, glm::vec3 &pos, glm::quat &rot, glm::vec3 &scale)
{
	pos = mat[3];
	for(int i = 0; i < 3; i++)
		scale[i] = glm::length(glm::vec3(mat[i]));
	const glm::mat3 rotMtx(
			glm::vec3(mat[0]) / scale[0],
			glm::vec3(mat[1]) / scale[1],
			glm::vec3(mat[2]) / scale[2]);
	rot = glm::quat_cast(rotMtx);
}
