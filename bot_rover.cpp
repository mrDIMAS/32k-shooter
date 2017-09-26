#include "precompiled.h"
#include "bot_rover.h"
#include "player.h"
#include "gib.h"
#include "particlesystem.h"

Rover::Rover() : mWheelAngle( 0.0f ) {
	mBody = new Collision::Body( Collision::Body::Type::Sphere );
	mBody->mRadius = 0.3f;

	mSpeed = 0.07f;

	Resurrect();
}

Rover::~Rover() {

}

void Rover::Resurrect() {
	Bot::Resurrect();
	for( int i = 0; i < WheelCount; i++ ) {
		mWheels[i] = new SceneNode;
		mWheels[i]->AddSurface( Surface::MakeSphere( 7, 7, 0.1 ));
		mWheels[i]->AttachTo( mPivot );
		mWheels[i]->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
	}

	mWheels[0]->SetLocalPosition( Math::Vector3( -0.2f, -0.05f, -0.2f ));
	mWheels[1]->SetLocalPosition( Math::Vector3( 0.2f, -0.05f, -0.2f ));
	mWheels[2]->SetLocalPosition( Math::Vector3( 0.2f, -0.05f, 0.2f ));
	mWheels[3]->SetLocalPosition( Math::Vector3( -0.2f, -0.05f, 0.2f ));
	mWheels[4]->SetLocalPosition( Math::Vector3( -0.2f, -0.05f, 0.0f ));
	mWheels[5]->SetLocalPosition( Math::Vector3( 0.2f, -0.05f, 0.0f ));

	mBodyModel = new SceneNode;
	mBodyModel->AddSurface( Surface::MakeCube( 0.3f, 0.2f, 0.4f ));
	mBodyModel->AttachTo( mPivot );
	mBodyModel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];

	SceneNode * tower = new SceneNode;
	tower->AddSurface( Surface::MakeCylinder( 10, 0.1f, 0.25f ));
	tower->AttachTo( mPivot );
	tower->SetLocalPosition( Math::Vector3( 0.0f, 0.25f, -0.1f ));
	tower->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0f ), 90.0f ));
	tower->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];

	SceneNode * cover1 = new SceneNode;
	cover1->AddSurface( Surface::MakeCube( 0.099, 0.01, 0.654 ));
	cover1->SetLocalPosition( Math::Vector3( 0.191, 0.073, 0.0 ));
	cover1->AttachTo( mPivot );
	cover1->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];

	SceneNode * cover1_1 = new SceneNode;
	cover1_1->AddSurface( Surface::MakeCube( 0.099, 0.01, 0.654 ));
	cover1_1->SetLocalPosition( Math::Vector3( 0.279, 0.044, 0.0 ));
	cover1_1->SetLocalRotation( Math::Quaternion( Math::Vector3( 0, 0, 1 ), -35 ));
	cover1_1->AttachTo( mPivot );
	cover1_1->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];

	SceneNode * cover2 = new SceneNode;
	cover2->AddSurface( Surface::MakeCube( 0.099, 0.01, 0.654 ));
	cover2->SetLocalPosition( Math::Vector3( -0.191, 0.073, 0.0 ));
	cover2->AttachTo( mPivot );
	cover2->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];
	SceneNode * cover2_1 = new SceneNode;
	cover2_1->AddSurface( Surface::MakeCube( 0.099, 0.01, 0.654 ));
	cover2_1->SetLocalPosition( Math::Vector3( -0.279, 0.044, 0.0 ));
	cover2_1->SetLocalRotation( Math::Quaternion( Math::Vector3( 0, 0, 1 ), 35 ));
	cover2_1->AttachTo( mPivot );
	cover2_1->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];

	SceneNode * eye1 = new SceneNode;
	eye1->AddSurface( Surface::MakeCube( 0.052, 0.022, 0.01 ));
	eye1->SetLocalPosition( Math::Vector3( 0.095, 0.043, 0.21 ));
	eye1->AttachTo( mPivot );
	eye1->mColor = Math::Vector3( 1, 0, 1 );
	eye1->mAlbedo = 1.0f;

	SceneNode * eye2 = new SceneNode;
	eye2->AddSurface( Surface::MakeCube( 0.052, 0.022, 0.01 ));
	eye2->SetLocalPosition( Math::Vector3( -0.095, 0.043, 0.21 ));
	eye2->AttachTo( mPivot );
	eye2->mColor = Math::Vector3( 1, 0, 1 );
	eye2->mAlbedo = 1.0f;

	SceneNode * gas = new SceneNode;
	gas->AddSurface( Surface::MakeCylinder( 10, 0.025, 0.087 ));
	gas->SetLocalPosition( Math::Vector3( 0.06, -0.062, -0.281 ));
	gas->AttachTo( mPivot );

	ParticleSystem * fire = new ParticleSystem( 80, 0.1f );
	fire->mAutoResurrection = true;
	fire->mEmitDirectionMin = Math::Vector3( -0.001f, 0.001f, -0.01f );
	fire->mEmitDirectionMax = Math::Vector3( 0.001f, 0.006f, -0.05f );
	fire->mColor = Math::Color( 120, 120, 120, 255 );
	fire->mRadius = 0.45;
	fire->ResurrectParticles();
	fire->AttachTo( gas );

	for( int i = 0; i < mWeapons.mCount; ++i ) {
		mWeapons[i]->mNode->AttachTo( mPivot );
		mWeapons[i]->mNode->SetLocalPosition( Math::Vector3( 0, 0.2f, 0.0f ));
		mWeapons[i]->mUseWiggle = false;
	}
}

void Rover::SetTeamColors()
{
	mBodyModel->mColor = mTeam == Team::Blue ? Math::Vector3( 0.0f, 0.0f, 1.0f ) : Math::Vector3( 1.0f, 0.0f, 0.0f );
}

void Rover::OnMove()
{
	for( int i = 0; i < WheelCount; ++i ) {
		mWheels[i]->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0f ), mWheelAngle ));
	}
	mWheelAngle += 3.0f;
}

