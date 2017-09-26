#pragma once

#include "scenenode.h"

class Item {
public:
	MakeList( Item );

	enum class Type {
		SmallHealth,
		MediumHealth,
		LargeHealth,
		YellowArmor,
		AmmoBelt,
		Rockets,
		Grenades,
		Rails,
		Plasma,
		Shells
	};

	SceneNode * mPivot;
	SceneNode * mNode;

	Type mType;

	char * mDesc;

	float mAngle;
	int mSpawnTimer;
	int mSpawnInterval;
	int mContent;

	explicit Item( Type type );
	~Item();
	void Update();
	static void UpdateAll();
	static void DeleteAll();
	bool CanPickup() {
		return mSpawnTimer <= 0;
	}
};