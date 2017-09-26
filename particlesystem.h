#pragma once

#include "scenenode.h"

class Particle {
public:
	Math::Vector3 mPosition;
	Math::Vector3 mSpeed;
	int mLifeTime;
	Math::Color mColor;
	float mSize;

	Particle();
};

class ParticleSystem : public SceneNode {
public:
	Particle * mParticles;
	int mParticleCount;
	int mAliveParticles;
	Math::Vector3 mEmitDirectionMax;
	Math::Vector3 mEmitDirectionMin;
	float mRadius;
	float mParticleSize;
	bool mAutoResurrection;
	bool mLifeTimeAlpha;
	int mParticleLifeTime;
	Math::Color mColor;

	explicit ParticleSystem( int particleCount, float radius );
	virtual ~ParticleSystem();
	void Render();
	void Update();
	void Resurrect( Particle & p );
	void ResurrectParticles();

	static ParticleSystem * CreateFire( float length );
	static ParticleSystem * CreateTrail( float radius, float length );
};
