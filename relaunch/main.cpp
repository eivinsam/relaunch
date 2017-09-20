#include "window.h"
#include "debug.h"
#include <iostream>
#include <optional>
#include <chrono>
#include <cmath>

#include "text.h"


#include <GL/glew.h>
namespace window
{

	Description initialize()
	{
		return { 640, 480 };
	}


	void update()
	{
		static oui::Font font("Segoe UI", 24);
		static std::optional<decltype(std::chrono::high_resolution_clock::now())> last_frame_end;
		static const auto first_time = std::chrono::high_resolution_clock::now();
		auto frame_begin = std::chrono::high_resolution_clock::now();
		static float max_cpu = 0;


		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		oui::Rectangle area =
		{
			{ float(viewport[0]), float(viewport[1]) },
			{ float(viewport[0] + viewport[2]), float(viewport[1] + viewport[3]) }
		};

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(area.upperLeft.x, area.lowerRight.x, area.lowerRight.y, area.upperLeft.x);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();


		oui::fill(area.popTop(32), oui::colors::blue);

		area.upperLeft = font.drawText(area, "Test - \xce\xa9");
		auto frame_end = std::chrono::high_resolution_clock::now();
		auto frame_duration = std::chrono::duration<float>(frame_end - frame_begin).count();
		float cpu = 60 * frame_duration;
		if (max_cpu < cpu)
			max_cpu = cpu;

		font.drawText(area, " render cpu: " + std::to_string(cpu) + " (max: " + std::to_string(max_cpu) + ")");

		if (last_frame_end)
		{
			auto dt = std::chrono::duration<float>(frame_end - *last_frame_end).count();
			//debug::println(std::to_string(1/dt) + "fps");
		}
		last_frame_end = frame_end;
	}
}

