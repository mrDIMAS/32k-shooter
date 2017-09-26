#pragma once

#include "bot.h"


class Rover : public Bot {
public:
	SceneNode * mBodyModel;
	static const int WheelCount = 6;
	SceneNode * mWheels[WheelCount];
	float mWheelAngle;

	Rover();
	~Rover();
	virtual void Resurrect();
	virtual void SetTeamColors();

	virtual void OnMove();
};