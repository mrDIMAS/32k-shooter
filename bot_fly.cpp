#include "precompiled.h"
#include "bot_fly.h"
#include "player.h"

Fly::Fly() {
	mBody = new Collision::Body( Collision::Body::Type::Sphere );
	mBody->mRadius = 0.35f;
	mBody->mGravity = Math::Vector3( 0.0f, 0.0f, 0.0f );

	mFlying = true;

	mSpeed = 0.07f;

	Resurrect();
}

void Fly::Explode() {
	mEngineFire->mRadius = 0.05f;
	mThrustEngines[0]->mRadius = 0.05f;
	mThrustEngines[1]->mRadius = 0.05f;
	Bot::Explode();
}

void Fly::Resurrect() {
	Bot::Resurrect();

	mBodyModel = new SceneNode;
	mBodyModel->AttachTo( mPivot );
	mBodyModel->AddSurface( Surface::MakeCube( 0.2, 0.2, 0.4 ));

	SceneNode * tower = new SceneNode;
	tower->AttachTo( mPivot );
	tower->AddSurface( Surface::MakeCylinder( 10, 0.07, 0.2f ));
	tower->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0f ), -90.0f ));
	tower->SetLocalPosition( Math::Vector3( 0, 0.06, -0.05 ));
	tower->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];

	mEngineFire = ParticleSystem::CreateFire( 0.6f );
	mEngineFire->AttachTo( tower );

	mThrustEngines[0] = ParticleSystem::CreateFire( 0.45f );
	mThrustEngines[0]->AttachTo( mPivot );
	mThrustEngines[0]->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), 45.0f ));

	mThrustEngines[1] = ParticleSystem::CreateFire( 0.45f );
	mThrustEngines[1]->AttachTo( mPivot );
	mThrustEngines[1]->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), -45.0f ));

	SceneNode * nozzle1 = new SceneNode;
	nozzle1->AddSurface( Surface::MakeCone( 10, 0.06, 0.24 ));
	nozzle1->AttachTo( mPivot );
	nozzle1->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), -135.0f ));
	nozzle1->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];

	SceneNode * nozzle2 = new SceneNode;
	nozzle2->AddSurface( Surface::MakeCone( 10, 0.06, 0.24 ));
	nozzle2->AttachTo( mPivot );
	nozzle2->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), 135.0f ));
	nozzle2->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];

	SceneNode * nozzle3 = new SceneNode;
	nozzle3->AddSurface( Surface::MakeCone( 10, 0.06, 0.2 ));
	nozzle3->AttachTo( mPivot );
	nozzle3->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0f ), 90 ));
	nozzle3->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];

	for( int i = 0; i < mWeapons.mCount; ++i ) {
		mWeapons[i]->mNode->AttachTo( mPivot );
		mWeapons[i]->mNode->SetLocalPosition( Math::Vector3( 0, 0.2f, 0.0f ));
		mWeapons[i]->mUseWiggle = false;
	}
}

void Fly::SetTeamColors()
{
	mBodyModel->mColor = mTeam == Team::Blue ? Math::Vector3( 0.0f, 0.0f, 1.0f ) : Math::Vector3( 1.0f, 0.0f, 0.0f );
}

