#include "precompiled.h"
#include "light.h"

Light::Light( float radius, const Math::Vector3 & color ) : mRadius( radius )
{
	mType = SceneNode::Type::Light;
	mRadiusCoeff = 1.0f;
	mCounter = 0.0f;
	mMultiplier = Math::RandFloat( 0.8f, 1.2f );
	mCosHalfAngle = -1;
	mSpot = false;
	mSpotDirection = Math::Vector3( 0, 0, 1 );
	mColor = color;
}

Light::~Light()
{

}

void Light::Update()
{
	mCounter += mMultiplier;
	float a = mCounter / 4;
	float t = (Math::Sin( a / 2 ) + Math::Cos( a / 4 ) * Math::Sin( a )) / 2;
	mRadiusCoeff = 1.0f + 0.05 * t;
}
