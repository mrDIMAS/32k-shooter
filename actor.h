#pragma once

#include "collision.h"
#include "scenenode.h"
#include "cstring.h"
#include "weapon.h"
#include "flag.h"
#include "Pathfinder.h"




class Actor {
public:
	MakeList( Actor );

	SceneNode * mPivot;
	Collision::Body * mBody;
	Math::Vector3 mDestVelocity;
	static int msCount;

	float mHealth;
	float mArmor;
	float mMaxHealth;
	float mMaxArmor;
	float mSpeed;
	int mFrags;
	int mDeaths;
	Team mTeam;
	bool mDead;
	bool mFlying;
	Flag * mFlag;
	bool mIsPlayer;
	char mName[256];
	Array<Weapon*> mWeapons;
	int mCurrentWeapon;

	void Jump( float height = 0.075f );

	Actor();
	virtual ~Actor();
	void Move( const Math::Vector3 & direction, bool fly = false );	
	virtual void Damage( float howMuch, Actor * attacker );
	void Update();
	void TryGrabFlag( Flag * f );
	void ThrowFlag();
	virtual void Resurrect( );
	virtual void Think();
	virtual void Explode();
	virtual void AddWeapon( Weapon * weapon );
	virtual void SetTeamColors();
	virtual void OnMove();
	void Clear();
	static void UpdateAll();
	static void DeleteAll();
	static int Count();
	static void SelectTeams();
};