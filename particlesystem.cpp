#include "precompiled.h"
#include "particlesystem.h"
#include "camera.h"

ParticleSystem::ParticleSystem( int particleCount, float radius ) : 
	mParticleSize( 0.04f ), mParticleCount( particleCount ), mAliveParticles( mParticleCount ), mRadius( radius ),
	mColor( 204, 157, 132, 255 ), mEmitDirectionMin( Math::Vector3( -0.05, 0.1, -0.05 )), mParticleLifeTime( 200 ),
	mEmitDirectionMax( Math::Vector3( 0.05, 0.3, 0.05 )), mAutoResurrection( false ), mLifeTimeAlpha( false )
{
	mType = SceneNode::Type::ParticleSystem;
	
	mParticles = new Particle[mParticleCount];

	ResurrectParticles();

	AddSurface( new Surface( particleCount * 2 ));
	mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RoundParticle];
}

ParticleSystem::~ParticleSystem() {
	delete mParticles;
}

void ParticleSystem::Render() {
	mSurfaces[0]->Render();
}

void ParticleSystem::ResurrectParticles() {
	for( int i = 0; i < mParticleCount; i++ ) {
		Resurrect( mParticles[i] );
	}
}

ParticleSystem * ParticleSystem::CreateFire( float length ) {
	ParticleSystem * fire = new ParticleSystem( 80, 0.1f );
	fire->mAutoResurrection = true;
	fire->mEmitDirectionMin = Math::Vector3( -0.001f, -0.001f, -0.01f );
	fire->mEmitDirectionMax = Math::Vector3( 0.001f, 0.001f, -0.05f );
	fire->mColor = Math::Color( 255, 127, 32, 255 );
	fire->mRadius = length;
	fire->ResurrectParticles();
	return fire;
}

ParticleSystem * ParticleSystem::CreateTrail( float radius, float length ) {
	const int pCount = 400;
	ParticleSystem * trail = new ParticleSystem( pCount, 0.1f );
	trail->mAutoResurrection = false;
	trail->mEmitDirectionMin = Math::Vector3( 0.0f, 0.0002f, 0.0f );
	trail->mEmitDirectionMax = Math::Vector3( 0.0f, 0.0008f, 0.0f );
	trail->mColor = Math::Color( 0, 255, 0, 255 );
	trail->mRadius = length;
	trail->mParticleSize = 0.05f;
	trail->mLifeTimeAlpha = true;
	trail->mParticleLifeTime = 70;
	trail->ResurrectParticles();
	float zStep = length / pCount; 
	float z = 0.0f;
	for( int i = 0; i < trail->mParticleCount; ++i ) {
		trail->mParticles[i].mPosition = Math::Vector3( radius * Math::Cos( i ), radius * Math::Sin( i ), z );
		z += zStep;
	}
	return trail;
}

void ParticleSystem::Resurrect( Particle & p ) {
	p.mSpeed = Math::Vector3::Random( mEmitDirectionMin, mEmitDirectionMax );
	p.mPosition = Math::Vector3( 0.0f, 0.0f, 0.0f );
	p.mLifeTime = mParticleLifeTime;
	p.mSize = mParticleSize;
	p.mColor = mColor;
}

void ParticleSystem::Update() {
	mAliveParticles = 0;

	float r2 = mRadius * mRadius;

	for( int i = 0; i < mParticleCount; i++ ) {
		Particle & p = mParticles[i];

		p.mPosition += p.mSpeed;

		float d2 = p.mPosition.SqrLength();

		if( d2 > r2 ) {
			if( mAutoResurrection ) {
				Resurrect( p );
			} else {
				p.mLifeTime = 0;
			}
		}

		if( mLifeTimeAlpha ) {
			float lt = p.mLifeTime >= 0 ? p.mLifeTime : 0.0f;
			float t = lt / 200.0f;
			p.mColor.mAlpha = 255.0f * t;
		} else {
			p.mColor.mAlpha = 255.0f * (1.0f - d2 / r2);
		}

		--p.mLifeTime;

		if( p.mLifeTime <= 0 ) {
			if( mAutoResurrection ) {
				Resurrect( p );
			}
		} else {
			Vertex * v = &mSurfaces[0]->mVertices[ mAliveParticles * 6 ];

			v->position = p.mPosition;
			v->texCoord.x = 0.0f;
			v->texCoord.y = 0.0f;
			v->normal.x = -1.0f;
			v->normal.y = -1.0f;
			v->normal.z = p.mSize;
			v->color = p.mColor;

			++v;
			v->position = p.mPosition;
			v->texCoord.x = 0.0f;
			v->texCoord.y = 1.0f;
			v->normal.x = -1.0f;
			v->normal.y = 1.0f;
			v->normal.z = p.mSize;
			v->color = p.mColor;

			++v;
			v->position = p.mPosition;
			v->texCoord.x = 1.0f;
			v->texCoord.y = 1.0f;
			v->normal.x = 1.0f;
			v->normal.y = 1.0f;
			v->normal.z = p.mSize;
			v->color = p.mColor;

			++v;
			v->position = p.mPosition;
			v->texCoord.x = 0.0f;
			v->texCoord.y = 0.0f;
			v->normal.x = -1.0f;
			v->normal.y = -1.0f;
			v->normal.z = p.mSize;
			v->color = p.mColor;

			++v;
			v->position = p.mPosition;
			v->texCoord.x = 1.0f;
			v->texCoord.y = 1.0f;
			v->normal.x = 1.0f;
			v->normal.y = 1.0f;
			v->normal.z = p.mSize;
			v->color = p.mColor;

			++v;
			v->position = p.mPosition;
			v->texCoord.x = 1.0f;
			v->texCoord.y = 0.0f;
			v->normal.x = 1.0f;
			v->normal.y = -1.0f;
			v->normal.z = p.mSize;
			v->color = p.mColor;

			++mAliveParticles;
		}
	}	

	mSurfaces[0]->mNeedUpdate = true;
	mSurfaces[0]->mVertexCount = mAliveParticles * 3;
}

Particle::Particle() : mPosition( 0.0f, 0.0f, 0.0f ), mColor( 204, 157, 132, 255 ), mLifeTime( 200 ), mSize( 0.04f )
{

}
