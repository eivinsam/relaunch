#include "window.h"
#include "debug.h"
#include <iostream>
#include <optional>
#include <chrono>
#include <cmath>

#include <GL/glew.h>

#include "text.h"

namespace oui
{
	void verticalSlider(Rectangle area, float& value, Input& input, const Color& color)
	{

		// handle mouse drag!
		if (auto delta = input.mouse.dragging(area))
		{
			value += delta->y / (area.height()*0.9f);
			if (value < 0)
				value = 0;
			if (value > 1)
				value = 1;
		}

		const float h = area.height()*0.1f;
		area.popTop(oui::Ratio(value*0.9f));
		fill(area.popTop(h), color);
	}
}


namespace window
{

	Description initialize()
	{
		return { 640, 480 };
	}


	void update(oui::Rectangle area, oui::Input& input)
	{
		static oui::Font font("Segoe UI", 24);
		static std::optional<decltype(std::chrono::high_resolution_clock::now())> last_frame_end;
		static const auto first_time = std::chrono::high_resolution_clock::now();
		auto frame_begin = std::chrono::high_resolution_clock::now();
		static float max_cpu = 0;
		static std::string message;


		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(area.min.x, area.max.x, area.max.y, area.min.x);
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
			{
				const auto button_area = row.popLeft(oui::Ratio(1.0f / (8 - j)));
				if (input.mouse.pressed(button_area))
					message = "(" + std::to_string(i) + " " + std::to_string(j) + ")";
				if (input.mouse.longPressed(button_area))
					message = "[" + std::to_string(i) + " " + std::to_string(j) + "]";
				fill(button_area.shrink(oui::Ratio(0.85f)),
					 oui::Color{ i / 8.0f, j / 8.0f });
			}
		}

		text_overlay.min = font.drawText(text_overlay, "Test - \xce\xa9 - " + message);
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

