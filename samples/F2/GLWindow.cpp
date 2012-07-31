#include "GLWindow.h"

#include <gl/glew.h>


GLWindow::GLWindow() :
	m_WindowPosX(0),
	m_WindowPosY(0),
	m_WindowWidth(1024),
	m_WindowHeight(768),
	m_BitsPerPixel(32),
	m_IsFullScreen(false),

	m_hWnd(NULL),
	m_hDC(NULL),
	m_hRC(NULL),

	m_nearPlane(1.0f),
	m_farPlane(100.0f)
{
	m_ScreenWidth = m_WindowWidth;
	m_ScreenHeight = m_WindowHeight;
}


int GLWindow::GetWidth()
{
	if (m_IsFullScreen == true)
	{
		return m_ScreenWidth;
	}
	else
	{
		return m_WindowWidth;
	}
}
int GLWindow::GetHeight()
{
	if (m_IsFullScreen == true)
	{
		return m_ScreenHeight;
	}
	else
	{
		return m_WindowHeight;
	}
}

void GLWindow::SetWidth(int width)
{
	if (m_IsFullScreen == true)
	{
		m_ScreenWidth = width;
	}
	else
	{
		m_WindowWidth = width;
	}
}
void GLWindow::SetHeight(int height)
{
	if (m_IsFullScreen == true)
	{
		m_ScreenHeight = height;
	}
	else
	{
		m_WindowHeight = height;
	}
}

int GLWindow::GetPosX()
{
	if (m_IsFullScreen == false)
	{
		return m_WindowPosX;
	}
	return 0;
}
int GLWindow::GetPosY()
{
	if (m_IsFullScreen == false)
	{
		return m_WindowPosY;
	}
	return 0;
}

void GLWindow::SetPosX(int x)
{
	if (m_IsFullScreen == false)
	{
		m_WindowPosX = x;
	}
}
void GLWindow::SetPosY(int y)
{
	if (m_IsFullScreen == false)
	{
		m_WindowPosY = y;
	}
}

bool GLWindow::ChangeScreenResolution()
{
	DEVMODE dmScreenSettings;
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));
	dmScreenSettings.dmSize			= sizeof(DEVMODE);
	dmScreenSettings.dmPelsWidth	= GetWidth();
	dmScreenSettings.dmPelsHeight	= GetHeight();
	dmScreenSettings.dmBitsPerPel	= m_BitsPerPixel;
	dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return false;
	}
	return true;
}

bool GLWindow::Create(const char * window_title, bool full_screen, const char * class_name, HINSTANCE h_instance, LPVOID lpParam)
{
	m_IsFullScreen = full_screen;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,																// Version Number
		PFD_DRAW_TO_WINDOW	|											// Format Must Support Window
		PFD_SUPPORT_OPENGL	|											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		m_BitsPerPixel,													// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;

	if (m_IsFullScreen == true)
	{
		if (ChangeScreenResolution() == false)
		{
			MessageBox(HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			m_IsFullScreen = false;
		}
		else
		{
			ShowCursor(false);
			windowStyle = WS_POPUP;
			windowExtendedStyle |= WS_EX_TOPMOST;
		}
	}

	RECT windowRect = {GetPosX(), GetPosY(), GetPosX() + GetWidth(), GetPosY() + GetHeight()};
	if (m_IsFullScreen == false)
	{
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
		if (windowRect.left < 0)
		{
			windowRect.right -= windowRect.left;
			windowRect.left = 0;
		}
		if (windowRect.top < 0)
		{
			windowRect.bottom -= windowRect.top;
			windowRect.top = 0;
		}
	}

	// Create The OpenGL Window
	m_hWnd = CreateWindowEx(windowExtendedStyle, class_name, window_title, windowStyle,
							windowRect.left, windowRect.top,
							windowRect.right - windowRect.left,
							windowRect.bottom - windowRect.top,
							HWND_DESKTOP,
							0,											// No Menu
							h_instance,
							lpParam);

	while (m_hWnd != 0)
	{
		m_hDC = GetDC(m_hWnd);
		if (m_hDC == 0)
		{
			break;
		}
		GLuint PixelFormat = ChoosePixelFormat(m_hDC, &pfd);
		if (PixelFormat == 0)
		{
			break;
		}
		if (SetPixelFormat(m_hDC, PixelFormat, &pfd) == false)
		{
			break;
		}
		m_hRC = wglCreateContext(m_hDC);
		if (m_hRC == 0)
		{
			break;
		}
		if (wglMakeCurrent(m_hDC, m_hRC) == false)
		{
			break;
		}

		int attribs[] = 
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
//			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,

//			WGL_CONTEXT_FLAGS_ARB, 0,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
//			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB|WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		}; 

		glewInit();
		glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControlARB");
		glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
		glDebugMessageInsertARB = (PFNGLDEBUGMESSAGEINSERTARBPROC)wglGetProcAddress("glDebugMessageInsertARB");

		if(wglCreateContextAttribsARB != NULL)
		{
			wglMakeCurrent(NULL,NULL); 
			wglDeleteContext(m_hRC);
			m_hRC = wglCreateContextAttribsARB(m_hDC,0, attribs);
			wglMakeCurrent(m_hDC, m_hRC);
		}
		ShowWindow(m_hWnd, SW_NORMAL);
		return true;
	}

	Destroy();
	return false;
}

// Destroy OpenGL Window
void GLWindow::Destroy()
{
	if (m_hWnd != 0)
	{
		if (m_hDC != 0)
		{
			wglMakeCurrent(m_hDC, 0);
			if (m_hRC != 0)
			{
				wglDeleteContext(m_hRC);
				m_hRC = 0;
			}
			ReleaseDC(m_hWnd, m_hDC);
			m_hDC = 0;
		}
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}

	if (m_IsFullScreen)
	{
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(true);
	}
}
