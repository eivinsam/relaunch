#pragma once

#include "oui.h"

namespace window
{
	struct Description
	{
		int width = 640;
		int height = 480;
	};
	Description initialize();

	void update(oui::Rectangle, oui::Input&);
}