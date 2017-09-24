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

	class ColorPicker
	{
		float k_r;
		float k_b;
	public:
		constexpr ColorPicker(float k_r, float k_b) : k_r(k_r), k_b(k_b) { }

		Optional<Color> operator()(const Rectangle& area, const float y) const
		{
			auto put_point = [](const Point& p, const Color& c) { glColor3f(c.r, c.g, c.b); glVertex2f(p.x, p.y); };

			const float coef[6] =
			{
				k_r,             // red
				1 - k_b,         // yellow
				1 - (k_r + k_b), // green
				1 - k_r,         // cyan
				k_b,             // blue
				k_r + k_b        // magenta
			};
			const float ym = 1 - y;
			float axis_max[6];
			for (int i = 0; i < 6; ++i)
				axis_max[i] = (y < coef[i] ? 
							   y / coef[i] : 
							   ym / coef[(i + 3) % 6]);

			const float max_a = max(max(max(max(max(axis_max[0], axis_max[1]), axis_max[2]), axis_max[3]), axis_max[4]), axis_max[5]);

			const auto c = area.center();
			const float h = min(area.width(), area.height())/2;

			static constexpr float sin3 = 0.86602540378443864676372317075294f;
			static constexpr Vector axes[6] = 
			{
				{ +0.0f, -1.0f },
				{ +sin3, -0.5f },
				{ +sin3, +0.5f },
				{ -0.0f, +1.0f },
				{ -sin3, +0.5f },
				{ -sin3, -0.5f }
			};
			using namespace colors;
			static constexpr Color color[6] = { red, yellow, green, cyan, blue, magenta };

			auto color_max = [&](int i)
			{
				const int j = i < 3 ? i + 3 : i - 3;
				return color[i] * (y / coef[i]) + color[j] * (y - coef[i] * axis_max[i]) / coef[j];
			};

			glBegin(GL_TRIANGLE_FAN);
			put_point(c, white*y);
			for (int i = 0; i < 6; ++i)
				put_point(c + h*axes[i]*axis_max[i]/max_a, color_max(i));
			put_point(c + h*axes[0]*axis_max[0]/max_a, color_max(0));
			glEnd();

			return std::nullopt;
		}
	};

	static constexpr ColorPicker pickColorUV(0.299f, 0.114f);
	static constexpr ColorPicker pickColor(0.3f, 0.2f);

	//Optional<Color> pickColorUV(const Rectangle& area, float y)
	//{
	//	static constexpr float Yr = 0.299f;
	//	static constexpr float Yb = 0.114f;
	//	static constexpr float Yg = 1-(Yr+Yb);
	//	auto put_color = [y](float r, float b) { glColor3f(r, (y - (r*Yr + b*Yb)) / Yg, b);  };
	//
	//	glBegin(GL_QUADS);
	//	put_color(0, 0); glVertex2f(area.min.x, area.min.y);
	//	put_color(0, 1); glVertex2f(area.min.x, area.max.y);
	//	put_color(1, 1); glVertex2f(area.max.x, area.max.y);
	//	put_color(1, 0); glVertex2f(area.max.x, area.min.y);
	//	glEnd();
	//
	//	return std::nullopt;
	//}
}


namespace window
{

	Description initialize()
	{
		return { 960, 480 };
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
			auto right = area.popRight(area.width() - area.height());
			static float intensity = 0.5f;

			oui::verticalSlider(right.popRight(20), intensity, input, oui::colors::white);
			oui::pickColor(right, 1-intensity);
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

