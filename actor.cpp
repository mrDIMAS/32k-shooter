#include "precompiled.h"
#include "actor.h"
#include "spawnpoint.h"
#include "hud.h"
#include "player.h"

Actor * Actor::msRoot;
int Actor::msCount;

void Actor::Jump( float height /*= 0.075f */ )
{
	if( !mFlying ) {
		for( int i = 0; i < mBody->mContactCount; i++ ) {
			if( mBody->mContacts[i].mNormal.y > 0.707 ) {
				mBody->mLinearVelocity.y += height;
				break;
			}
		}
	}
}

Actor::Actor() : mBody( nullptr ), mHealth( 100.0f ), mArmor( 100.0f ), mSpeed( 0.05f ), mTeam( Team::Red ), mDead( false ), 
	mNext( nullptr ), mPrev( nullptr ), mFrags( 0 ), mIsPlayer( false ), mDeaths( 0 ), mFlag( nullptr ), mCurrentWeapon( 0 ), 
	mFlying( false )
{
	LinkedList::Append( msRoot, this );
	mPivot = new SceneNode;
	String::Format( mName, "Bot %d", msCount++ );
}

Actor::~Actor() {
	Clear();
	delete mPivot;
	delete mBody;
	LinkedList::Extract( msRoot, this );
	--msCount;
}

void Actor::Move( const Math::Vector3 & direction, bool fly ) {
	mDestVelocity.x = direction.x * mSpeed;
	mDestVelocity.z = direction.z * mSpeed;
	if( fly ) {
		mDestVelocity.y = direction.y * mSpeed;
	}

	if( mDestVelocity.IsNaN() ) {
		mDestVelocity = Math::Vector3( 0.0f, 0.0f, 0.0f );
	}

	float kFrict = (mBody->mContactCount > 0 || fly) ? 0.07f : 0.01f;
	mBody->mLinearVelocity.x += ( mDestVelocity.x - mBody->mLinearVelocity.x ) * kFrict;
	mBody->mLinearVelocity.z += ( mDestVelocity.z - mBody->mLinearVelocity.z ) * kFrict;
	if( fly ) {
		mBody->mLinearVelocity.y += ( mDestVelocity.y - mBody->mLinearVelocity.y ) * (kFrict * 3);
	}
	 
	mPivot->SetLocalPosition( mBody->mPosition );

	OnMove();
} 

void Actor::Damage( float howMuch, Actor * attacker ) {
	// modify damage
	if( attacker != Player::msCurrent ) {
		if( gDifficulty == Difficulty::Easy ) {
			howMuch *= 0.1f;
		}
		if( gDifficulty == Difficulty::Medium ) {
			howMuch *= 0.3f;
		}
		if( gDifficulty == Difficulty::Hard ) {
			howMuch *= 0.6f;
		}
		if( gDifficulty == Difficulty::Nightmare ) {
			howMuch *= 1.0f;
		}
	}

	bool teamGameType = gGameType == GameType::CaptureTheFlag || gGameType == GameType::TeamDeathmatch;
	// 'attacker' == 'this' means that actor falls into darkness and died from it's stupidness
	if( mTeam != attacker->mTeam || !teamGameType || attacker == this ) {
		if( mArmor > 0.0f ) {
			mArmor -= howMuch;
			howMuch = 0.0f;
			if( mArmor < 0 ) {
				howMuch = -mArmor;
				mArmor = 0;
			}
		}
		if( howMuch > 0.0f ) {
			mHealth -= howMuch;
		}
		if( mHealth <= 0.0f && !mDead ) {
			if( attacker != this ) {
				++attacker->mFrags;
			} else {
				if( gGameType == GameType::TeamDeathmatch ) {
					if( mTeam == Team::Blue ) {
						--gBlueTeamScore;
					} 
					if( mTeam == Team::Red ) {
						--gRedTeamScore;
					}
				}
			}
			++mDeaths;
			if( gGameType == GameType::TeamDeathmatch ) {
				if( attacker->mTeam == Team::Blue ) {
					++gBlueTeamScore;
				} 
				if( attacker->mTeam == Team::Red ) {
					++gRedTeamScore;
				}
			}
			Explode();
			mDead = true;

			GUIText * info = HUD::msCurrent->AddInfoLine();
			if( attacker == this ) {
				String::Format( info->mText, "%s fucked up!", attacker->mName, mName );		
			} else {
				String::Format( info->mText, "%s kills %s", attacker->mName, mName );		
			}
		}
	}
}

void Actor::Update() {
	if( gGameType != GameType::Deathmatch ) {
		SetTeamColors();
	}

	if( mBody->mPosition.y < -25.0f ) {
		Damage( 1000000, this );
	}

	TryGrabFlag( Flag::msBlue );
	TryGrabFlag( Flag::msRed );

	if( mFlag ) {
		if( mFlag->mType == Flag::Type::Blue ) {
			bool flagReturned = Flag::msRed->IsReturned();
			if( Flag::msRed->mNode->GetGlobalPosition().SqrDistance( mBody->mPosition ) < 1 && !Flag::msRed->mGrabbed && flagReturned ) {
				ThrowFlag(); 
				Flag::msBlue->Return();
				GUIText * info = HUD::msCurrent->AddInfoLine();
				String::Format( info->mText, "Red scores!" );		
				++gRedTeamScore;
			}
		} else if( mFlag->mType == Flag::Type::Red ) {
			bool flagReturned = Flag::msBlue->IsReturned();
			if( Flag::msBlue->mNode->GetGlobalPosition().SqrDistance( mBody->mPosition ) < 1 && !Flag::msBlue->mGrabbed && flagReturned ) {
				ThrowFlag();
				Flag::msRed->Return();
				GUIText * info = HUD::msCurrent->AddInfoLine();
				String::Format( info->mText, "Blue scores!" );				
				++gBlueTeamScore;
			}
		}
	}
}

void Actor::TryGrabFlag( Flag * f ) {
	if( f && !mFlag && gGameType == GameType::CaptureTheFlag ) {		
		if( f->mNode->GetGlobalPosition().SqrDistance( mBody->mPosition ) < 1 && !f->mGrabbed ) {
			if( f->mType == Flag::Type::Blue && mTeam == Team::Red || f->mType == Flag::Type::Red && mTeam == Team::Blue ) {
				f->mGrabbed = true;
				mFlag = f;
				f->mNode->SetLocalPosition( Math::Vector3( 0.0f, 0.0f, 0.0f ));
				f->mNode->AttachTo( mPivot );
				GUIText * info = HUD::msCurrent->AddInfoLine();
				String::Format( info->mText, "%s captured %s flag!", mName, f->mType == Flag::Type::Blue ? "Blue" : "Red" );				
			} else {
				// return team flag
				if( !f->IsReturned() ) {
					GUIText * info = HUD::msCurrent->AddInfoLine();
					String::Format( info->mText, "%s returns %s flag!", mName, f->mType == Flag::Type::Blue ? "Blue" : "Red" );	
					f->Return();
					mFlag = nullptr;
				}
			}
		}
	}
	if( mFlag ) {
		if( f->mNode->GetGlobalPosition().SqrDistance( mBody->mPosition ) < 1 && !f->mGrabbed ) {
			if( !(f->mType == Flag::Type::Blue && mTeam == Team::Red || f->mType == Flag::Type::Red && mTeam == Team::Blue)) {
				if( !f->IsReturned() ) {
					GUIText * info = HUD::msCurrent->AddInfoLine();
					String::Format( info->mText, "%s returns %s flag!", mName, f->mType == Flag::Type::Blue ? "Blue" : "Red" );	
					f->Return();
				}
			}
		}
	}
}

void Actor::ThrowFlag() {
	if( mFlag ) {
		mFlag->mGrabbed = false;
		mFlag->mNode->DetachFromParent();
		mFlag->mNode->SetLocalPosition( mBody->mPosition );
		mFlag = nullptr;
	}
}

void Actor::Resurrect() {
	mHealth = mMaxHealth;
	mArmor = mMaxArmor;
	mDead = false;
	Clear();
}

void Actor::Think() {

}

void Actor::Explode() {

}

void Actor::AddWeapon( Weapon * weapon ) {
	mWeapons.Append( weapon );
}

void Actor::SetTeamColors()
{

}

void Actor::OnMove()
{

}

void Actor::Clear() {
	ThrowFlag();
	while( mWeapons.mCount ) {
		delete mWeapons[0];
		mWeapons.Remove( mWeapons[0] );
	}
}

void Actor::UpdateAll() {
	auto actor = msRoot;
	while( actor ) {
		auto next = actor->mNext;	
		if( actor->mHealth > 0 ) {
			actor->Update();
			actor->Think();
		}
		actor = next;
	}
}

void Actor::DeleteAll() {
	while( msRoot ) {
		delete msRoot;
	}
}

int Actor::Count() {
	int count = 0;
	auto actor = msRoot;
	while( actor ) {
		++count;
		actor = actor->mNext;
	}
	return count;
}

void Actor::SelectTeams() {
	int actorN = 0;
	int actorCount = Count();
	auto actor = msRoot;
	while( actor ) {
		if( actorN > actorCount / 2 ) {
			actor->mTeam = Team::Blue;
		} else {
			actor->mTeam = Team::Red;
		}
		actor->SetTeamColors();
		++actorN;
		actor = actor->mNext;
	}
}
