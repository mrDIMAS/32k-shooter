#include "precompiled.h"
#include "mathlib.h"
#include <Windows.h>

namespace Math {

static unsigned long int gRandNext = 1;

float Sin( float x ) {
	double result ;

	__asm {
		fld x
		fsin
		fstp result
	}

	return result;
}

float Cos( float x ) {
	double result;

	__asm {
		fld x
		fcos
		fstp result
	};

	return result;
}

int Rand(void) {
	gRandNext = gRandNext * 1103515245 + 12345;
	return (unsigned int)(gRandNext/65536) % 32768;
}

void SeedRand(unsigned int seed) {
	gRandNext = seed;
}


FPtr_Atan2 Atan2;
FPtr_Sqrt Sqrt;
FPtr_Asin Asin;
FPtr_Fmod Fmod;
FPtr_log10 Log10;
FPtr_floor Floor;
FPtr_exp Exp;
FPtr_atan Atan;

void LoadFunctions() {
	HINSTANCE crt = LoadLibraryA( "msvcrt.dll" );
	Atan2 = (FPtr_Atan2)GetProcAddress( crt, "atan2" );
	Sqrt = (FPtr_Sqrt)GetProcAddress( crt, "sqrt" );
	Asin = (FPtr_Asin)GetProcAddress( crt, "asin" );
	Fmod = (FPtr_Fmod)GetProcAddress( crt, "fmod" );
	Log10 = (FPtr_log10)GetProcAddress( crt, "log10" );
	Floor = (FPtr_floor)GetProcAddress( crt, "floor" );
	Exp = (FPtr_exp)GetProcAddress( crt, "exp" );
	Atan = (FPtr_atan)GetProcAddress( crt, "atan" );
}

int RandNormal() {
	return (Rand() + Rand() + Rand() + Rand() + Rand() + Rand()) / 6;
}

float RandFloat( float min, float max ) {
	return (Rand() / (float)RandMax) * ( max - min ) + min;
}

float FAbs( float f ) {
	return f < 0 ? -f : f;
}



Quaternion::Quaternion( ) : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 1.0f ) { };

Quaternion::Quaternion( float x, float y, float z, float w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
};

void VMinMax( Vector3 & vMin, Vector3 & vMax, const Vector3 & a )
{
	if( a.x < vMin.x ) vMin.x = a.x;
	if( a.y < vMin.y ) vMin.y = a.y;
	if( a.z < vMin.z ) vMin.z = a.z;

	if( a.x > vMax.x ) vMax.x = a.x;
	if( a.y > vMax.y ) vMax.y = a.y;
	if( a.z > vMax.z ) vMax.z = a.z;
}

Quaternion operator *  (const Quaternion& q1, const Quaternion & q2 ) {
	 return Quaternion(  q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z  );
}

Quaternion::Quaternion( const Vector3 & axis, float angle ) {
	angle *= Pi / 180.0f;

	float d = axis.Length();
	float s = Sin( angle * 0.5f ) / d;
	x = axis.x * s;
	y = axis.y * s;
	z = axis.z * s;
	w = Cos( angle * 0.5f );
};

Quaternion::Quaternion( const Matrix & m ) {
	Matrix mat = m.Transpose();
	float trace = mat.f[0] + mat.f[5] + mat.f[10];
	if( trace > 0 ) {
		float s = 0.5f / Math::Sqrt( trace+ 1.0f);
		w = 0.25f / s;
		x = ( mat.f[9] - mat.f[6] ) * s; 
		y = ( mat.f[2] - mat.f[8] ) * s;
		z = ( mat.f[4] - mat.f[1] ) * s;
	} else {
		if ( mat.f[0] > mat.f[5] && mat.f[0] > mat.f[10] ) {
			float s = 2.0f * Math::Sqrt( 1.0f + mat.f[0] - mat.f[5] - mat.f[10] );
			w = (mat.f[9] - mat.f[6] ) / s;
			x = 0.25f * s;
			y = (mat.f[1] + mat.f[4] ) / s;
			z = (mat.f[2] + mat.f[8] ) / s;
		} else if (mat.f[5] > mat.f[10]) {
			float s = 2.0f * Math::Sqrt( 1.0f + mat.f[5] - mat.f[0] - mat.f[10]);
			w = (mat.f[2] - mat.f[8] ) / s;
			x = (mat.f[1] + mat.f[4] ) / s;
			y = 0.25f * s;
			z = (mat.f[6] + mat.f[9] ) / s;
		} else {
			float s = 2.0f * Math::Sqrt( 1.0f + mat.f[10] - mat.f[0] - mat.f[5] );
			w = (mat.f[4] - mat.f[1] ) / s;
			x = (mat.f[2] + mat.f[8] ) / s;
			y = (mat.f[6] + mat.f[9] ) / s;
			z = 0.25f * s;
		}
	}
}

Matrix::Matrix() {
	Identity();
}

void Matrix::Identity() {
	for( int i = 0; i < 16; i++ ) {
		f[i] = 0;
	}

	f[0] = 1.0f;
	f[5] = 1.0f;
	f[10] = 1.0f;
	f[15] = 1.0f;
}

Math::Vector3 Matrix::GetOrigin() const
{
	return Vector3( f[12], f[13], f[14] );
}

Math::Matrix Matrix::Transpose() const {
	Matrix out;
	for(int n = 0; n < 16; n++) {
		int i = n/4;
		int j = n%4;
		out.f[n] = f[ 4*j + i];
	}
	return out;
}



Math::Vector3 Matrix::GetLook() const
{
	return Vector3( f[8], f[9], f[10] );
}

Math::Vector3 Matrix::GetUp() const
{
	return Vector3( f[4], f[5], f[6] );
}

Math::Vector3 Matrix::GetRight() const
{
	return Vector3( f[0], f[1], f[2] );
}

void Matrix::SetRotationOrigin( Quaternion q, Vector3 v ) {
	float s = 2.0f / ( q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w );
	float xs = q.x * s,   ys = q.y * s,   zs = q.z * s;
	float wx = q.w * xs,  wy = q.w * ys,  wz = q.w * zs;
	float xx = q.x * xs,  xy = q.x * ys,  xz = q.x * zs;
	float yy = q.y * ys,  yz = q.y * zs,  zz = q.z * zs;
	f[0] = 1.0f - (yy + zz);
	f[1] = xy + wz;
	f[2] = xz - wy;
	f[3] = 0.0f;
	f[4] = xy - wz;
	f[5] = 1.0f - (xx + zz);
	f[6] = yz + wx;
	f[7] = 0.0f;
	f[8] = xz + wy;
	f[9] = yz - wx;
	f[10] = 1.0f - (xx + yy);
	f[11] = 0.0f;
	f[12] = v.x;
	f[13] = v.y;
	f[14] = v.z;
	f[15] = 1.0f; 
}

void Matrix::Scale( Vector3 v ) {
	for( int i = 0; i < 16; i++ ) {
		f[i] = 0;
	}

	f[0] = v.x;
	f[5] = v.y;
	f[10] = v.z;
	f[15] = 1.0f;
}

Matrix Matrix::operator * ( Matrix b ) {
	Matrix temp( false );

	temp.f[0] = f[0] * b.f[0] + f[1] * b.f[4] + f[2] * b.f[8] + f[3] * b.f[12]; 
	temp.f[1] = f[0] * b.f[1] + f[1] * b.f[5] + f[2] * b.f[9] + f[3] * b.f[13]; 
	temp.f[2] = f[0] * b.f[2] + f[1] * b.f[6] + f[2] * b.f[10] + f[3] * b.f[14]; 
	temp.f[3] = f[0] * b.f[3] + f[1] * b.f[7] + f[2] * b.f[11] + f[3] * b.f[15]; 
	temp.f[4] = f[4] * b.f[0] + f[5] * b.f[4] + f[6] * b.f[8] + f[7] * b.f[12]; 
	temp.f[5] = f[4] * b.f[1] + f[5] * b.f[5] + f[6] * b.f[9] + f[7] * b.f[13]; 
	temp.f[6] = f[4] * b.f[2] + f[5] * b.f[6] + f[6] * b.f[10] + f[7] * b.f[14]; 
	temp.f[7] = f[4] * b.f[3] + f[5] * b.f[7] + f[6] * b.f[11] + f[7] * b.f[15]; 
	temp.f[8] = f[8] * b.f[0] + f[9] * b.f[4] + f[10] * b.f[8] + f[11] * b.f[12]; 
	temp.f[9] = f[8] * b.f[1] + f[9] * b.f[5] + f[10] * b.f[9] + f[11] * b.f[13]; 
	temp.f[10] = f[8] * b.f[2] + f[9] * b.f[6] + f[10] * b.f[10] + f[11] * b.f[14]; 
	temp.f[11] = f[8] * b.f[3] + f[9] * b.f[7] + f[10] * b.f[11] + f[11] * b.f[15]; 
	temp.f[12] = f[12] * b.f[0] + f[13] * b.f[4] + f[14] * b.f[8] + f[15] * b.f[12]; 
	temp.f[13] = f[12] * b.f[1] + f[13] * b.f[5] + f[14] * b.f[9] + f[15] * b.f[13]; 
	temp.f[14] = f[12] * b.f[2] + f[13] * b.f[6] + f[14] * b.f[10] + f[15] * b.f[14]; 
	temp.f[15] = f[12] * b.f[3] + f[13] * b.f[7] + f[14] * b.f[11] + f[15] * b.f[15]; 	

	return temp;
}

void Matrix::PerspectiveFov( float fov, float aspect, float zNear, float zFar ) {
	float t = Sin( fov ) / Cos( fov );
	float yMax = zNear * t;
	float xMax = yMax * aspect;
	Frustum( -xMax, xMax, -yMax, yMax, zNear, zFar );
}

void Matrix::Frustum( float left, float right, float bottom, float top, float zNear, float zFar ) {
	float temp = 2.0f * zNear;
	float temp2 = right - left;
	float temp3 = top - bottom;
	float temp4 = zFar - zNear;
	f[0] = temp / temp2;
	f[1] = 0.0f;
	f[2] = 0.0f;
	f[3] = 0.0f;
	f[4] = 0.0f;
	f[5] = temp / temp3;
	f[6] = 0.0f;
	f[7] = 0.0f;
	f[8] = ( right + left ) / temp2;
	f[9] = ( top + bottom ) / temp3;
	f[10] = (-zFar - zNear ) / temp4;
	f[11] = -1.0f;
	f[12] = 0.0f;
	f[13] = 0.0f;
	f[14] = ( -temp * zFar ) / temp4;
	f[15] = 0.0f;
}

void Matrix::Ortho2D( float left, float right, float bottom, float top, float zNear, float zFar ) {
	f[0] = 2.0f / ( right - left );
	f[1] = 0.0f;
	f[2] = 0.0f;
	f[3] = 0.0f;
	f[4] = 0.0f;
	f[5] = 2.0f / ( top - bottom );
	f[6] = 0.0f;
	f[7] = 0.0f;
	f[8] = 0.0f;
	f[9] = 0.0f;
	f[10] = 1.0f / ( zFar - zNear );
	f[11] = 0.0f;
	f[12] = ( left + right ) / ( left - right );
	f[13] = ( top + bottom ) / ( bottom - top );
	f[14] = zNear / ( zNear - zFar );
	f[15] = 1.0f;
}

void Matrix::Translation( Vector3 v ) {
	for( int i = 0; i < 16; i++ ) {
		f[i] = 0;
	}
	f[0] = 1.0f;
	f[5] = 1.0f;
	f[10] = 1.0f;
	f[12] = v.x;
	f[13] = v.y;
	f[14] = v.z;
	f[15] = 1.0f;
}

void Matrix::LookAt( Vector3 eye, Vector3 look, Vector3 up ) {
	Identity();

	Vector3 forwVec = ( look - eye ).Normalize();    
	Vector3 rightVec = forwVec.Cross( up ).Normalize();
	Vector3 upNorm = rightVec.Cross( forwVec ).Normalize();

	f[0] = rightVec.x;
	f[4] = rightVec.y;
	f[8] = rightVec.z;

	f[1] = upNorm.x;
	f[5] = upNorm.y;
	f[9] = upNorm.z;

	f[2] = -forwVec.x;
	f[6] = -forwVec.y;
	f[10] = -forwVec.z;

	Matrix translation;
	translation.Translation( -eye );

	*this = translation * *this;
}

Math::Vector3 Matrix::TransformVector( const Vector3 & vec ) {
	return Vector3( vec.x * f[0] + vec.y * f[4] + vec.z * f[8]  + f[12],
		vec.x * f[1] + vec.y * f[5] + vec.z * f[9]  + f[13],
		vec.x * f[2] + vec.y * f[6] + vec.z * f[10] + f[14] );
}

Math::Matrix Matrix::Inverse() const {
	Matrix out;
	const float* a = f;
	float* r = out.f;
	r[0]  =  a[5]*a[10]*a[15] - a[5]*a[14]*a[11] - a[6]*a[9]*a[15] + a[6]*a[13]*a[11] + a[7]*a[9]*a[14] - a[7]*a[13]*a[10];
	r[1]  = -a[1]*a[10]*a[15] + a[1]*a[14]*a[11] + a[2]*a[9]*a[15] - a[2]*a[13]*a[11] - a[3]*a[9]*a[14] + a[3]*a[13]*a[10];
	r[2]  =  a[1]*a[6]*a[15]  - a[1]*a[14]*a[7]  - a[2]*a[5]*a[15] + a[2]*a[13]*a[7]  + a[3]*a[5]*a[14] - a[3]*a[13]*a[6];
	r[3]  = -a[1]*a[6]*a[11]  + a[1]*a[10]*a[7]  + a[2]*a[5]*a[11] - a[2]*a[9]*a[7]   - a[3]*a[5]*a[10] + a[3]*a[9]*a[6];
	r[4]  = -a[4]*a[10]*a[15] + a[4]*a[14]*a[11] + a[6]*a[8]*a[15] - a[6]*a[12]*a[11] - a[7]*a[8]*a[14] + a[7]*a[12]*a[10];
	r[5]  =  a[0]*a[10]*a[15] - a[0]*a[14]*a[11] - a[2]*a[8]*a[15] + a[2]*a[12]*a[11] + a[3]*a[8]*a[14] - a[3]*a[12]*a[10];
	r[6]  = -a[0]*a[6]*a[15]  + a[0]*a[14]*a[7]  + a[2]*a[4]*a[15] - a[2]*a[12]*a[7]  - a[3]*a[4]*a[14] + a[3]*a[12]*a[6];
	r[7]  =  a[0]*a[6]*a[11]  - a[0]*a[10]*a[7]  - a[2]*a[4]*a[11] + a[2]*a[8]*a[7]   + a[3]*a[4]*a[10] - a[3]*a[8]*a[6];
	r[8]  =  a[4]*a[9]*a[15]  - a[4]*a[13]*a[11] - a[5]*a[8]*a[15] + a[5]*a[12]*a[11] + a[7]*a[8]*a[13] - a[7]*a[12]*a[9];
	r[9]  = -a[0]*a[9]*a[15]  + a[0]*a[13]*a[11] + a[1]*a[8]*a[15] - a[1]*a[12]*a[11] - a[3]*a[8]*a[13] + a[3]*a[12]*a[9];
	r[10] =  a[0]*a[5]*a[15]  - a[0]*a[13]*a[7]  - a[1]*a[4]*a[15] + a[1]*a[12]*a[7]  + a[3]*a[4]*a[13] - a[3]*a[12]*a[5];
	r[11] = -a[0]*a[5]*a[11]  + a[0]*a[9]*a[7]   + a[1]*a[4]*a[11] - a[1]*a[8]*a[7]   - a[3]*a[4]*a[9]  + a[3]*a[8]*a[5];
	r[12] = -a[4]*a[9]*a[14]  + a[4]*a[13]*a[10] + a[5]*a[8]*a[14] - a[5]*a[12]*a[10] - a[6]*a[8]*a[13] + a[6]*a[12]*a[9];
	r[13] =  a[0]*a[9]*a[14]  - a[0]*a[13]*a[10] - a[1]*a[8]*a[14] + a[1]*a[12]*a[10] + a[2]*a[8]*a[13] - a[2]*a[12]*a[9];
	r[14] = -a[0]*a[5]*a[14]  + a[0]*a[13]*a[6]  + a[1]*a[4]*a[14] - a[1]*a[12]*a[6]  - a[2]*a[4]*a[13] + a[2]*a[12]*a[5];
	r[15] =  a[0]*a[5]*a[10]  - a[0]*a[9]*a[6]   - a[1]*a[4]*a[10] + a[1]*a[8]*a[6]   + a[2]*a[4]*a[9]  - a[2]*a[8]*a[5];
	float det = a[0]*r[0] + a[4]*r[1] + a[8]*r[2] + a[12]*r[3];
	if( Math::FAbs( det ) > 0.00001f ) {
		det = 1.0f / det;
	}
	for( int i = 0; i < 16; i++ ) {
		r[i] *= det;
	}
	return out;
}

Ray::Ray( ) {

}

Ray::Ray( const Math::Vector3 & a, const Math::Vector3 & b ) : mBegin( a ), mEnd( b ), mDir( b - a ) {
	
}


Plane::Plane( const Math::Vector3 & normal, float distance ) : mNormal( normal ), mDistance( distance ) {

}

bool Plane::Intersects( const Ray & ray, Math::Vector3 & intPoint ) {
	// solve plane equation 
	float u = -( ray.mBegin.Dot( mNormal ) + mDistance );
	float v = ray.mDir.Dot( mNormal );
	float t = u / v;

	// ray miss 
	if( t < 0.0f ) {
		return false;
	}

	// find intersection point 
	intPoint = ray.mBegin + ray.mDir * t;

	return true;
}

float Plane::Distance( const Math::Vector3 & point ) {
	return Math::FAbs( mNormal.Dot( point ) + mDistance );
}



void Vector3::operator*=( const Vector3 & v )
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
}

void Vector3::operator*=( float a )
{
	x *= a;
	y *= a;
	z *= a;
}

Math::Vector3 Vector3::operator*( float a ) const
{
	return Vector3( x * a, y * a, z * a );
}

float Vector3::SqrDistance( const Vector3 & v ) const
{
	return (*this - v).SqrLength();
}

Math::Vector3 Vector3::operator/( const Vector3 & v ) const
{
	return Vector3( x / v.x, y / v.y, z / v.z );
}

Math::Vector3 Vector3::operator*( const Vector3 & v ) const
{
	return Vector3( x * v.x, y * v.y, z * v.z );
}

Math::Vector3 Vector3::operator-( const Vector3 & v ) const
{
	return Vector3( x - v.x, y - v.y, z - v.z );
}

void Vector3::operator-=( const Vector3 & v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

Math::Vector3 Vector3::operator-()
{
	return Vector3( -x, -y, -z );
}

void Vector3::operator/=( const Vector3 & v )
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
}

float Vector3::SqrLength() const
{
	return x * x + y * y + z * z;
}

Math::Vector3 Vector3::Random( const Math::Vector3 & a, const Math::Vector3 & b )
{
	return Math::Vector3 ( 
		Math::RandFloat( a.x, b.x ),
		Math::RandFloat( a.y, b.y ),
		Math::RandFloat( a.z, b.z )
		);
}

Math::Vector3 Vector3::Randomize( Math::Vector3 rCoeff ) const
{
	Math::Vector3 out( *this );
	return out * rCoeff;
}

void Vector3::operator+=( const Vector3 & v )
{
	x += v.x;
	y += v.y;
	z += v.z;
}


Math::Vector3 Vector3::operator+( const Vector3 & v ) const
{
	return Vector3( x + v.x, y + v.y, z + v.z );
}

float Vector3::Length() const
{
	return Sqrt( x * x + y * y + z * z );
}

Math::Vector3 Vector3::Normalize()
{
	float length = 1.0f / Sqrt( x * x + y * y + z * z );
	if( length > Epsilon ) {
		x *= length;
		y *= length;
		z *= length;
	}
	return *this;
}

Math::Vector3 Vector3::Normalized() const
{
	Vector3 out( *this );
	out.Normalize();
	return out;
}

Math::Vector3 Vector3::NormalizeEx( float & outLength )
{
	outLength = Sqrt( x * x + y * y + z * z );
	x /= outLength;
	y /= outLength;
	z /= outLength;
	return *this;
}

Math::Vector3 Vector3::Cross( const Vector3 & v ) const
{
	return Vector3( 
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x 
		);
}


float Vector3::Dot( const Vector3 & v ) const
{
	return x * v.x + y * v.y + z * v.z;
}

bool Vector3::Equals( const Vector3 & v ) const
{
	return FAbs(x - v.x) <= EpsilonRough && FAbs(y - v.y) <= EpsilonRough && FAbs(z - v.z) <= EpsilonRough;
}

Math::Vector3 Vector3::Reflect( const Math::Vector3 & v, const Math::Vector3 & n )
{
	return v - n * 2.0f * v.Dot( n );
}

};