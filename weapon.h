#pragma once

#include "projectile.h"
#include "light.h"
#include "collision.h"

class Weapon {
public:
	enum class Type {
		Generic,
		Minigun,
		RocketLauncher,
		GrenadeLauncher,
		Shotgun,
		PlasmaGun,
		RailGun
	};

	Collision::Body * mOwnerBody;
	Weapon( Type type, Collision::Body * owner );
	~Weapon();

	Type mType;
	SceneNode * mNode;
	SceneNode * mEnergyContainer;
	int mBulletCount;
	int mShootTimer;
	int mShootInterval;
	Light * mLight;
	float mAngle;
	float mTurnSpeed;
	bool mShooting;
	bool mUseWiggle;
	char * mName;
	Math::Vector3 mShotOffset;
	Math::Vector3 mShotOffsetTo;
	Math::Vector3 mShakeOffset;
	Projectile::Type mProjectileType;

	void Shoot( class Actor * owner );
	void Update();
};