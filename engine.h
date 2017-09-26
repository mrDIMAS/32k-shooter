#pragma once

#include "scenenode.h"
#include "light.h"
#include "sound.h"

class Engine {
public:
	HGLRC mGLContext;
	HWND mWindow;
	HDC mDeviceContext;

	// Shared depth-stencil buffer
	GLuint mDepthBuffer;

	// G-Buffer stuff
	GLuint mColorTex;
	GLuint mDepthTex;
	GLuint mNormalsTex;

	GLuint mGBufferFrameBuffer;
	GLuint mDepthRenderTarget;
	GLuint mColorRenderTarget;
	GLuint mNormalRenderTarget;

	GLuint mGBufferVertexShader;
	GLuint mGBufferFragmentShader;
	GLuint mGBufferProgram;

	// Lighting stuff
	GLuint mLightFrameBuffer;
	GLuint mFrameTex;
	GLuint mFrameRenderTarget;

	GLuint mLightVertexShader;
	GLuint mLightFragmentShader;
	GLuint mLightProgram;

	GLuint mAmbientLightVertexShader;
	GLuint mAmbientLightFragmentShader;
	GLuint mAmbientLightProgram;

	// Quad stuff
	GLuint mQuadVertexShader;
	GLuint mQuadFragmentShader;
	GLuint mQuadProgram;

	GLuint mParticleVertexShader;
	GLuint mParticleFragmentShader;
	GLuint mParticleProgram;

	int mGBufferWidth;
	int mGBufferHeight;

	SceneNode * mSphere;

	GLuint unform_gbuf_worldMat;
	GLuint uniform_gbuf_color;

	GLint uniform_dep;
	GLint uniform_col;
	GLint uniform_nor;
	GLint uniform_invViewProj;
	GLint uniform_lp;
	GLint uniform_lc;
	GLint uniform_lv;

	GLint puniform_wm;
	GLint puniform_vpm;
	GLint puniform_camR;
	GLint puniform_camU;

	static LRESULT CALLBACK WindowProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void UpdateMouse();
	void FlushMouse();
	void CreateLightSphere( );

	~Engine();
	Engine( int w, int h );
	void UpdateMessages();
	void PrintShaderLog( GLuint object );
	GLuint CreateShader( GLenum type, const char * source );
	void BuildShader( const char * vertexSource, const char * fragmentSource, GLuint & vs, GLuint & fs, GLuint & prog  );
	void InitShaders();
	void DrawQuad( float x, float y, float w, float h );
	void SetCameraProjection();
	void SetOrthoProjection();
	void DrawLightSphere( Light * pLight );
	void RenderWorld(); 
	void Update();
};