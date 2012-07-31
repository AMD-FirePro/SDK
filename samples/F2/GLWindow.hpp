
#pragma once

#include "utils.hpp"

class GLWindow
{
public:
	GLWindow();

	bool	Create(const char * window_title, bool full_screen, const char * class_name, HINSTANCE h_instance, LPVOID lpParam);
	void	Destroy();
	bool	ChangeScreenResolution();
	void	SwapBuffers() { ::SwapBuffers(m_hDC); }

	void	SetPosX(int x);
	void	SetPosX(unsigned short x) { SetPosX((signed short)x); }
	void	SetPosY(int y);
	void	SetPosY(unsigned short y) { SetPosY((signed short)y); }
	int		GetWidth();
	int		GetHeight();
	void	SetWidth(int width);
	void	SetHeight(int height);
	int		GetPosX();
	int		GetPosY();
	operator HWND() { return m_hWnd; }

	float GetNearPlane()		{ return m_nearPlane;	}
	float GetFarPlane()			{ return m_farPlane;	}

	void StartCaptureMouse()	{ SetCapture(m_hWnd);	}
	void StopCaptureMouse()		{ ReleaseCapture();		}

private:
	HWND	m_hWnd;
	HDC		m_hDC;
	HGLRC	m_hRC;

	int		m_WindowPosX;
	int		m_WindowPosY;
	int		m_WindowWidth;
	int		m_WindowHeight;
	int		m_ScreenWidth;
	int		m_ScreenHeight;
	int		m_BitsPerPixel;
	bool	m_IsFullScreen;

	float	m_nearPlane;
	float	m_farPlane;
};

