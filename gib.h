#pragma once

#include "scenenode.h"
#include "collision.h"

class Gib {
public:
	MakeList( Gib );

	SceneNode * mModel;
	Collision::Body * mBody;
	int mLifeTime;
	int mMaxLifeTime;
	Math::Vector3 mDisappearOffset;


	Gib( SceneNode * model, float size );
	~Gib();
	void Update();
	static void UpdateAll();
	static void DeleteAll() {
		while( msRoot ) {
			delete msRoot;
		}
	}
};