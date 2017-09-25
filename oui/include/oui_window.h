#pragma once

#include <string>
#include <oui.h>

namespace oui
{
	namespace window
	{
		unsigned dpi();
		inline float dpiFactor() { return dpi() / 96.0f; }

		struct Description
		{
			std::string title;
			int width = 640;
			int height = 480;
			int	sampleCount = 1;
		};
		Description initialize();

		void update(oui::Rectangle, oui::Input&);
	}
}