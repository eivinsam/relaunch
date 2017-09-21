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

		auto text_overlay = area;

		if (area.height() < area.width())
		{
			const auto half_delta = (area.width() - area.height())/2;
			area.popLeft(half_delta);
			area.popRight(half_delta);
		}
		else
		{
			area.popBottom(area.height() - area.width());
		}

		for (int i = 0; i < 8; ++i)
		{
			auto row = area.popTop(oui::Ratio(1.0f / (8 - i)));
			for (int j = 0; j < 8; ++j)
				fill(row.popLeft(oui::Ratio(1.0f / (8 - j))).shrink(oui::Ratio(0.85f)), 
					 oui::Color{ i / 8.0f, j / 8.0f });
		}

		text_overlay.upperLeft = font.drawText(text_overlay, "Test - \xce\xa9");
		auto frame_end = std::chrono::high_resolution_clock::now();
		auto frame_duration = std::chrono::duration<float>(frame_end - frame_begin).count();
		float cpu = 60 * frame_duration;
		if (max_cpu < cpu)
			max_cpu = cpu;

		font.drawText(text_overlay, " render cpu: " + std::to_string(cpu) + " (max: " + std::to_string(max_cpu) + ")");

		if (last_frame_end)
		{
			auto dt = std::chrono::duration<float>(frame_end - *last_frame_end).count();
			//debug::println(std::to_string(1/dt) + "fps");
		}
		last_frame_end = frame_end;
	}
}

