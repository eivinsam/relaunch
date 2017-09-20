#include "window.h"
#include "debug.h"
#include <iostream>
#include <optional>
#include <chrono>
#include <cmath>


#include <GL/glew.h>
namespace window
{

	Description initialize()
	{
		return { 640, 480 };
	}


	void update()
	{
		static std::optional<decltype(std::chrono::high_resolution_clock::now())> last_frame_end;
		static const auto first_time = std::chrono::high_resolution_clock::now();
		auto t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - first_time).count();

		/* rotate a triangle around */
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(0, 1);
		glColor3f(0.0f, 1.0f, 0.0f); glVertex2f(-cos(t), -1);
		glColor3f(0.0f, 0.0f, 1.0f); glVertex2f(+cos(t), -1);
		glEnd();
		auto frame_end = std::chrono::high_resolution_clock::now();
		if (last_frame_end)
		{
			auto dt = std::chrono::duration<float>(frame_end - *last_frame_end).count();
			//debug::println(std::to_string(1/dt) + "fps");
		}
		last_frame_end = frame_end;
	}
}

