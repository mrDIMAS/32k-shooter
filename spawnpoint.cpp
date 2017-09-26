#include "precompiled.h"
#include "spawnpoint.h"
#include "bot_rover.h"
#include "bot_fly.h"
#include "player.h"

SpawnPoint * SpawnPoint::msRoot;

SpawnPoint::SpawnPoint( const Math::Vector3 & position, Team team ) : mTeam( team ), mFloater( 0.0f ), mNext( nullptr ), mPrev( nullptr ), mSpawnInterval( 400 ), mSpawnTimer( 0 )
{
	mNode = new SceneNode;
	mNode->SetLocalPosition( position );

	SceneNode * p = new SceneNode;
	p->AddSurface( Surface::MakeCylinder( 10, 0.6f, 0.2f ));
	p->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
	p->AttachTo( mNode );
	p->SetLocalRotation( Math::Quaternion( Math::Vector3( 1, 0, 0 ), 90.0f ));

	SceneNode * pad = new SceneNode;
	pad->AddSurface( Surface::MakeCylinder( 10, 0.2f, 0.3f ));
	pad->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
	pad->AttachTo( mNode );
	pad->SetLocalRotation( Math::Quaternion( Math::Vector3( 1, 0, 0 ), 90.0f ));

	mLightBulb = new SceneNode;
	mLightBulb->AttachTo( mNode );
	mLightBulb->AddSurface( Surface::MakeSphere( 6, 6, 0.1f ));
	mLightBulb->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::RustyMetal];

	
	Math::Vector3 color;

	bool teamGameType = gGameType == GameType::CaptureTheFlag || gGameType == GameType::TeamDeathmatch;

	if( teamGameType ) {
		if( mTeam == Team::Blue ) {
			color = Math::Vector3( 0.0f, 0.0f, 1.0f );
		} 
		if( mTeam == Team::Red ) {
			color = Math::Vector3( 1.0f, 0.0f, 0.0f );
		} 
	} else {
		color = Math::Vector3( 0.0f, 1.0f, 0.0f );
	}

	Light * light = new Light( 4, color ); 
	light->AttachTo( mLightBulb );

	LinkedList::Append( msRoot, this );

	Bot * bot;
	if( Math::RandNormal() & 1 ) {
		bot = new Rover;
	} else {
		bot = new Fly;
	}
	bot->mBody->mPosition = mNode->GetGlobalPosition() + Math::Vector3( 0.0f, 1.0f, 0.0f );	
	if( teamGameType ) {
		bot->mTeam = mTeam;
		bot->SetTeamColors();
	}
}

SpawnPoint::~SpawnPoint() {
	LinkedList::Extract( msRoot, this );
}

void SpawnPoint::Update() {
	mFloater += 0.1f;

	mLightBulb->SetLocalPosition( Math::Vector3( 0, 1.6f, 0.0f ) * ( 1.0f + 0.1f * Math::Sin( mFloater )));

	mSpawnTimer--;

	bool teamGameType = gGameType == GameType::CaptureTheFlag || gGameType == GameType::TeamDeathmatch;

	if( mSpawnTimer <= 0 ) {
		// Look for dead actors and resurrect it
		auto actor = Actor::msRoot;
		while( actor ) {
			auto next = actor->mNext;		
			if( actor->mHealth <= 0 ) {
				if( teamGameType && actor->mTeam == mTeam || !teamGameType ) {
					actor->Resurrect();
					actor->mBody->mLinearVelocity = Math::Vector3( 0.0f, 0.0f, 0.0f );
					actor->mBody->mPosition = mNode->GetGlobalPosition() + Math::Vector3( 0.0f, 2.5f, 0.0f );		
					break;
				}				
			}
			actor = next;
		}

		mSpawnTimer = mSpawnInterval;		
	}
}

void SpawnPoint::UpdateAll() {
	auto sp = msRoot;
	while( sp ) {
		sp->Update();
		sp = sp->mNext;
	}
}
