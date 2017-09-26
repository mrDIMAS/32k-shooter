#include "precompiled.h"
#include "perlinnoise.h"
#include "mathlib.h"

namespace PerlinNoise {
	float FNoise2D(int x, int y) {
		int n = x + y * 57;
		n = (n<<13) ^ n;
		return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) /
			1073741824.0f);
	}

	float SmoothedNoise2D(float x, float y) {
		float corners = ( FNoise2D(x - 1.0f, y - 1.0f) + FNoise2D(x + 1.0f, y - 1.0f) + FNoise2D(x - 1.0f, y + 1.0f)+FNoise2D( x + 1.0f, y + 1.0f) ) / 16.0f;
		float sides = ( FNoise2D(x - 1.0f, y) + FNoise2D(x + 1.0f, y) + FNoise2D(x, y - 1.0f)  +FNoise2D(x, y + 1.0f) ) / 8.0f;
		float center = FNoise2D(x, y) / 4.0f;
		return corners + sides + center;
	}

	float CosineInterpolate( float y1, float y2, float mu) {
		float mu2 = (1-Math::Cos(mu*Math::Pi))/2;
		return( y1 * ( 1- mu2 )+y2*mu2);
	}

	float CompileNoise(float x, float y) {
		float int_X = int(x);
		float fractional_X = x - int_X;

		float int_Y = int(y);
		float fractional_Y = y - int_Y;

		float v1 = SmoothedNoise2D(int_X,     int_Y);
		float v2 = SmoothedNoise2D(int_X + 1, int_Y);
		float v3 = SmoothedNoise2D(int_X,     int_Y + 1);
		float v4 = SmoothedNoise2D(int_X + 1, int_Y + 1);

		float i1 = CosineInterpolate(v1 , v2 , fractional_X);
		float i2 = CosineInterpolate(v3 , v4 , fractional_X);

		return CosineInterpolate(i1 , i2 , fractional_Y);
	}

	float Noise2D(float x,float y,float factor)	{
		float total = 0;
		float persistence=0.5f;
		float frequency = 0.75f;
		float amplitude=1;

		x+= factor;
		y+= factor;

		const int numOctaves = 2;

		for(int i=0;i<numOctaves;i++) {
			total += CompileNoise(x*frequency, y*frequency) * amplitude;
			amplitude *= persistence;
			frequency*=2;
		}

		return total < 0 ? -total : total;
	}

}