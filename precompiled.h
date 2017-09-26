#pragma once

#pragma warning( disable:4244 )


#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "mathlib.h"
#include <glext.h>
#include "time.h"
#include <initguid.h>
#include <dsound.h>

extern "C" void * __cdecl memset(void *, int, size_t);
#pragma intrinsic(memset)

struct PackedVertex {
	short x, y, z;
};

struct PackedTexCoord {
	short x, y;
};

struct Vertex {
	Math::Vector3 position;
	Math::Vector3 normal;
	Math::Vector2 texCoord;
	Math::Color color;
};

extern Math::Vector2 gGUIGlobalScale;

typedef int (*FPtr_sprintf)( char * buffer, const char * format, ... );

#ifdef _DEBUG


extern FPtr_sprintf Sprintf;

#define GLCall( glFunc ) glFunc
#define GLCheckFunc( glExt )
/*
#define GLCall( glFunc )							 \
	glFunc;											 \
	if( glGetError() != GL_NO_ERROR ) {				 \
		char buf[2048];								 \
		Sprintf( buf, "Error in %s on %d line", __FILE__, __LINE__ ); \
		MessageBoxA( 0, buf, 0, MB_ICONERROR );          \
	};												 \


#define GLCheckFunc( glExt )						\
	if( !glExt )	{								\
		MessageBoxA( 0, #glExt, 0, MB_ICONERROR );	\
		ExitProcess( 1 );							\
	};												\
	*/
#else

#define GLCall( glFunc ) glFunc
#define GLCheckFunc( glExt )

#endif

struct Face {
	unsigned char a, b, c, ta, tb, tc;
};

#define ArraySize(x) (sizeof(x)/sizeof(x[0]))

extern float gAspect;
extern bool gRunning;

extern Math::Vector3 gAmbientColor;
#define FixedPointFloatPackAccuracy 0.05f

static inline float ExpandToFloat( short input ) {
	return input * FixedPointFloatPackAccuracy;
}

extern int gAllocatedMemoryBlocks;

void * operator new( size_t size );
void operator delete( void * ptr );
void * operator new[](size_t size);
void operator delete[](void *ptr);
void * Realloc( void * ptr, size_t newSize );

struct Mouse {
	int x, y;
	int lastX, lastY;
	int xSpeed, ySpeed;
	int wheelSpeed;
	bool mFree;
	bool buttonPressed[5];
	bool buttonReleased[5];
};

extern Mouse gMouse;
extern class Engine * gEngine;
extern class Menu * gMenu;

enum class GameType : int {
	None, // When game is not started
	Deathmatch,
	TeamDeathmatch,
	CaptureTheFlag
};

enum class Difficulty : int {
	Easy		= 0,
	Medium		= 1,
	Hard		= 2,
	Nightmare	= 3,
	Count
};

// Rules
void TotalCleanup();
extern GameType gGameType;
extern int gBlueTeamScore;
extern int gRedTeamScore;
extern int gGameTime;
extern int gFlagLimit;
extern int gFragLimit;
extern bool gRoundOver;
extern Difficulty gDifficulty;


extern float gFOV;
extern float gMouseSens;
extern bool gInverseMouse;


extern class SceneNode * gSkySphere;
struct GLFuncList {
	PFNGLGENBUFFERSARBPROC GenBuffersARB;
	PFNGLBINDBUFFERARBPROC BindBufferARB;
	PFNGLDELETEBUFFERSARBPROC DeleteBuffersARB;
	PFNGLBUFFERDATAARBPROC BufferDataARB;

	PFNGLACTIVETEXTUREARBPROC ActiveTextureARB;
	PFNGLCLIENTACTIVETEXTUREARBPROC ClientActiveTextureARB;

	PFNGLCREATEPROGRAMOBJECTARBPROC CreateProgramObjectARB;
	PFNGLCREATESHADEROBJECTARBPROC CreateShaderObjectARB;
	PFNGLSHADERSOURCEARBPROC ShaderSourceARB;
	PFNGLCOMPILESHADERARBPROC CompileShaderARB;
	PFNGLGETINFOLOGARBPROC GetInfoLogARB;
	PFNGLATTACHOBJECTARBPROC AttachObjectARB;
	PFNGLLINKPROGRAMARBPROC LinkProgramARB;
	PFNGLUSEPROGRAMOBJECTARBPROC UseProgramObjectARB;
	PFNGLDELETEOBJECTARBPROC DeleteObjectARB;
	PFNGLGETOBJECTPARAMETERIVARBPROC GetObjectParameterivARB;
	PFNGLGETUNIFORMLOCATIONARBPROC GetUniformLocationARB;
	PFNGLUNIFORM4FARBPROC Uniform4fARB;
	PFNGLUNIFORMMATRIX4FVARBPROC UniformMatrix4fvARB;
	PFNGLUNIFORM1IARBPROC Uniform1iARB;

	PFNGLGENFRAMEBUFFERSEXTPROC GenFramebuffersEXT;
	PFNGLBINDFRAMEBUFFEREXTPROC BindFramebufferEXT;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC FramebufferTexture2DEXT;
	PFNGLDELETEFRAMEBUFFERSEXTPROC DeleteFramebuffersEXT;

	PFNGLGENRENDERBUFFERSEXTPROC GenRenderbuffersEXT;
	PFNGLBINDRENDERBUFFEREXTPROC BindRenderbufferEXT;
	PFNGLRENDERBUFFERSTORAGEEXTPROC RenderbufferStorageEXT;
	PFNGLDELETERENDERBUFFERSEXTPROC DeleteRenderbuffersEXT;

	PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC FramebufferRenderbufferEXT;

	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC CheckFramebufferStatusEXT;
	
	PFNGLDRAWBUFFERSARBPROC DrawBuffersARB;
};

extern GLFuncList gl;


typedef unsigned char UByte;

namespace LinkedList {

#define MakeList(T)	   	\
	static T * msRoot;	\
	T * mNext;			\
	T * mPrev;			\

template<class T>
void Append( T & root, T element ) {
	if( root ) {
		T last = root;
		while( last->mNext ) {
			last = last->mNext;
		}
		last->mNext = element;
		element->mPrev = last;
	} else {
		root = element;
	}
}

template<class T>
void Extract( T & root, T element ) {
	if( element->mNext ) {
		element->mNext->mPrev = element->mPrev;
	}
	if( element->mPrev ) {
		element->mPrev->mNext = element->mNext;
	}
	if( root == element ) {
		root = element->mNext;
	}
}

};

namespace Memory {
	typedef void * (*FPtr_memcpy)( void * str, const void * src, size_t n );
	typedef void * (*FPtr_memmove) ( void * dst, const void * src, size_t n );

	extern FPtr_memcpy Copy;
	extern FPtr_memmove Move;

	void LoadFunctions();
}



template< class T >
class Array {
public:
	T * mElements;
	int mCount;

	Array( ) : mElements( nullptr ), mCount( 0 ) {

	}

	~Array() {
		delete mElements;
	}

	void Append( T element ) {
		++mCount;
		mElements = (T*)Realloc( mElements, mCount * sizeof( T ));
		mElements[ mCount - 1 ] = element;
	}

	int Remove( T element ) {			
		for( int i = 0; i < mCount; ++i ) {
			if( mElements[ i ] == element ) {
				--mCount;
				for( int j = i; j < mCount; ++j ) {
					mElements[ j ] = mElements[ j + 1 ];
				}
				mElements = (T*)Realloc( mElements, mCount * sizeof( T ));
				return i;
			}
		}
		return -1;
	}

	void Reverse() {
		for( int i = 0; i < mCount / 2; ++i ) {
			T temp = mElements[ mCount - i - 1 ];
			mElements[ mCount - i - 1 ] = mElements[i];
			mElements[i] = temp;
		}
	}

	void Clear() {
		delete mElements;
		mCount = 0;
		mElements = nullptr;
	}


	T operator [] ( int n ) {
		return mElements[n];
	}
};

enum class Team {
	Red,
	Blue
};