#include <oui_debug.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void oui::debug::println(std::string text)
{
	text += '\n';
	OutputDebugStringA(text.c_str());
}
