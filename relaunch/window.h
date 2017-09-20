#pragma once

namespace window
{
	struct Description
	{
		int width = 640;
		int height = 480;
	};
	Description initialize();

	void update();
}