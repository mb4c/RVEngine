#pragma once
#include <SceneCamera.hpp>

class EditorCamera : public SceneCamera
{
public:
	EditorCamera() = default;
	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	void SetViewMatrix(glm::mat4 view);

private:
	glm::mat4 m_ViewMatrix;
};