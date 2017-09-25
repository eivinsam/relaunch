#include "oui.h"

#include <GL/glew.h>

namespace oui
{
	void fill(const Rectangle& area, const Color& color)
	{
		glColor4f(color.r, color.g, color.b, color.a);
		glBegin(GL_QUADS);
		glVertex2f(area.min.x, area.min.y);
		glVertex2f(area.min.x, area.max.y);
		glVertex2f(area.max.x, area.max.y);
		glVertex2f(area.max.x, area.min.y);
		glEnd();
	}
}
