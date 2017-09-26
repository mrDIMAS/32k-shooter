#pragma once

#include "scenenode.h"
#include "collision.h"

class Projectile {
public:
	MakeList( Projectile );

	enum class Type {
		Bullet,
		ShotgunBullet,
		Rocket,
		Grenade,
		Plasma,
		Rail,
	};

	class Actor * mOwner;
	SceneNode * mNode;
	Collision::Body * mPhysicalBody;
	Type mType;
	float mDamage;
	float mFlightSpeed;
	bool mSplashDamage;
	int mLifeTime;
	bool mDestroyed;
	// special case for shotgun and other weapons shooting pellets
	int mRayCount;
	Sound * mExplosionSound;

	explicit Projectile( Type type, const Math::Vector3 & origin, const Math::Matrix & orientation, Actor * owner );
	virtual ~Projectile();
	void Update( );
	void DoSplashDamage();
	void DoContactDamage();
	void Explode();
	float PushBody( Collision::Body * body );
	static void UpdateAll();
	static void DeleteAll();
};