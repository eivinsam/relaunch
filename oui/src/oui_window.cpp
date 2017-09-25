/* An example of the minimal Win32 & OpenGL program.  It only works in
16 bit color modes or higher (since it doesn't create a
palette). */

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windowsx.h>			/* must include this before GL/gl.h */
#include <ShellScalingApi.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <unordered_map>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "Shcore.lib")

#include <oui_window.h>
#include <oui_debug.h>

namespace oui
{
	Input input;

	LONG WINAPI
		WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static std::unordered_map<UINT, std::string> msg_names =
		{
			{ WM_PAINT, "WM_PAINT" },
			{ WM_CLOSE, "WM_CLOSE" },
			{ WM_DESTROY, "WM_DESTROY" },
			{ WM_QUIT, "WM_QUIT" },
			{ WM_CHAR, "WM_CHAR "},
			{ WM_MOUSEMOVE, "WM_MOUSEMOVE" },
			{ WM_LBUTTONDOWN, "WM_LBUTTONDOWN" },
			{ WM_LBUTTONUP, "WM_LBUTTONUP" },
			{ WM_RBUTTONDOWN, "WM_RBUTTONDOWN" },
			{ WM_RBUTTONUP, "WM_RBUTTONUP" }
		};
		auto found = msg_names.find(msg);
		if (found != msg_names.end())
			debug::println(found->second);
		else
			debug::println(std::to_string(msg));

		auto get_point_lparam = [](LPARAM p) { return oui::Point{ float(GET_X_LPARAM(p)), float(GET_Y_LPARAM(p)) }; };

		switch (msg) {
			//case WM_PAINT:
			//	return 0;

		case WM_SIZE:
			glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_CHAR:
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_MOUSEMOVE:
			input.mouse.move(get_point_lparam(lParam));
			return 0;
		case WM_LBUTTONDOWN:
			SetCapture(wnd);
			input.mouse.press(get_point_lparam(lParam));
			return 0;
		case WM_RBUTTONDOWN:
			SetCapture(wnd);
			input.mouse.longPress(get_point_lparam(lParam));
			return 0;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture();
			input.mouse.release(get_point_lparam(lParam));
			return 0;
		default:
			return DefWindowProc(wnd, msg, wParam, lParam);
		}
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

	namespace window
	{
		unsigned window::dpi()
		{
			const POINT home = { 0 ,0 };
			UINT xdpi, ydpi;
			GetDpiForMonitor(MonitorFromPoint(home, MONITOR_DEFAULTTOPRIMARY), MDT_DEFAULT, &xdpi, &ydpi);
			return unsigned(sqrt(xdpi*ydpi));
		}
	}

	class Window
	{
		friend class Renderer;
		HWND _wnd;
		HDC _dc;
	public:
		Window(const window::Description& desc, std::optional<int> custom_pixel_format = std::nullopt)
		{
			static HINSTANCE hInstance = 0;

			/* only register the window class once - use hInstance as a flag. */
			if (!hInstance) {
				hInstance = GetModuleHandle(NULL);
				WNDCLASS    wc;
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

				if (!RegisterClass(&wc))
					throw std::runtime_error("RegisterClass() failed:  Cannot register window class");
			}

			_wnd = CreateWindow("OpenGL", desc.title.c_str(), WS_OVERLAPPEDWINDOW |
								WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								CW_USEDEFAULT, CW_USEDEFAULT, desc.width, desc.height, NULL, NULL, hInstance, NULL);
			if (_wnd == NULL)
				throw std::runtime_error("CreateWindow() failed:  Cannot create a window.");

			_dc = GetDC(_wnd);

			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;

			const int pf = custom_pixel_format.value_or(ChoosePixelFormat(_dc, &pfd));
			if (pf == 0)
				throw std::runtime_error("ChoosePixelFormat() failed:  Cannot find a suitable pixel format");

			if (SetPixelFormat(_dc, pf, &pfd) == FALSE)
				throw std::runtime_error("SetPixelFormat() failed:  Cannot set format specified");
		}
		Window(const Window&) = delete;
		Window(Window&& b) : _wnd(b._wnd), _dc(b._dc) { b._wnd = NULL; b._dc = NULL; }
		~Window()
		{
			if (_dc != NULL)
				ReleaseDC(_wnd, _dc);
			if (_wnd != NULL)
			{
				DestroyWindow(_wnd);
				dispatchMessages();
			}
		}

		void show(int mode) const { ShowWindow(_wnd, mode); }

		void swapBuffers() const { SwapBuffers(_dc); }
	};

	class Renderer
	{
		HGLRC _rc;

		static std::optional<int> maybe_fancy_format(const window::Description& desc)
		{
			if (desc.sampleCount <= 1)
				return std::nullopt;

			int format = 0;
			auto dummy_desc = desc;
			dummy_desc.sampleCount = 1;
			Renderer dummy(dummy_desc);

			int int_attributes[] =
			{
				WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
				WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB,24,
				WGL_ALPHA_BITS_ARB,8,
				WGL_DEPTH_BITS_ARB,16,
				WGL_STENCIL_BITS_ARB,0,
				WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
				WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
				WGL_SAMPLES_ARB, desc.sampleCount,
				0, 0
			};
			float float_attributes[] = { 0, 0 };

			UINT formatc = 0;
			wglChoosePixelFormatARB(dummy.window._dc, int_attributes, float_attributes, 1, &format, &formatc);
			if (formatc == 0)
				throw std::runtime_error("Failed to find appropriate pixel format");

			return format;
		}
	public:
		Window window;

		Renderer(const window::Description& desc) : window(desc, maybe_fancy_format(desc))
		{
			static bool glew_inited = false;

			_rc = wglCreateContext(window._dc);
			wglMakeCurrent(window._dc, _rc);

			if (!glew_inited)
				glew_inited = glewInit() == GLEW_OK;

		}
		Renderer(Renderer&& b) : _rc(b._rc), window(std::move(b.window)) { b._rc = NULL; }
		Renderer(const Renderer&) = delete;
		~Renderer()
		{
			if (_rc != NULL)
			{
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(_rc);
			}
		}
	};
}

int APIENTRY
WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst,
		LPSTR lpszCmdLine, int nCmdShow)
{
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

	oui::Renderer renderer(oui::window::initialize());


	renderer.window.show(nCmdShow);

	while (oui::dispatchMessages())
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		oui::Rectangle area =
		{
			{ float(viewport[0]), float(viewport[1]) },
			{ float(viewport[0] + viewport[2]), float(viewport[1] + viewport[3]) }
		};

		oui::window::update(area, oui::input);
		renderer.window.swapBuffers();
		glFlush();
		oui::input.mouse.takeAll();
	}

	return 0;
}
