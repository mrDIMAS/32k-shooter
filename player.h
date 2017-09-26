#pragma once


#include "camera.h"
#include "gui.h"
#include "actor.h"

class Player : public Actor {
public:
	Camera * mCamera;
	Light * mFlashlight;
	SceneNode * mWeaponPivot;

	float mPitch;
	float mYaw;

	bool mMove;
	bool mRun;

	float mFov;
	float mFovTo;
	float mZoomFov;

	float mCameraShakeCoeff;
	Math::Vector3 mCameraShakeOffset;

	Sound * mFootsteps[4];
	static Player * msCurrent;
	float mPath;

	explicit Player( const Math::Vector3 & spawnPosition );
	~Player();
	void SetPosition( const Math::Vector3 & newPosition );
	virtual void AddWeapon( Weapon * weapon );	
	virtual void Think();
	virtual void Resurrect();
};