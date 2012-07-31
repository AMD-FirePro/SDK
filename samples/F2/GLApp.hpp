
#pragma once
#include <map>

#include "GLWindow.hpp"
class RenderManager;

class GLApp
{
public:
	static GLApp * Create(const char * class_name);
	virtual ~GLApp();

	RenderManager * GetRenderManager()	{ return m_pRenderManager;	}
	GLWindow * GetWindow()				{ return &m_Window;	}

	virtual bool	Initialize();
	virtual void	Deinitialize();
	virtual void	Update(DWORD milliseconds) = 0;
	virtual void	Render() = 0;
	virtual void	RenderModel() = 0;

	virtual bool OnKeyDown(unsigned int key);
	virtual bool OnKeyUp(unsigned int key);
	virtual bool OnLeftButtonDown(int x, int y) { return false; }
	virtual bool OnLeftButtonUp(int x, int y) { return false; }
	virtual bool OnMouseMove(int x, int y) { return false; }
	virtual bool OnMouseWheel(int x, int y, int w) { return false; }

	void	ToggleFullscreen();
	void	TerminateApplication();
	void	ResizeDraw(bool enable) { m_ResizeDraw = enable; }

	GLApp(const char * class_name);

	bool	m_bShadow;		// shadowmap
	bool	m_bDiffuse;		// diff texture
	bool	m_bNormal;		// normal map
	bool	m_bEnvmap;		// envmap
	bool	m_bSSAO;		// Screen-space Ambient Occlusion
	bool	m_bOIT;			// Order Independent Transparency
	bool	m_bGlow;
	bool	m_disableGLDEbug;

	GLuint m_texDiffuse;
	GLuint m_texBackground;
	GLuint m_texBump;
	GLuint m_texEnvCube;
	GLuint m_texNoise;


	const glm::vec4 & GetLightDirection0()			{ return m_lightDirection0;	}
	void	SetLightDirection0(glm::vec4 v)	{ m_lightDirection0 = glm::normalize(v);	}

	glm::mat4 mModelView;
	glm::mat4 mModelViewProj;
	glm::mat4 mModelViewInv;

	void StartCaptureMouse()	{ m_Window.StartCaptureMouse();	}
	void StopCaptureMouse()		{ m_Window.StopCaptureMouse();	}

	void SetModelSize(float size)							{ m_fModelSize = size;		}
	void SetModelCenter(const glm::vec3 & center)			{ m_vModelCenter = center;	}

	float GetModelSize()				{ return m_fModelSize;		}
	const glm::vec3 &GetModelCenter()	{ return m_vModelCenter;	}

private:
	friend int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	int		Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

	friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	Message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static const UINT WM_TOGGLEFULLSCREEN = (WM_USER + 1);

	GLWindow		m_Window;
	RenderManager * m_pRenderManager;

	const char * m_ClassName;
	bool		m_IsProgramLooping;
	bool		m_CreateFullScreen;
	bool		m_IsVisible;
	bool		m_ResizeDraw;
	DWORD		m_LastTickCount;
	glm::vec4	m_lightDirection0; // used for self-shadowing

	glm::vec3	m_vModelCenter;
	float		m_fModelSize;

protected:
	int		m_iLeftMouseDownXlast;
	int		m_iLeftMouseDownYlast;
	bool	m_bLeftMouseDown;

	float	m_fZoom;
	float	m_fAngleX;
	float	m_fAngleY;
};

