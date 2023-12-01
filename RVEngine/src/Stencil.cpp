#include <Stencil.hpp>

void Stencil::EnableStencil()
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
//	glDisable(GL_DEPTH_TEST);
}

void Stencil::DisableStencil()
{
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
}

void Stencil::DefaultStencil()
{
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
//	glEnable(GL_DEPTH_TEST);
}
