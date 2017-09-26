#pragma once

#include "light.h"

class Flag {
	
public:
	static Flag * msRed;
	static Flag * msBlue;

	enum class Type {
		None,
		Red,
		Blue
	};

	Math::Vector3 mPosition;

	Type mType;
	float mAngle;
	bool mGrabbed;
	SceneNode * mNode;

	Flag( Type type, const Math::Vector3 & position );

	bool IsReturned() {
		return mPosition == mNode->GetGlobalPosition();
	}
	void Return();
	void Update();

	static void UpdateAll();
	static void DeleteAll() {
		delete msRed;
		delete msBlue;
		msRed = nullptr;
		msBlue = nullptr;
	}
};