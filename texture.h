#pragma once

#include "perlinnoise.h"




struct RGBAPixel {
	UByte r, g, b, a;

	RGBAPixel () : r( 255 ), g( 255 ), b( 255 ), a( 255 ) {	};
	RGBAPixel( UByte _r, UByte _g, UByte _b, UByte _a ) : r( _r ), g( _g ), b( _b ), a( _a ) { };
	RGBAPixel operator * ( float t ) {
		return RGBAPixel( r * t, g * t, b * t, a );
	}
};

class Texture {
public:
	enum class Type : int {
		Mud = 0,
		Metal,
		RustyMetal,
		White,
		Yellow,
		RoundParticle,
		Grass,
		Trunk,
		Sky,
		Space,
		Count
	};

	GLuint mID;

	Texture( Type type );
	~Texture();	
	void Bind();
	void DrawVerticalLine( RGBAPixel * pixels, int w, int x, int y1, int y2, const RGBAPixel & color );
	void DrawHorizontalLine( RGBAPixel * pixels, int h, int y, int x1, int x2, const RGBAPixel & color );

	static void CreateLibrary();
	static Texture * msLibrary[Type::Count];
	static void FreeLibrary();
};