#include "precompiled.h"
#include "engine.h"
#include "camera.h"
#include "particlesystem.h"
#include "gui.h"


Mouse gMouse;
GLFuncList gl;
Engine * gEngine;
static char * glFuncListNames[] = {
	"glGenBuffersARB",
	"glBindBufferARB",
	"glDeleteBuffersARB",
	"glBufferDataARB",
	"glActiveTextureARB",
	"glClientActiveTextureARB",
	"glCreateProgramObjectARB",
	"glCreateShaderObjectARB",
	"glShaderSourceARB",
	"glCompileShaderARB",
	"glGetInfoLogARB",
	"glAttachObjectARB",
	"glLinkProgramARB",
	"glUseProgramObjectARB",
	"glDeleteObjectARB",
	"glGetObjectParameterivARB",
	"glGetUniformLocationARB",
	"glUniform4fARB",
	"glUniformMatrix4fvARB",
	"glUniform1iARB",
	"glGenFramebuffersEXT",
	"glBindFramebufferEXT",
	"glFramebufferTexture2DEXT",
	"glDeleteFramebuffersEXT",
	"glGenRenderbuffersEXT",
	"glBindRenderbufferEXT",
	"glRenderbufferStorageEXT",
	"glDeleteRenderbuffersEXT",
	"glFramebufferRenderbufferEXT",
	"glCheckFramebufferStatusEXT",
	"glDrawBuffersARB"
};

void LoadGLFunctions() {
	void ** fn = (void**)&gl;
	for( int i = 0; i < sizeof( gl ) / sizeof(PROC); i++ ) {
		fn[i] = wglGetProcAddress( glFuncListNames[i] );
	}
}

bool gWindowFocus = true;

LRESULT CALLBACK Engine::WindowProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
	case WM_DESTROY:
		gRunning = false;
		break;
	case WM_KILLFOCUS:
		gWindowFocus = false;	
		break;
	case WM_SETFOCUS:
		gWindowFocus = true;
		break;
	case WM_MOUSEWHEEL:
		gMouse.wheelSpeed = ((short)HIWORD( wParam )) / WHEEL_DELTA;
		break;
	case WM_MOUSEMOVE:
		gMouse.x = LOWORD( lParam );
		gMouse.y = HIWORD( lParam );
		break;			
	case WM_LBUTTONDOWN: 
		gMouse.buttonPressed[ 0 ] = true;  
		gMouse.buttonReleased[ 0 ] = false;            
		break;
	case WM_LBUTTONUP:
		gMouse.buttonReleased[ 0 ] = true;  
		gMouse.buttonPressed[ 0 ] = false;  
		break;
	case WM_RBUTTONDOWN:
		gMouse.buttonPressed[ 1 ] = true;  
		gMouse.buttonReleased[ 1 ] = false; 
		break;
	case WM_RBUTTONUP:
		gMouse.buttonReleased[ 1 ] = true;  
		gMouse.buttonPressed[ 1 ] = false;  
		break;
	case WM_MBUTTONDOWN:
		gMouse.buttonPressed[ 2 ] = true;  
		gMouse.buttonReleased[ 2 ] = false; 
		break;
	case WM_MBUTTONUP:
		gMouse.buttonReleased[ 2 ] = true;  
		gMouse.buttonPressed[ 2 ] = false;  
		break;
	}
	return DefWindowProcA ( wnd, msg, wParam, lParam );
}

void Engine::UpdateMouse() {
	RECT clientRect;
	POINT clientCenter;
	POINT mousePos;

	if( gWindowFocus ) {
		GetCursorPos( &mousePos );
		ScreenToClient( mWindow, &mousePos );

		gMouse.xSpeed = mousePos.x - gMouse.lastX;
		gMouse.ySpeed = mousePos.y - gMouse.lastY;

		GetClientRect( mWindow, &clientRect );

		clientCenter.x = (clientRect.right - clientRect.left) / 2;
		clientCenter.y = (clientRect.bottom - clientRect.top) / 2;

		gMouse.lastX = clientCenter.x;
		gMouse.lastY = clientCenter.y;

		ClientToScreen( mWindow, &clientCenter );

		if( !gMouse.mFree ) {
			SetCursorPos( clientCenter.x, clientCenter.y );
		} else {
			gMouse.xSpeed = 0;
			gMouse.ySpeed = 0;
		}
	}
}

void Engine::FlushMouse()
{
	gMouse.buttonPressed[ 0 ] = false;  
	gMouse.buttonReleased[ 0 ] = false;            
	gMouse.buttonReleased[ 0 ] = false;  
	gMouse.buttonPressed[ 0 ] = false;  
	gMouse.buttonPressed[ 1 ] = false;  
	gMouse.buttonReleased[ 1 ] = false; 
	gMouse.buttonReleased[ 1 ] = false;  
	gMouse.buttonPressed[ 1 ] = false;  
	gMouse.buttonPressed[ 2 ] = false;  
	gMouse.buttonReleased[ 2 ] = false; 
	gMouse.buttonReleased[ 2 ] = false;  
	gMouse.buttonPressed[ 2 ] = false;
}

void Engine::CreateLightSphere()
{
	mSphere = new SceneNode;
	mSphere->AddSurface( Surface::MakeSphere( 5, 5, 1.0f ));
	mSphere->mSurfaces[0]->mNoTextures = true;
	mSphere->mInRenderChain = false;
}

Engine::~Engine() {
	GLCall( glDeleteTextures( 1, &mDepthTex ));
	GLCall( glDeleteTextures( 1, &mColorTex ));
	GLCall( glDeleteTextures( 1, &mNormalsTex ));
	GLCall( gl.DeleteFramebuffersEXT( 1, &mGBufferFrameBuffer ));
	GLCall( gl.DeleteRenderbuffersEXT( 1, &mDepthRenderTarget ));
	GLCall( gl.DeleteRenderbuffersEXT( 1, &mColorRenderTarget ));
	GLCall( gl.DeleteRenderbuffersEXT( 1, &mNormalRenderTarget ));
	GLCall( gl.DeleteRenderbuffersEXT( 1, &mDepthBuffer ));
}

Engine::Engine( int w, int h ) {
	gEngine = this;

	gRunning = true;

	// Create Window
	const char * className = "Game";
	WNDCLASSEXA wcx;
	wcx.cbSize = sizeof ( wcx );
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = WindowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = GetModuleHandle ( 0 );
	wcx.hIcon = 0;
	wcx.hCursor = LoadCursor ( NULL, IDC_ARROW );
	wcx.hbrBackground = 0;
	wcx.lpszMenuName = 0;		
	wcx.lpszClassName = className;
	wcx.hIconSm = 0;

	RegisterClassExA ( &wcx );
	DWORD style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	mWindow = CreateWindowA( className, className, style, 0, 0, w, h, 0, 0, wcx.hInstance, 0 );
	mDeviceContext = GetDC( mWindow );

	ShowWindow ( mWindow, SW_SHOW );
	UpdateWindow ( mWindow );

	// Setup OpenGL
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	SetPixelFormat( mDeviceContext, ChoosePixelFormat( mDeviceContext, &pfd ), &pfd );     
	mGLContext = wglCreateContext( mDeviceContext );
	wglMakeCurrent( mDeviceContext, mGLContext );

	GLCall( glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST ));
	GLCall( glShadeModel( GL_SMOOTH ));
	GLCall( glClearDepth( 1.0f ));
	GLCall( glDepthFunc( GL_LEQUAL ));
	GLCall( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ));
	GLCall( glEnable( GL_DEPTH_TEST ));
	GLCall( glEnable( GL_TEXTURE_2D ));

	gAspect = (float)w / (float)h; 



	LoadGLFunctions();

	RECT clientRect;
	GetClientRect( mWindow, &clientRect ) ;
	mGBufferWidth = clientRect.right - clientRect.left;
	mGBufferHeight = clientRect.bottom - clientRect.top;

	const float typicalWidth = 1024.0f;
	const float typicalHeight = 768.0f;

	gGUIGlobalScale.x = mGBufferWidth / typicalWidth;
	gGUIGlobalScale.y = mGBufferHeight / typicalHeight;

	///////////////////////////////////////////////////////////
	// Deferred shading stuff
	GLCall( gl.GenFramebuffersEXT( 1, &mGBufferFrameBuffer ));
	GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, mGBufferFrameBuffer ));

	GLCall( gl.GenRenderbuffersEXT( 1, &mDepthRenderTarget ));
	GLCall( gl.BindRenderbufferEXT( GL_RENDERBUFFER_EXT, mDepthRenderTarget ));
	GLCall( gl.RenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_R32F, mGBufferWidth, mGBufferHeight ));
	GLCall( gl.FramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, mDepthRenderTarget ));

	GLCall( gl.GenRenderbuffersEXT( 1, &mColorRenderTarget ));
	GLCall( gl.BindRenderbufferEXT( GL_RENDERBUFFER_EXT, mColorRenderTarget ));
	GLCall( gl.RenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGBA8, mGBufferWidth, mGBufferHeight ));
	GLCall( gl.FramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER_EXT, mColorRenderTarget ));

	GLCall( gl.GenRenderbuffersEXT( 1, &mNormalRenderTarget ));
	GLCall( gl.BindRenderbufferEXT( GL_RENDERBUFFER_EXT, mNormalRenderTarget ));
	GLCall( gl.RenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_RGBA8, mGBufferWidth, mGBufferHeight ));
	GLCall( gl.FramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_RENDERBUFFER_EXT, mNormalRenderTarget ));

	GLCall( gl.GenRenderbuffersEXT( 1, &mDepthBuffer ));
	GLCall( gl.BindRenderbufferEXT( GL_RENDERBUFFER_EXT, mDepthBuffer ));
	GLCall( gl.RenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, mGBufferWidth, mGBufferHeight ));
	GLCall( gl.FramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, mDepthBuffer ));

	GLCall( glGenTextures( 1, &mDepthTex ));
	GLCall( glBindTexture( GL_TEXTURE_2D, mDepthTex ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));
	GLCall( glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, mGBufferWidth, mGBufferHeight, 0, GL_BGRA, GL_FLOAT, nullptr ));	

	GLCall( gl.FramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mDepthTex, 0 ));

	GLCall( glGenTextures( 1, &mColorTex ));
	GLCall( glBindTexture( GL_TEXTURE_2D, mColorTex ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));
	GLCall( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, mGBufferWidth, mGBufferHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr ));

	GLCall( gl.FramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mColorTex, 0 ));

	GLCall( glGenTextures( 1, &mNormalsTex ));
	GLCall( glBindTexture( GL_TEXTURE_2D, mNormalsTex ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));
	GLCall( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, mGBufferWidth, mGBufferHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr ));	

	GLCall( gl.FramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mNormalsTex, 0 ));

#ifdef _DEBUG
	if( gl.CheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT ) {
		MessageBoxA( NULL, "Unable to initialize FBO.", 0, MB_ICONERROR );
		ExitProcess( 1 );
	};
#endif

	GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ));

	// Create another framebuffer for stencil optimizations
	GLCall( gl.GenFramebuffersEXT( 1, &mLightFrameBuffer ));
	GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, mLightFrameBuffer ));

	GLCall( glGenTextures( 1, &mFrameTex ));
	GLCall( glBindTexture( GL_TEXTURE_2D, mFrameTex ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));
	GLCall( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, mGBufferWidth, mGBufferHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr ));	

	GLCall( gl.FramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mFrameTex, 0 ));

	GLCall( gl.FramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, mDepthBuffer ));

#ifdef _DEBUG
	if( gl.CheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT ) {
		MessageBoxA( NULL, "Unable to initialize FBO.", 0, MB_ICONERROR );
		ExitProcess( 1 );
	};
#endif

	Texture::CreateLibrary();

	GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ));

	InitShaders();

	CreateLightSphere();

	Sound::InitSoundSystem();
}

void Engine::UpdateMessages() {
	MSG message;
	while ( PeekMessageA ( &message, NULL, 0, 0, PM_REMOVE ) ) {
		TranslateMessage( &message );
		DispatchMessageA ( &message );
	}
}

void Engine::PrintShaderLog( GLuint object ) {
	static char buffer[1024];
	gl.GetInfoLogARB( object, 1024, NULL, buffer );
	MessageBoxA( 0, buffer, "Error",  MB_OK | MB_ICONERROR );
	ExitProcess(1);
}

GLuint Engine::CreateShader( GLenum type, const char * source ) {
	GLuint shader = gl.CreateShaderObjectARB( type );
	GLCall( gl.ShaderSourceARB( shader, 1, &source, 0 ));
	GLCall( gl.CompileShaderARB( shader ));
	GLint compiled;
	GLCall( gl.GetObjectParameterivARB( shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled ));
	if( !compiled ) {
		PrintShaderLog( shader );
	}
	return shader;
}

void Engine::BuildShader( const char * vertexSource, const char * fragmentSource, GLuint & vs, GLuint & fs, GLuint & prog ) {
	vs = CreateShader( GL_VERTEX_SHADER, vertexSource );
	fs = CreateShader( GL_FRAGMENT_SHADER, fragmentSource );

	prog = gl.CreateProgramObjectARB();

	GLCall( gl.AttachObjectARB( prog, vs ));
	GLCall( gl.AttachObjectARB( prog, fs ));

	GLCall( gl.LinkProgramARB( prog ));

	GLint linked;
	GLCall( gl.GetObjectParameterivARB( prog, GL_OBJECT_LINK_STATUS_ARB, &linked ));
	if( !linked ) {
		PrintShaderLog( prog );
	}   

	GLCall( gl.UseProgramObjectARB( 0 ));
}

void Engine::InitShaders() {
	const char * gBufferVertexSource = 
		"#version 130\n"
		"uniform mat4 wm;\n"
		"varying vec4 p;\n"
		"varying vec3 n;\n"
		"varying vec2 tc;\n"
		"void main(){\n"
			"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;\n"
			"tc=gl_MultiTexCoord0.xy;\n"
			"n=mat3(wm)*gl_Normal;\n"
			"p=gl_Position;\n"
		"}\n"
		;

	const char * gBufferFragmentSource = 
		"#version 130\n"
		"uniform sampler2D dif;\n"
		"varying vec4 p;\n"
		"varying vec3 n;\n"
		"varying vec2 tc;\n"
		"uniform vec4 c;\n"
		"uniform vec4 alb;\n"
		"void main(){\n"
			"gl_FragData[0]=vec4(p.z/p.w);\n"
			"gl_FragData[1]=c*texture2D(dif,tc);\n"
			"gl_FragData[1].a=alb.x;\n"
			"gl_FragData[2]=vec4(normalize(n)*.5+.5,1);\n"
		"}\n"
		;

	BuildShader( gBufferVertexSource, gBufferFragmentSource, mGBufferVertexShader, mGBufferFragmentShader, mGBufferProgram );

	const char * quadVertexSource = 
		"#version 130\n"
		"varying vec2 tc;\n"
		"varying vec4 c;\n"
		"void main(){\n"
			"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;\n"
			"tc=gl_MultiTexCoord0.xy;\n"
			"c=gl_Color;"
		"}\n"
		;

	const char * lightFragmentSource = 
		"#version 130\n"
		"uniform sampler2D dep,col,nor;\n"
		"uniform vec4 lp,lc,lv;\n"
		"uniform mat4 invViewProj;\n"
		"varying vec2 tc;\n"
		"void main(){\n"
			"vec3 n=texture2D(nor,tc).xyz*2.-1.;\n"
			"vec4 s;\n"
			"s.x=tc.x*2.-1.;\n"
			"s.y=tc.y*2.-1.;\n"
			"s.z=texture2D(dep,tc).r;\n"
			"s.w=1.;\n"
			"vec4 p=invViewProj*s;\n"
			"p/=p.w;\n"
			"vec3 l=p.xyz-lp.xyz;\n"
			"float d=min(length(l),lp.w);\n"
			"vec3 ln=l/d;\n"
			"float y=dot(lv.xyz,ln);\n"
			"float k=max(dot(n,ln),0);\n"
			"float a=1.+cos((d/lp.w)*3.14159);\n"
			"if(y<lv.w){"
				"a*=smoothstep(lv.w-.1,lv.w,y);\n"				
			"};\n"
			"gl_FragColor=1.5*k*a*texture2D(col,tc)*lc;\n"
		"}\n"
		;

	BuildShader( quadVertexSource, lightFragmentSource, mLightVertexShader, mLightFragmentShader, mLightProgram );


	const char * quadFragmentSource = 
		"#version 130\n"
		"uniform sampler2D col;\n"
		"varying vec2 tc;\n"
		"varying vec4 c;\n"
		"void main(){\n"
			"gl_FragColor=c*texture2D(col,tc);\n"
		"}\n"
		;

	BuildShader( quadVertexSource, quadFragmentSource, mQuadVertexShader, mQuadFragmentShader, mQuadProgram );

	const char * particleVertexSource = 
		"#version 130\n"
		"uniform mat4 wm,vpm;\n"
		"uniform vec4 camR,camU;\n"
		"varying vec2 tc;\n"
		"varying vec4 c;\n"
		"void main(){\n"
			"c=gl_Color;\n"
			"tc=gl_MultiTexCoord0.xy;\n"
			"vec4 wp=wm*gl_Vertex;\n"
			"vec4 lp=(gl_Normal.x*camR+gl_Normal.y*camU)*gl_Normal.z;\n"
			"gl_Position=vpm*(lp+wp);\n"			
		"}\n"
		;

	const char * particleFragmentSource = 
		"#version 130\n"
		"uniform sampler2D col;\n"
		"varying vec2 tc;\n"
		"varying vec4 c;\n"
		"void main(){\n"
			"gl_FragColor=c*texture2D(col,tc);\n"
		"}\n"
		;

	BuildShader( particleVertexSource, particleFragmentSource, mParticleVertexShader, mParticleFragmentShader, mParticleProgram );

	const char * ambientFragmentSource = 
		"#version 130\n"
		"uniform sampler2D col;\n"
		"varying vec2 tc;\n"
		"varying vec4 c;\n"
		"uniform vec4 amb;\n"
		"void main(){\n"
		"vec4 dif=texture2D(col,tc);\n"
		"gl_FragColor=(amb+vec4(dif.a))*dif;\n"
		"}\n"
		;

	BuildShader( quadVertexSource, ambientFragmentSource, mAmbientLightVertexShader, mAmbientLightFragmentShader, mAmbientLightProgram );

	// get uniform locations
	unform_gbuf_worldMat = gl.GetUniformLocationARB( mGBufferProgram, "wm" );
	uniform_gbuf_color = gl.GetUniformLocationARB( mGBufferProgram, "c" );
	
	uniform_dep = gl.GetUniformLocationARB( mLightProgram, "dep" );
	uniform_col = gl.GetUniformLocationARB( mLightProgram, "col" );
	uniform_nor = gl.GetUniformLocationARB( mLightProgram, "nor" );
	uniform_invViewProj = gl.GetUniformLocationARB( mLightProgram, "invViewProj" );
	uniform_lp = gl.GetUniformLocationARB( mLightProgram, "lp" );
	uniform_lc = gl.GetUniformLocationARB( mLightProgram, "lc" );
	uniform_lv = gl.GetUniformLocationARB( mLightProgram, "lv" );

	puniform_wm = gl.GetUniformLocationARB( mParticleProgram, "wm" );
	puniform_vpm = gl.GetUniformLocationARB( mParticleProgram, "vpm" );
	puniform_camR = gl.GetUniformLocationARB( mParticleProgram, "camR" );
	puniform_camU = gl.GetUniformLocationARB( mParticleProgram, "camU" );
}

void Engine::DrawQuad( float x, float y, float w, float h ) {
	glBegin( GL_QUADS );

	glColor4ub( 255, 255, 255, 255 );

	glTexCoord2f( 0.0f, 0.0f );
	glVertex2f( x, y + h );

	glTexCoord2f( 1.0f, 0.0f );
	glVertex2f( x + w, y + h );

	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f( x + w, y );

	glTexCoord2f( 0.0f, 1.0f );
	glVertex2f( x, y );

	GLCall( glEnd());
}

void Engine::SetCameraProjection() {
	GLCall( glMatrixMode( GL_PROJECTION ));
	GLCall( glLoadMatrixf( Camera::msCurrentCamera->mProjectionMatrix.f ));
}

void Engine::SetOrthoProjection() {
	GLCall( glMatrixMode( GL_PROJECTION ));
	GLCall( glLoadIdentity() );
	GLCall( glOrtho( 0, mGBufferWidth, mGBufferHeight, 0, -1.0f, 1.0f ));

	GLCall( glMatrixMode( GL_MODELVIEW ));
	GLCall( glLoadIdentity() );
}

void Engine::DrawLightSphere( Light * pLight ) {
	mSphere->SetLocalPosition( pLight->mGlobalTransform.GetOrigin() );
	mSphere->SetLocalScale( Math::Vector3( pLight->mRadius * 1.2f, pLight->mRadius * 1.2f, pLight->mRadius * 1.2f ));
	mSphere->mInRenderChain = true;
	mSphere->Render( Camera::msCurrentCamera );
	mSphere->mInRenderChain = false;
}

void Engine::RenderWorld() {
	if( Camera::msCurrentCamera ) {
		RECT clientRect;
		GetClientRect( mWindow, &clientRect ) ;
		glViewport( clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top );

		// Begin rendering into framebuffer
		GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, mGBufferFrameBuffer ));
		GLCall( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ));

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
		GLCall( gl.DrawBuffersARB( 3, buffers ));

		GLCall( gl.UseProgramObjectARB( mGBufferProgram ));

		GLCall( glEnable( GL_CULL_FACE ));
		GLCall( glCullFace( GL_BACK ));

		GLuint uniform_alb = gl.GetUniformLocationARB( mGBufferProgram, "alb" );
		// Fill G-Buffer
		if( Camera::msCurrentCamera ) {
			Camera::msCurrentCamera->Update();

			GLCall( glMatrixMode( GL_MODELVIEW ));

			SceneNode * node = SceneNode::msRoot;
			while( node ) {
				if( node->mType == SceneNode::Type::Base ) {	
					if( node->IsVisible() ) {
						GLCall( gl.Uniform4fARB( uniform_alb, node->mAlbedo, 0.0f, 0.0f, 0.0f ));
						GLCall( gl.UniformMatrix4fvARB( unform_gbuf_worldMat, 1, GL_FALSE, node->mGlobalTransform.f ));
						GLCall( gl.Uniform4fARB( uniform_gbuf_color, node->mColor.x, node->mColor.y, node->mColor.z, 1.0f ));
						node->Render( Camera::msCurrentCamera );	
					}
				}
				node = node->mNext;
			}
		}

		// Render Lights
		GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, mLightFrameBuffer ));

		GLenum lightBuffers[] = { GL_COLOR_ATTACHMENT0_EXT };
		GLCall( gl.DrawBuffersARB( 1, lightBuffers ));

		GLCall( glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ));


		GLCall( glDisable( GL_BLEND ));

		GLCall( glDepthMask( GL_FALSE ));

		GLCall( glDisable( GL_STENCIL_TEST ));

		GLCall( glStencilMask( 0xFF ));

		SetOrthoProjection();

		GLCall( gl.UseProgramObjectARB( mAmbientLightProgram ));

		GLint uniform_dcol = gl.GetUniformLocationARB( mAmbientLightProgram, "col" );
		GLCall( gl.ActiveTextureARB( GL_TEXTURE1_ARB ));
		GLCall( glEnable(GL_TEXTURE_2D ));
		GLCall( glBindTexture( GL_TEXTURE_2D, mColorTex ));
		GLCall( gl.Uniform1iARB( uniform_dcol, 1 )); 
		
		GLint uniform_amb = gl.GetUniformLocationARB( mAmbientLightProgram, "amb" );
		GLCall( gl.Uniform4fARB( uniform_amb, gAmbientColor.x, gAmbientColor.y, gAmbientColor.z, 1.0f ));

		DrawQuad( 0, 0, mGBufferWidth, mGBufferHeight );

		GLCall( gl.UseProgramObjectARB( mLightProgram ));

		GLCall( glEnable( GL_BLEND ));
		GLCall( glBlendFunc( GL_ONE, GL_ONE ));
		

		GLCall( gl.ActiveTextureARB( GL_TEXTURE0_ARB ));
		GLCall( glEnable( GL_TEXTURE_2D ));
		GLCall( glBindTexture( GL_TEXTURE_2D, mDepthTex ));
		GLCall( gl.Uniform1iARB( uniform_dep, 0 ));

		GLCall( gl.ActiveTextureARB( GL_TEXTURE1_ARB ));
		GLCall( glEnable(GL_TEXTURE_2D ));
		GLCall( glBindTexture( GL_TEXTURE_2D, mColorTex ));
		GLCall( gl.Uniform1iARB( uniform_col, 1 )); 

		GLCall( gl.ActiveTextureARB( GL_TEXTURE2_ARB ));
		GLCall( glEnable(GL_TEXTURE_2D ));
		GLCall( glBindTexture( GL_TEXTURE_2D, mNormalsTex ));
		GLCall( gl.Uniform1iARB( uniform_nor, 2 ));		

		GLCall( gl.UniformMatrix4fvARB( uniform_invViewProj, 1, GL_FALSE, Camera::msCurrentCamera->mInvViewProjMatrix.f ));

		GLCall( glEnable( GL_STENCIL_TEST ));

		SceneNode * light = SceneNode::msRoot;
		int lightNum = 0;
		while( light ) {
			if( light->mType == SceneNode::Type::Light ) {				
				Light * pLight = static_cast<Light*>( light );

				if( Camera::msCurrentCamera->IsSphereInFrustum( pLight->GetGlobalPosition(), pLight->mRadius )) {				


					Math::Vector3 origin = pLight->mGlobalTransform.GetOrigin();

					SetCameraProjection();

					GLCall( glColorMask( 0, 0, 0, 0 ));

					GLCall( glEnable( GL_CULL_FACE ));

					GLCall( glCullFace( GL_FRONT ));
					GLCall( glStencilFunc( GL_ALWAYS, 0, 0xFF ));
					GLCall( glStencilOp( GL_KEEP, GL_INCR, GL_KEEP ));
					DrawLightSphere( pLight );

					GLCall( glCullFace( GL_BACK ));
					GLCall( glStencilFunc( GL_ALWAYS, 0, 0xFF ));
					GLCall( glStencilOp( GL_KEEP, GL_DECR, GL_KEEP ));
					DrawLightSphere( pLight );

					GLCall( glStencilFunc( GL_NOTEQUAL, 0, 0xFF ));
					GLCall( glStencilOp( GL_KEEP, GL_KEEP, GL_ZERO ));

					GLCall( glColorMask( 1, 1, 1, 1 ));

					GLCall( glDisable( GL_CULL_FACE ));

					SetOrthoProjection();			

					GLCall( gl.Uniform4fARB( uniform_lp, origin.x, origin.y, origin.z, pLight->mRadius * pLight->mRadiusCoeff ));
					GLCall( gl.Uniform4fARB( uniform_lc, pLight->mColor.x, pLight->mColor.y, pLight->mColor.z, 1 ));
					GLCall( gl.Uniform4fARB( uniform_lv, pLight->mSpotDirection.x, pLight->mSpotDirection.y, pLight->mSpotDirection.z, pLight->mCosHalfAngle ));

					DrawQuad( 0, 0, mGBufferWidth, mGBufferHeight );
				}

				lightNum++;
			}
			light = light->mNext;
		}		

		GLCall( glDisable( GL_STENCIL_TEST ));

		GLCall( glDisable( GL_BLEND ));

		GLCall( glDepthMask( GL_TRUE ));

		GLCall( gl.ActiveTextureARB( GL_TEXTURE0_ARB ));
		GLCall( glBindTexture( GL_TEXTURE_2D, 0 ));

		GLCall( gl.ActiveTextureARB( GL_TEXTURE1_ARB ));
		GLCall( glBindTexture( GL_TEXTURE_2D, 0 ));

		GLCall( gl.ActiveTextureARB (GL_TEXTURE2_ARB ));
		GLCall( glBindTexture( GL_TEXTURE_2D, 0 ));

		// Particle systems
		Camera::msCurrentCamera->Update();

		SetCameraProjection();

		GLCall( gl.UseProgramObjectARB( mParticleProgram ));



		Math::Matrix iv = Camera::msCurrentCamera->mViewMatrix.Inverse();
		Math::Vector3 camU = iv.GetUp();
		Math::Vector3 camR = iv.GetRight();

		gl.Uniform4fARB( puniform_camU, camU.x, camU.y, camU.z, 0.0f );
		gl.Uniform4fARB( puniform_camR, camR.x, camR.y, camR.z, 0.0f );
		gl.UniformMatrix4fvARB( puniform_vpm, 1, GL_FALSE, Camera::msCurrentCamera->mViewProjectionMatrix.f );

		GLCall( glDepthMask( GL_FALSE ));
		GLCall( glEnable( GL_BLEND ));
		GLCall( glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA ));
		GLCall( glDisable( GL_CULL_FACE ));

		SceneNode * node = SceneNode::msRoot;
		while( node ) {
			auto next = node->mNext;
			if( node->mType == SceneNode::Type::ParticleSystem ) {					
				ParticleSystem * ps = static_cast<ParticleSystem*>( node );

				if( Camera::msCurrentCamera->IsSphereInFrustum( ps->GetGlobalPosition(), ps->mRadius )) {				
					gl.UniformMatrix4fvARB( puniform_wm, 1, GL_FALSE, ps->mGlobalTransform.f );

					ps->Render();	
					if( ps->mAliveParticles <= 0 ) {					
						delete ps;
					}
				}
			} 
			node = next;
		}	
		GLCall( glDisable( GL_BLEND ));
		GLCall( glDepthMask( GL_TRUE ));
	}

	// Render final frame
	SetOrthoProjection();

	GLCall( gl.BindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ));
	GLCall( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ));

	GLCall( gl.UseProgramObjectARB( mQuadProgram ));

	GLCall( gl.ActiveTextureARB( GL_TEXTURE0_ARB ));
	GLCall( glBindTexture( GL_TEXTURE_2D, mFrameTex ));

	DrawQuad( 0, 0, mGBufferWidth, mGBufferHeight );

	GLCall( gl.ActiveTextureARB( GL_TEXTURE0_ARB ));
	GLCall( glBindTexture( GL_TEXTURE_2D, 0 ));

	// GUI
	Texture::msLibrary[(int)Texture::Type::White]->Bind();
	SetOrthoProjection();
	
	GLCall( gl.UseProgramObjectARB( 0 ));

	auto guiNode = GUINode::msRoot;
	while( guiNode ) {
		if( guiNode->IsVisible() ) {
			guiNode->Render();
		}
		guiNode = guiNode->mNext;
	}

	SwapBuffers( mDeviceContext );
}

void Engine::Update() {
	SceneNode * node = SceneNode::msRoot;
	while( node ) {
		if( node->mType == SceneNode::Type::Light ) {				
			static_cast<Light*>( node )->Update();
		} else if( node->mType == SceneNode::Type::ParticleSystem ) {
			static_cast<ParticleSystem*>( node )->Update();
		}
		node = node->mNext;
	}
}

