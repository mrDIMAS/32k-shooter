#pragma once

#include "scenenode.h"
#include "collision.h"
#include "spawnpoint.h"
#include "Pathfinder.h"

const char * GetMapName( int i );
int GetMapCount();
class World {
public:
	Math::Vector3 mScale;
	float mGridStep;
	Array<GraphVertex*> mWayMap;
	
	static World * msCurrent;
	explicit World( int mapNum );
	~World();
};

