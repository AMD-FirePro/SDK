#pragma once

#include "GLApp.h"


class Mesh;
class ModelAndTextureLoader;

class GLSample : GLApp
{
public:
	bool	Initialize() override;
	void	Deinitialize() override;

	void	Update(DWORD milliseconds) override;
	void	Render() override;
	 void	RenderModel() override;

	void	DrawBackground();
	void	DrawGlow();

	bool OnKeyDown(unsigned int key) override;
	bool OnKeyUp(unsigned int key) override;
	bool OnLeftButtonDown(int x, int y) override;
	bool OnLeftButtonUp(int x, int y) override;
	bool OnMouseMove(int x, int y) override;
	bool OnMouseWheel(int x, int y, int w) override;


	void	SetupOpenGLStates();
	void	ResetMaterialProperties();
	void	SetViewTransform();
	void 	SetDebug();

private:
	friend class GLApp;
	GLSample(const char * class_name);

	void  recursiveMeshRendering(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh);

	ModelAndTextureLoader* fullModel;

	float	m_Angle;
	bool	m_bAnimate;		// auto-rotation of the model
	bool	m_doubleSided;	// toggle double sided lighting (Shaders only)
	bool	m_bBlurTest;	// toggle render to target + blurr pass
	int		m_numLights;
	float	m_texStrength;
	float	m_envStrength;
	float	m_envStrengthGoal;
	float	m_blurLevel;
	float	m_texStrengthGoal;
	float	m_shadowStrength;
	float	m_shadowStrengthGoal;
	float	m_fAlpha;
	bool	m_bUseSSAO;
	bool	m_bDebugRTT;
};

