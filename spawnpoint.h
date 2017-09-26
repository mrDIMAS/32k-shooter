#pragma once

#include "scenenode.h"
#include "light.h"

class SpawnPoint {
public:
	MakeList( SpawnPoint );

	SceneNode * mNode;
	SceneNode * mLightBulb;
	float mFloater;
	int mSpawnInterval;
	int mSpawnTimer;
	Team mTeam;

	explicit SpawnPoint( const Math::Vector3 & position, Team team );
	~SpawnPoint();
	void Update();
	static void UpdateAll();
	static void DeleteAll()
	{
		while( msRoot ) {
			delete msRoot;
		}
	}
};