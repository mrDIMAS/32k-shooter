#pragma once

#include "scenenode.h"

class Light : public SceneNode {
public:
	float mRadius;
	float mRadiusCoeff;
	float mCounter;
	float mMultiplier;
	bool mSpot;
	float mCosHalfAngle;
	Math::Vector3 mSpotDirection;

	Light( float radius, const Math::Vector3 & color );
	virtual ~Light();
	void Update();
};