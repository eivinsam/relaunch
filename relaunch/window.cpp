/* An example of the minimal Win32 & OpenGL program.  It only works in
16 bit color modes or higher (since it doesn't create a
palette). */

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>			/* must include this before GL/gl.h */
#include <stdio.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <unordered_map>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include "window.h"
#include "debug.h"

static HDC _dc;

LONG WINAPI
WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static std::unordered_map<UINT, std::string> msg_names = 
	{
		{ WM_PAINT, "WM_PAINT" },
		{ WM_CLOSE, "WM_CLOSE" },
		{ WM_DESTROY, "WM_DESTROY" },
		{ WM_QUIT, "WM_QUIT" },
		{ WM_CHAR, "WM_CHAR "}
	};
	auto found = msg_names.find(msg);
	if (found != msg_names.end())
		debug::println(found->second);
	else
		debug::println(std::to_string(msg));

	switch (msg) {
	//case WM_PAINT:
	//	return 0;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_CHAR:
		switch (wParam) {
		case 27:			/* ESC key */
			PostQuitMessage(0);
			break;
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(wnd, msg, wParam, lParam);
}

HWND
CreateOpenGLWindow(const char* title, int width, int height,
	BYTE type, DWORD flags)
{
	int         pf;
	HWND        _wnd;
	WNDCLASS    wc;
	PIXELFORMATDESCRIPTOR pfd;
	static HINSTANCE hInstance = 0;

	/* only register the window class once - use hInstance as a flag. */
	if (!hInstance) {
		hInstance = GetModuleHandle(NULL);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = (WNDPROC)WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = "OpenGL";

		if (!RegisterClass(&wc)) {
			MessageBox(NULL, "RegisterClass() failed:  "
				"Cannot register window class.", "Error", MB_OK);
			return NULL;
		}
	}

	_wnd = CreateWindow("OpenGL", title, WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

	if (_wnd == NULL) {
		MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
			"Error", MB_OK);
		return NULL;
	}

	_dc = GetDC(_wnd);

	/* there is no guarantee that the contents of the stack that become
	the pfd are zeroed, therefore _make sure_ to clear these bits. */
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
	pfd.iPixelType = type;
	pfd.cColorBits = 32;

	pf = ChoosePixelFormat(_dc, &pfd);
	if (pf == 0) {
		MessageBox(NULL, "ChoosePixelFormat() failed:  "
			"Cannot find a suitable pixel format.", "Error", MB_OK);
		return 0;
	}

	if (SetPixelFormat(_dc, pf, &pfd) == FALSE) {
		MessageBox(NULL, "SetPixelFormat() failed:  "
			"Cannot set format specified.", "Error", MB_OK);
		return 0;
	}

	DescribePixelFormat(_dc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	ReleaseDC(_wnd, _dc);

	return _wnd;
}

bool dispatchMessages()
{
	MSG   msg;				/* message */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

int APIENTRY
WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst,
	LPSTR lpszCmdLine, int nCmdShow)
{
	HGLRC _rc;				/* opengl context */
	HWND  _wnd;				/* window */

	auto desc = window::initialize();

	_wnd = CreateOpenGLWindow("relaunch", desc.width, desc.width, PFD_TYPE_RGBA, PFD_DOUBLEBUFFER);
	if (_wnd == NULL)
		return 1;

	_dc = GetDC(_wnd);
	_rc = wglCreateContext(_dc);
	wglMakeCurrent(_dc, _rc);
	glewInit();
	wglSwapIntervalEXT(1);

	ShowWindow(_wnd, nCmdShow);

	while (dispatchMessages())
	{
		window::update();
		SwapBuffers(_dc);
		glFlush();
	}

	wglMakeCurrent(NULL, NULL);
	ReleaseDC(_wnd, _dc);
	wglDeleteContext(_rc);
	DestroyWindow(_wnd);

	return 0;
}
