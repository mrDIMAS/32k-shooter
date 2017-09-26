#pragma once

#include "bot.h"
#include "particlesystem.h"

class Fly : public Bot {
public:
	SceneNode * mBodyModel;
	ParticleSystem * mEngineFire;
	ParticleSystem * mThrustEngines[2];
	Fly();
	virtual void Explode();
	virtual void Resurrect();
	virtual void SetTeamColors();
};