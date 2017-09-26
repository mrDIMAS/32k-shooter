#pragma once

#include "scenenode.h"
#include "collision.h"
#include "actor.h"
#include "weapon.h"

class Bot : public Actor {
public:
	Actor * mTarget;
	Actor * mPreciousAlly;
	Pathfinder mPathfinder;
	int mPathTimer;
	Array<GraphVertex*> mPath;
	int mCurrentPathVertex;
	bool mPathfinderInitialized;
	float mYaw;
	float mYawTo;

	explicit Bot();
	virtual ~Bot();
	virtual void Think();
	virtual void Explode();
	static int Count();	
	virtual void LookAt( const Math::Vector3 & lookAt );
	virtual void SelectTarget();
	virtual void Damage( float howMuch, Actor * attacker );
	virtual void Resurrect();
	int CanUseWeapon( Weapon::Type type );
	virtual bool SelectWeapon();
	virtual bool IsSeeTarget();
};
