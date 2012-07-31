#include "GLApp.hpp"
#include "Utils.hpp"
#include "RenderManager.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int ret = -1;
	GLApp * appl = GLApp::Create("SampleGL");
	if (appl != 0)
	{
		ret = appl->Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
		delete appl;
	}
	else
	{
		MessageBox(HWND_DESKTOP, "Error Creating Application", "Error", MB_OK | MB_ICONEXCLAMATION);
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR user_data = GetWindowLongPtr (hWnd, GWLP_USERDATA);
	if (user_data == 0)
	{
		if (uMsg == WM_CREATE)
		{
			CREATESTRUCT * creation = reinterpret_cast<CREATESTRUCT *>(lParam);
			GLApp * appl = reinterpret_cast<GLApp *>(creation->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(appl));
			appl->m_IsVisible = true;
			return 0;
		}
	}
	else
	{
		GLApp * appl = reinterpret_cast<GLApp *>(user_data);
		return appl->Message(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////////////////////
GLApp::GLApp(const char * class_name) :
	m_ClassName(class_name),
	m_IsProgramLooping(true),
	m_CreateFullScreen(false),
	m_IsVisible(false),
	m_ResizeDraw(false),
	m_LastTickCount(0),
	m_pRenderManager(NULL),

	m_bDiffuse(true),
	m_bNormal(false),
	m_bShadow(true),
	m_bEnvmap(false),
	m_bSSAO(false),
	m_bOIT(true),
	m_disableGLDEbug(false),
	m_bGlow(false),

	m_bLeftMouseDown(false),
	m_iLeftMouseDownXlast(0),
	m_iLeftMouseDownYlast(0),

	m_fZoom(1.0f),
	m_fAngleX(0.0f),
	m_fAngleY(0.0f)
{
	m_pRenderManager = new RenderManager(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////
GLApp::~GLApp()
{
	delete m_pRenderManager;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool GLApp::Initialize()
{
	return m_pRenderManager->Initialize();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLApp::Deinitialize()
{
	m_pRenderManager->Deinitialize();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLApp::ToggleFullscreen()
{
	PostMessage(m_Window, WM_TOGGLEFULLSCREEN, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLApp::TerminateApplication()
{
	PostMessage(m_Window, WM_QUIT, 0, 0);
	m_IsProgramLooping = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Message Handler
LRESULT GLApp::Message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SYSCOMMAND:
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
				break;
			}
		break;

		case WM_CLOSE:
			TerminateApplication();
			return 0;
		break;

		case WM_EXITMENULOOP:
		case WM_EXITSIZEMOVE:
			m_LastTickCount = GetTickCount();
			return 0;
		break;

		case WM_MOVE:
			m_Window.SetPosX(LOWORD(lParam));
			m_Window.SetPosY(HIWORD(lParam));
			return 0;
		break;

		case WM_PAINT:
			if (m_ResizeDraw == true)
			{
				Render();
				m_Window.SwapBuffers();
			}
		break;

		case WM_SIZING:
		{
			RECT * rect = (RECT *)lParam;
			m_Window.SetWidth(rect->right - rect->left);
			m_Window.SetHeight(rect->bottom - rect->top);
			m_pRenderManager->OnResize(m_Window.GetWidth(), m_Window.GetHeight());
			return TRUE;
		}
		break;

		case WM_SIZE:
			switch (wParam)
			{
				case SIZE_MINIMIZED:
					m_IsVisible = false;
					return 0;
				break;

				case SIZE_MAXIMIZED:
				case SIZE_RESTORED:
					m_IsVisible = true;
					m_Window.SetWidth(LOWORD(lParam));
					m_Window.SetHeight(HIWORD(lParam));
					m_LastTickCount = GetTickCount();
					m_pRenderManager->OnResize(m_Window.GetWidth(), m_Window.GetHeight());
					return 0;
				break;
			}
		break;

		case WM_KEYDOWN:
			OnKeyDown((unsigned int)wParam);
			return 0;
		break;

		case WM_KEYUP:
			OnKeyUp((unsigned int)wParam);
			return 0;
		break;

		case WM_MOUSEWHEEL:
			OnMouseWheel((short)LOWORD(lParam), (short)HIWORD(lParam), GET_WHEEL_DELTA_WPARAM(wParam));
			return 0;

		case WM_MOUSEMOVE:
			OnMouseMove((short)LOWORD(lParam), (short)HIWORD(lParam));
			return 0;

		case WM_LBUTTONDOWN:
			OnLeftButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam));
			return 0;

		case WM_LBUTTONUP:
			OnLeftButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam));
			return 0;

		case WM_TOGGLEFULLSCREEN:
			m_CreateFullScreen = !m_CreateFullScreen;
			PostMessage(hWnd, WM_QUIT, 0, 0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLApp::OnKeyDown(unsigned int key)
{
	switch (key)
	{
		case VK_ESCAPE:
			TerminateApplication();
			break;

		case VK_F1:
			ToggleFullscreen();
			break;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLApp::OnKeyUp(unsigned int key)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Program Main Loop
int GLApp::Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);
	windowClass.hInstance		= hInstance;
	windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName	= m_ClassName;
	if (RegisterClassEx(&windowClass) == 0)
	{
		MessageBox(HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}

	while (m_IsProgramLooping)
	{
		if (m_Window.Create("GL Framework", m_CreateFullScreen, m_ClassName, hInstance, this) == true)
		{
			if (Initialize() == false)
			{
				TerminateApplication();
			}
			else
			{
				MSG msg;
				bool isMessagePumpActive = true;
				m_LastTickCount = GetTickCount();
				while (isMessagePumpActive == true)
				{
					if (PeekMessage(&msg, m_Window, 0, 0, PM_REMOVE) != 0)
					{
						if (msg.message != WM_QUIT)
						{
							DispatchMessage(&msg);
						}
						else
						{
							isMessagePumpActive = false;
						}
					}
					else
					{
						if (m_IsVisible == false)
						{
							WaitMessage();
						}
						else
						{
							DWORD tickCount = GetTickCount();
							Update(tickCount - m_LastTickCount);
							m_LastTickCount = tickCount;
							Render();
							m_Window.SwapBuffers();
						}
					}
				}
			}

			Deinitialize();
			m_Window.Destroy();
		}
		else
		{
			MessageBox(HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			m_IsProgramLooping = false;
		}
	}

	UnregisterClass(m_ClassName, hInstance);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////