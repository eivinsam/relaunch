#include "oui.h"

#include <GL/glew.h>

namespace oui
{
	void fill(const Rectangle& area, const Color& color)
	{
		glColor4f(color.r, color.g, color.b, color.a);
		glBegin(GL_QUADS);
		glVertex2f(area.upperLeft.x, area.upperLeft.y);
		glVertex2f(area.upperLeft.x, area.lowerRight.y);
		glVertex2f(area.lowerRight.x, area.lowerRight.y);
		glVertex2f(area.lowerRight.x, area.upperLeft.y);
		glEnd();
	}
}
