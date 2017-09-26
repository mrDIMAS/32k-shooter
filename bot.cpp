#include "precompiled.h"
#include "bot.h"
#include "player.h"
#include "gib.h"
#include "particlesystem.h"
#include "World.h"
#include "item.h"

Bot::Bot() : mTarget( nullptr ), mPreciousAlly( nullptr ), mPathTimer( 0 ),
	mPathfinderInitialized( false ), mYawTo( 0.0f ), mYaw( 0.0f ), mCurrentPathVertex( 0 ) {	
	if( gDifficulty == Difficulty::Easy ) {
		mArmor = 30;
		mHealth = 30;
		mMaxHealth = 30;
		mMaxArmor = 30;
	}
	if( gDifficulty == Difficulty::Medium ) {
		mArmor = 50;
		mHealth = 50;
		mMaxHealth = 50;
		mMaxArmor = 50;
	}
	if( gDifficulty == Difficulty::Hard ) {
		mArmor = 100;
		mHealth = 100;
		mMaxHealth = 100;
		mMaxArmor = 100;
	}
	if( gDifficulty == Difficulty::Nightmare ) {
		mArmor = 150;
		mHealth = 150;
		mMaxHealth = 150;
		mMaxArmor = 150;
	}
}

void Bot::Think() {
	SelectTarget();

	bool ammoDepleted = SelectWeapon();

	// Select proper target
	Math::Vector3 targetPos;

	if( mHealth < 25 ) {
		float suitableDistance = FLT_MAX;
		auto item = Item::msRoot;
		while( item ) {
			if( item->CanPickup() ) {
				if( item->mType == Item::Type::SmallHealth  ||
					item->mType == Item::Type::MediumHealth ||
					item->mType == Item::Type::LargeHealth ) 
				{
					float d2 = item->mNode->GetGlobalPosition().SqrDistance( mBody->mPosition );
					if( d2 < suitableDistance ) {
						targetPos = item->mNode->GetGlobalPosition();
					}
				}
			}
			item = item->mNext;
		}
	} else {
		if( ammoDepleted ) {
			float suitableDistance = FLT_MAX;
			auto item = Item::msRoot;
			while( item ) {
				if( item->CanPickup() ) {
					if( item->mType == Item::Type::Rockets  ||
						item->mType == Item::Type::Grenades ||
						item->mType == Item::Type::AmmoBelt ) 
					{
						float d2 = item->mNode->GetGlobalPosition().SqrDistance( mBody->mPosition );
						if( d2 < suitableDistance ) {
							targetPos = item->mNode->GetGlobalPosition();
						}
					}
				}
				item = item->mNext;
			}
		} else {
			if( !mPreciousAlly ) {
				if( gGameType == GameType::CaptureTheFlag ) {
					if( !Flag::msBlue->mGrabbed && mTeam == Team::Red ) {
						targetPos = Flag::msBlue->mNode->GetGlobalPosition();				
					}
					if( !Flag::msRed->mGrabbed && mTeam == Team::Blue ) {
						targetPos = Flag::msRed->mNode->GetGlobalPosition();				
					}

					if( mFlag ) {
						// Carry flag to Blue flag
						if( mFlag->mType == Flag::Type::Red && mTeam == Team::Blue ) {
							targetPos = Flag::msBlue->mNode->GetGlobalPosition();					
						} 
						// Carry flag to Red flag
						if( mFlag->mType == Flag::Type::Blue && mTeam == Team::Red ) {
							targetPos = Flag::msRed->mNode->GetGlobalPosition();					
						} 
					}
				} else {
					// Just move to randomly selected enemy
					if( mTarget ) {
						targetPos = mTarget->mBody->mPosition;
					}
				}
			} else {
				// Protect precious ally (with flag) at all costs
				targetPos = mPreciousAlly->mBody->mPosition;
			}
		}
	}

	// build path
	if( mPathTimer <= 0 ) {
		if( !mPathfinderInitialized ) {
			mPathfinder.SetVertices( World::msCurrent->mWayMap );
			mPathfinderInitialized = true;
		}

		// Build path to selected target position
		GraphVertex * endVertex = mPathfinder.GetVertexNearestTo( targetPos );
		GraphVertex * beginVertex = mPathfinder.GetVertexNearestTo( mBody->mPosition );
		mPathfinder.BuildPath( beginVertex, endVertex, mPath );
		
		mCurrentPathVertex = 0;

		// update path every N frames
		mPathTimer = 60; 
	}

	for( int i = 0; i < mWeapons.mCount; ++i ) {
		mWeapons[i]->Update();
		mWeapons[i]->mNode->mVisible = false;		
	}

	if( !mDead ) {
		mWeapons[mCurrentWeapon]->mNode->mVisible = true;
	}

	if( IsSeeTarget() ) {
		mWeapons[mCurrentWeapon]->Shoot( this );
		mWeapons[mCurrentWeapon]->Update();
	} else {
		mTarget = nullptr;
	}
	
	Math::Vector3 delta;
	if( mPath.mCount ) {
		GraphVertex * targetVertex = mPath[ mCurrentPathVertex ];

		// Move along builded path
		if( targetVertex ) {

			delta = targetVertex->mPosition - mBody->mPosition;

			targetPos = targetVertex->mPosition; // <- HAAX

			if( delta.SqrLength() < 0.5f ) {
				++mCurrentPathVertex;
				if( mCurrentPathVertex >= mPath.mCount ) {
					mPathTimer = -1;
					mCurrentPathVertex = 0;
				}
			}
		}
	}

	if( mTarget ) {
		LookAt( mTarget->mBody->mPosition );
	} else {
		LookAt( targetPos );
	}

	Math::Vector3 v = delta.Normalized();

	Move( v, mFlying );

	--mPathTimer;
}

void Bot::SelectTarget() {	
	if( !mTarget || mTarget->mDead || !IsSeeTarget() ) {
		Actor * suitable = nullptr;
		float suitableDistance = FLT_MAX;
		auto actor = Actor::msRoot;
		while( actor ) {
			if( !actor->mDead && actor != this ) {
				if( gGameType == GameType::CaptureTheFlag ) {
					if( actor->mFlag && actor->mTeam != mTeam ) {
						suitable = actor;
						break;
					}
				}
				float dist = actor->mBody->mPosition.SqrDistance( mBody->mPosition );
				if( dist < suitableDistance ) {										
					if( gGameType == GameType::Deathmatch ) {
						suitable = actor;
						suitableDistance = dist;
					}
					if( gGameType == GameType::TeamDeathmatch || gGameType == GameType::CaptureTheFlag ) {
						if( actor->mTeam != mTeam ) {
							suitable = actor;
							suitableDistance = dist;
						}
					}
				}
			}
			actor = actor->mNext;
		}
		mTarget = suitable;
	}



	// Find ally with flag, and protect him at all costs
	auto actor = Actor::msRoot;
	while( actor ) {
		if( !actor->mDead && actor != this ) {
			if( gGameType == GameType::CaptureTheFlag ) {
				if( actor->mFlag && actor->mTeam == mTeam ) {
					mPreciousAlly = actor;
				}
			}
		}

		// check for collision with other actors, and jump if it is
		for( int i = 0; i < mBody->mContactCount; ++i ) {
			if( mBody->mContacts[i].mBody == actor->mBody ) {
				Jump( 0.1f );
				break;
			}
		}

		actor = actor->mNext;
	}	

	if( mPreciousAlly ) {
		if( mPreciousAlly->mDead || mPreciousAlly->mFlag == nullptr ) {
			mPreciousAlly = nullptr;
		}
	}
}

void Bot::Damage( float howMuch, Actor * attacker ) {
	Actor::Damage( howMuch, attacker );
	bool teamGameType = gGameType == GameType::CaptureTheFlag || gGameType == GameType::TeamDeathmatch;
	if( mTeam != attacker->mTeam || !teamGameType ) {
		if( howMuch > 0.5f ) {
			mTarget = attacker;
		}
	}
}

void Bot::Resurrect() {
	Actor::Resurrect();
	mPath.Clear();
	mCurrentPathVertex = 0;
	mPathTimer = 0;
	mPreciousAlly = nullptr;
	mTarget = nullptr;
	mYawTo = 0.0f;
	mYaw = 0.0f;

	AddWeapon( new Weapon( Weapon::Type::Shotgun, mBody ));
	//AddWeapon( new Weapon( Weapon::Type::Minigun, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::GrenadeLauncher, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::RocketLauncher, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::PlasmaGun, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::RailGun, mBody ) );
}

int Bot::CanUseWeapon( Weapon::Type type ) {
	for( int i = 0; i < mWeapons.mCount; ++i ) {
		if( mWeapons[i]->mType == type && mWeapons[i]->mBulletCount > 0 ) {
			return i;
		}
	}
	return -1;
}

bool Bot::SelectWeapon() {
	if( mTarget && !mDead ) { 
		if( mWeapons[mCurrentWeapon]->mBulletCount <= 0 ) {
			mCurrentWeapon = -1;
		}

		// when target is too far, railgun is the best choice
		mCurrentWeapon = CanUseWeapon( Weapon::Type::RailGun );

		// max damage on middle distance
		if( mCurrentWeapon < 0 ) {
			mCurrentWeapon = CanUseWeapon( Weapon::Type::RocketLauncher );			
		}
		if( mCurrentWeapon < 0 ) {
			mCurrentWeapon = CanUseWeapon( Weapon::Type::GrenadeLauncher );			
		}
		// max damage on minimal distance
		if( mCurrentWeapon < 0 ) {
			mCurrentWeapon = CanUseWeapon( Weapon::Type::Shotgun );		
		}
		// significant damage on any distance
		if( mCurrentWeapon < 0 ) {
			mCurrentWeapon = CanUseWeapon( Weapon::Type::PlasmaGun );
		}
		if( mCurrentWeapon < 0 ) {
			mCurrentWeapon = CanUseWeapon( Weapon::Type::Minigun );
		}
	}

	// no weapon can be selected because no ammo
	if( mCurrentWeapon < 0 ) {
		mCurrentWeapon = 0;
		return true;
	}

	return false;
}

bool Bot::IsSeeTarget() {
	if( mTarget ) {
		Collision::RayTraceResult rt;
		if( Collision::TraceRay( Math::Ray( mBody->mPosition, mTarget->mBody->mPosition ), rt, mBody )) {
			if( rt.mBody == mTarget->mBody ) {
				return true;
			}
		}
	}
	return false;
}

void Bot::Explode() {
	if( !mDead ) {
		Clear();
		while( mPivot->mChildren.mCount ) {
			SceneNode * child = ((SceneNode*)mPivot->mChildren[0]);
			child->DetachFromParent();
			new Gib( child, 0.1f );

			ParticleSystem * ps = new ParticleSystem( 100, 1.8 );
			ps->mParticleSize = 0.2f;
			ps->SetLocalPosition( mPivot->GetGlobalPosition() );
			ps->mEmitDirectionMin = Math::Vector3( -0.05, 0.02, -0.05 );
			ps->mEmitDirectionMax = Math::Vector3( 0.05, 0.12, 0.05 );
			ps->mColor = Math::Color( 255, 127, 32, 255 );
			ps->ResurrectParticles();		
		}
		mBody->mPosition = Math::Vector3( -1000, -1000, -1000 );
	}
}

int Bot::Count() {
	int count = 0;
	auto bot = msRoot;
	while( bot ) {
		++count;
		bot = bot->mNext;
	}
	return count;
}

void Bot::LookAt( const Math::Vector3 & lookAt ) {
	Math::Vector3 delta = (lookAt - mBody->mPosition).Normalize();
	
	mYawTo = Math::Atan2( delta.x, delta.z ) * 180.0f / Math::Pi;

	mYawTo = mYawTo > 0 ? mYawTo : (360.0f + mYawTo);
	mYaw = mYaw > 0 ? mYaw : (360.0f + mYaw);
	
	mYawTo = Math::Fmod( mYawTo, 360.0f );
	mYaw = Math::Fmod( mYaw, 360.0f );

	if( (int)mYawTo != (int)mYaw )
	{
		float change = 0;
		float diff = mYaw - mYawTo;
		if (diff < 0) {
			change = 1;
		} else {
			change = -1;
		}

		if( Math::FAbs( diff ) > 180) {
			change = -change;
		}

		mYaw += change * 2;
	}
	
	mPivot->SetLocalRotation( Math::Quaternion( Math::Vector3( 0, 1, 0 ), mYaw ));	
}


Bot::~Bot() {
	
}
