#include "precompiled.h"
#include "texture.h"

Texture * Texture::msLibrary[Type::Count];

void Texture::Bind() {
	GLCall( gl.ActiveTextureARB( GL_TEXTURE0_ARB ));
	GLCall( glBindTexture( GL_TEXTURE_2D, mID ));
}

Texture::~Texture() {
	GLCall( glDeleteTextures( 1, &mID ));
}

void Texture::CreateLibrary() {
	for( int i = 0; i < (int)Type::Count; i++ ) {
		new Texture( (Type)i );
	}
}

void Texture::FreeLibrary()
{
	for( int i = 0; i < (int)Type::Count; i++ ) {
		delete msLibrary[i];
	}
}

Texture::Texture( Type type ) {
	int size = 256;
	RGBAPixel * pixels = new RGBAPixel[ size * size ];

	for( int i = 0; i < size; i++ ) {
		for( int j = 0; j < size; j++ ) {
			RGBAPixel pixel;
			if( type == Type::Mud ) {
				float t = PerlinNoise::Noise2D( i, j , 3 );
				if( t < 0.3 ) {
					t = 0.3;
				}
				pixel = RGBAPixel( 204, 157, 132, 255 ) * t;
			} else if( type == Type::Trunk ) {
				float t = PerlinNoise::Noise2D( i, j , 3 );
				if( t < 0.3 ) {
					t = 0.3;
				}
				pixel = RGBAPixel( 128, 64, 64, 255 ) * t;
			} else if( type == Type::Sky ) {
				float t = PerlinNoise::Noise2D( i, j , 5 );
				Math::Vector3 c = (Math::Vector3( 1, 1, 1 ) + Math::Vector3( 0.4, 0.8, 1 ) * t) * 0.5;
				pixel = RGBAPixel( 255 * c.x, 255 * c.y, 255 * c.z, 255 );
			} else if( type == Type::Space ) {
				float t = PerlinNoise::Noise2D( i, j , 5 );
				pixel = RGBAPixel( 255, 255, 255, 255 ) * t;
			} else if( type == Type::Metal ) {
				float t = Math::RandFloat( 0.8, 1.0f );
				pixel = RGBAPixel( 50, 50, 50, 255 ) * t;
			} else if( type == Type::Grass ) {
				float t = Math::RandFloat( 0.8, 1.0f );
				pixel = RGBAPixel( 10, 100, 10, 255 ) * t;
			} else if( type == Type::RustyMetal ) {
				float t = Math::RandFloat( 0.8, 1.0f );
				pixel = RGBAPixel( 193, 122, 79, 255 ) * t;		
			} else if( type == Type::Yellow ) {
				pixel = RGBAPixel( 255, 255, 0, 255 );		
			} else if( type == Type::RoundParticle ) {
				int dx = (i - size / 2);
				int dy = (j - size / 2);
				float d = Math::Sqrt( dx * dx + dy * dy );
				if( d > (size/2)) {
					d = size/2;
				}
				unsigned char t = 255 * ( 1.0f - d / (float)(size / 2));
				pixel = RGBAPixel( 255, 255, 255, t );
			}
			pixels[ i * size + j ] = pixel;
		}
	}		
	
	msLibrary[(int)type] = this;

	GLCall( glGenTextures( 1, &mID ));

	GLCall( glBindTexture( GL_TEXTURE_2D, mID ));

	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE ));

	GLCall( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels ));

	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ));

	int maxAnisotropy;
	GLCall( glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy ));
	GLCall( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy ));

	delete [] pixels;
}

void Texture::DrawHorizontalLine( RGBAPixel * pixels, int h, int y, int x1, int x2, const RGBAPixel & color ) {
	for( int i = x1; i < x2; i++ ) {
		pixels[ y * h + i ] = color;
	}
}

void Texture::DrawVerticalLine( RGBAPixel * pixels, int w, int x, int y1, int y2, const RGBAPixel & color ) {
	for( int i = y1; i < y2; i++ ) {
		pixels[ w * i + x ] = color;
	}
}
