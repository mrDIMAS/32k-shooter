#pragma once

#pragma warning( disable:4244 )

namespace Math {

const float Pi = 3.1415926535897932384626433832795;
const float PiOver2 = 1.5707963267948966192313216916398;
const float Epsilon = 0.0001f;
const float EpsilonRough = 0.0015f;
const unsigned int RandMax = 32767;

float Sin( float x );
float Cos( float x );
int Rand();
int RandNormal();
float RandFloat( float min, float max );
void SeedRand(unsigned int seed);
float FAbs( float f );
static inline float Squared(float v) {
	return v * v;
}


typedef double (*FPtr_Atan2)( double y, double x );
typedef double (*FPtr_Sqrt)( double x );
typedef double (*FPtr_Asin)( double x );
typedef double (*FPtr_Fmod)( double x, double y );
typedef double (*FPtr_log10) (double x);
typedef double (*FPtr_floor) (double x);
typedef double (*FPtr_exp) (double x);
typedef double (*FPtr_atan) (double x);

extern FPtr_Atan2 Atan2;
extern FPtr_Sqrt Sqrt;
extern FPtr_Asin Asin;
extern FPtr_Fmod Fmod;
extern FPtr_log10 Log10;
extern FPtr_floor Floor;
extern FPtr_exp Exp;
extern FPtr_atan Atan;

void LoadFunctions();

class Vector3 {
public:
	float x, y, z;

	Vector3() : x( 0.0f ), y( 0.0f ), z( 0.0f ) { }

	Vector3( float _x, float _y, float _z ) : x( _x ), y( _y ), z( _z ) { }

	void operator=( const Vector3 & v )
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Vector3 operator*( float a ) const;
	float SqrDistance( const Vector3 & v ) const;
	float SqrLength() const;
	static Vector3 Random( const Math::Vector3 & a, const Math::Vector3 & b );
	static Math::Vector3 Vector3::RandomUnit()
	{
		return Math::Vector3 ( 
			Math::RandFloat( -1.0f, 1.0f ),
			Math::RandFloat( -1.0f, 1.0f ),
			Math::RandFloat( -1.0f, 1.0f )
			);
	}
	Vector3 Randomize( Math::Vector3 rCoeff ) const;
	Vector3 operator+( const Vector3 & v ) const;
	Vector3 operator-();
	void operator/=( const Vector3 & v );
	void operator-=( const Vector3 & v );
	void operator+=( const Vector3 & v );
	void operator*=( float a );
	void operator*=( const Vector3 & v );
	Vector3 operator/( const Vector3 & v ) const;
	Vector3 operator*( const Vector3 & v ) const;
	Vector3 operator-( const Vector3 & v ) const;
	float Length () const;
	Vector3 Normalize();
	Vector3 Normalized() const;
	Vector3 NormalizeEx( float & outLength );
	Vector3 Cross( const Vector3 & v ) const;
	float Dot( const Vector3 & v ) const;
	bool Equals( const Vector3 & v ) const;
	static Vector3 Reflect( const Math::Vector3 & v, const Math::Vector3 & n );
	bool IsNaN() const {
		return x != x || y != y || z != z;
	}
	bool operator == ( const Math::Vector3 & v ) {
		return FAbs( x - v.x ) < EpsilonRough && FAbs( y - v.y ) < EpsilonRough && FAbs( z - v.z ) < EpsilonRough;
	}
};

void VMinMax( Math::Vector3 & vMin, Math::Vector3 & vMax, const Math::Vector3 & a );

class Vector2 {
public:
	float x, y;

	Vector2::Vector2( float _x, float _y ) : x( _x ), y( _y ) {	}
	Vector2::Vector2( ) : x( 0.0f ), y( 0.0f ) { }
	void operator += ( const Math::Vector2 & other ) {
		x += other.x;
		y += other.y;
	}
	Vector2 operator + ( const Math::Vector2 & other ) {
		return Vector2( x + other.x, y + other.y );
	}
	Vector2 operator - ( const Math::Vector2 & other ) {
		return Vector2( x - other.x, y - other.y );
	}
	Vector2 operator / ( float a ) {
		return Vector2( x / a, y / a );
	}
	Vector2 operator * ( float a ) {
		return Vector2( x * a, y * a );
	}

};

static inline Vector2 operator * ( const Vector2 & v, const Vector2 & a ) {
	return Vector2( v.x * a.x, v.y * a.y );
}

class Color {
public:
	typedef unsigned char UByte;
	UByte mRed, mGreen, mBlue, mAlpha;
	Color( UByte r, UByte g, UByte b, UByte a ) : mRed( r ), mGreen( g ), mBlue( b ), mAlpha( a ) { }
	Color() : mRed( 255 ), mGreen( 255 ), mBlue( 255 ), mAlpha( 255 ) {	}
};

class Quaternion {
public:
	float x;
	float y;
	float z;
	float w;

	Quaternion( );
	Quaternion( float x, float y, float z, float w );
	Quaternion( const Vector3 & axis, float angle );
	Quaternion( const class Matrix & m );
};

Quaternion operator *  (const Quaternion& q1, const Quaternion & q2 );

class Matrix {
public:
	float f[16];

	Matrix();
	Matrix( bool unused ) {

	}
	void Identity();
	Vector3 GetOrigin() const;
	Vector3 GetLook() const;
	Vector3 GetUp() const;
	Vector3 GetRight() const;
	void SetRotationOrigin( Quaternion q, Vector3 v );
	void Scale( Vector3 v );
	Matrix operator * ( Matrix b );
	void PerspectiveFov( float fov, float aspect, float zNear, float zFar );
	void Frustum( float left, float right, float bottom, float top, float zNear, float zFar );
	void Ortho2D( float left, float right, float bottom, float top, float zNear, float zFar );
	void Translation( Vector3 v );
	void LookAt( Vector3 eye, Vector3 look, Vector3 up );
	Vector3 TransformVector( const Vector3 & vec );
	Matrix Inverse() const;
	Matrix Transpose() const;
};

class Ray {
public:
	Math::Vector3 mBegin;
	Math::Vector3 mEnd;
	Math::Vector3 mDir;

	Ray( );
	Ray( const Math::Vector3 & a, const Math::Vector3 & b );
};

class Plane {
public:
	Math::Vector3 mNormal;
	float mDistance;

	Plane( const Math::Vector3 & normal, float distance );
	bool Intersects( const Ray & ray, Math::Vector3 & intPoint );
	float Distance( const Math::Vector3 & point );
};

};