#include "precompiled.h"
#include "hud.h"
#include "player.h"
#include "menu.h"

HUD * HUD::msCurrent;

HUD::HUD() {
	mFont = new GUIFont( 16 );
	mLargeFont = new GUIFont( 32 );

	const int halfMaxTextWidth = 64;
	const int halfMaxTextHeight = 16;

	mRemoveTimer = 0;

	mWeaponBulletCount = new GUIText( mFont, "" );
	mWeaponBulletCount->SetLocalPosition( 896 - halfMaxTextWidth, 700 );
	mWeaponBulletCount->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mWeaponBulletCount->mAlignment = GUIText::Alignment::Center;

	mWeaponNameNumber = new GUIText( mFont, "" );
	mWeaponNameNumber->SetLocalPosition( 896 - halfMaxTextWidth, 670 );
	mWeaponNameNumber->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mWeaponNameNumber->mAlignment = GUIText::Alignment::Center;
	
	mRespawn = new GUIText( mFont, "" );
	mRespawn->SetLocalPosition( 0, 0 );
	mRespawn->SetSize( 1024, 700 );
	mRespawn->mAlignment = GUIText::Alignment::Center;
	mRespawn->mColor = Math::Color( 255, 0, 0, 255 );

	mHealth = new GUIText( mFont, "" );
	mHealth->SetLocalPosition( 512 - halfMaxTextWidth, 700 );
	mHealth->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mHealth->mAlignment = GUIText::Alignment::Center;

	mArmor = new GUIText( mFont, "" );
	mArmor->SetLocalPosition( 128 - halfMaxTextWidth, 700 );
	mArmor->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mArmor->mAlignment = GUIText::Alignment::Center;

	mCrosshair = new GUIText( mFont, "" );
	mCrosshair->SetSize( 64, 64 );
	mCrosshair->SetLocalPosition( 512 - 32, 384 - 32 );
	mCrosshair->mAlignment = GUIText::Alignment::Center;

	mFirstPlace = new GUIText( mFont, "" );
	mFirstPlace->SetLocalPosition( 930.0f, 16.0f );

	mSecondPlace = new GUIText( mFont, "" );
	mSecondPlace->SetLocalPosition( 930.0f, 32.0f );

	mTeam = new GUIText( mFont, "Red Team" );
	mTeam->SetLocalPosition( 128 - halfMaxTextWidth, 670 );
	mTeam->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mTeam->mAlignment = GUIText::Alignment::Center;
	mTeam->mColor = Math::Color( 255, 0, 0, 255 );
	mTeam->mVisible = false;

	mRedTeamScore = new GUIText( mLargeFont, "" );
	mRedTeamScore->SetLocalPosition( 64, 32.0f );
	mRedTeamScore->SetSize( 1024, halfMaxTextHeight * 2 );
	mRedTeamScore->mColor = Math::Color( 255, 0, 0, 255 );
	mRedTeamScore->mAlignment = GUIText::Alignment::Center;

	mBlueTeamScore = new GUIText( mLargeFont, "" );
	mBlueTeamScore->SetLocalPosition( -64, 32.0f );
	mBlueTeamScore->SetSize( 1024, halfMaxTextHeight * 2 );
	mBlueTeamScore->mColor = Math::Color( 0, 0, 255, 255 );
	mBlueTeamScore->mAlignment = GUIText::Alignment::Center;

	mGameTime = new GUIText( mFont, "" );
	mGameTime->SetLocalPosition( 512 - halfMaxTextWidth, 670 );
	mGameTime->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mGameTime->mAlignment = GUIText::Alignment::Center;

	mPressToContinue = new GUIText( mFont, "Press [Space] to continue" );
	mPressToContinue->SetLocalPosition( 512 - halfMaxTextWidth, 500 );
	mPressToContinue->SetSize( halfMaxTextWidth * 2, halfMaxTextHeight * 2 );
	mPressToContinue->mAlignment = GUIText::Alignment::Center;
	mPressToContinue->mVisible = false;

	mTeamWins = new GUIText( mLargeFont, "" );
	mTeamWins->SetLocalPosition( 0, 72 );
	mTeamWins->SetSize( 1024, halfMaxTextHeight * 2 );
	mTeamWins->mAlignment = GUIText::Alignment::Center;
	mTeamWins->mVisible = false;

	mScoreCaption = new GUIText( mFont, "" );
	mScoreCaption->SetLocalPosition( 0, 184 );
	mScoreCaption->SetSize( 1024, 16 );
	mScoreCaption->mAlignment = GUIText::Alignment::Center;
	mScoreCaption->mVisible = false;

	msCurrent = this;
}

void HUD::Update() {
	Player * p = Player::msCurrent;
	Weapon * w = p->mWeapons[p->mCurrentWeapon];

	if( p->mHealth <= 25 ) {
		mHealth->mColor = Math::Color( 255, 0, 0, 255 );
	}
	if( p->mHealth > 25 && p->mHealth <= 50 ) {
		mHealth->mColor = Math::Color( 220, 220, 0, 255 );
	}
	if( p->mHealth > 50 ) {
		mHealth->mColor = Math::Color( 0, 255, 0, 255 );
	}
	if( p->mArmor <= 25 ) {
		mArmor->mColor = Math::Color( 255, 0, 0, 255 );
	}
	if( p->mArmor > 25 && p->mHealth <= 50 ) {
		mArmor->mColor = Math::Color( 220, 220, 0, 255 );
	}
	if( p->mArmor > 50 ) {
		mArmor->mColor = Math::Color( 0, 255, 0, 255 );
	}

	mCrosshair->mText[0] = '+';
	mCrosshair->mText[1] = 0;

	String::Format( mWeaponBulletCount->mText, "Bullets: %d", w->mBulletCount );
	String::Format( mWeaponNameNumber->mText, "%s (%d/%d)", w->mName, p->mCurrentWeapon + 1, p->mWeapons.mCount );
	String::Format( mHealth->mText, "Health: %d", (int)p->mHealth );
	String::Format( mArmor->mText, "Armor: %d", (int)p->mArmor );
	if( p->mDead ) {
		String::Format( mRespawn->mText, "You Died. Wait for Respawn" );
	} else {
		String::Format( mRespawn->mText, " " );
	}

	int firstPlaceScore = 0;
	auto actor = Actor::msRoot;
	Actor * firstPlaceActor = nullptr;
	while( actor ) {
		if( actor->mFrags >= firstPlaceScore ) {
			firstPlaceScore = actor->mFrags;
			firstPlaceActor = actor;
			String::Format( mFirstPlace->mText, "I - %s : %d", actor->mName, actor->mFrags );
		}
		actor = actor->mNext;
	}

	int secondPlaceScore = 0;
	actor = Actor::msRoot;
	while( actor ) {
		if( actor->mFrags >= secondPlaceScore && actor->mFrags < firstPlaceScore ) {
			secondPlaceScore = actor->mFrags;
			String::Format( mSecondPlace->mText, "II - %s : %d", actor->mName, actor->mFrags );
		}
		actor = actor->mNext;
	}

	if( gGameType == GameType::CaptureTheFlag || gGameType == GameType::TeamDeathmatch ) {
		int limit;
		if( gGameType == GameType::TeamDeathmatch ) {
			limit = gFragLimit;
		} else {
			limit = gFlagLimit;
		}
		String::Format( mRedTeamScore->mText, "%d/%d", gRedTeamScore, limit );
		String::Format( mBlueTeamScore->mText, "%d/%d", gBlueTeamScore, limit );		
		mTeam->mVisible = true;
		mRedTeamScore->mVisible = true;
		mBlueTeamScore->mVisible = true;		
	} else {
		mTeam->mVisible = false;
		mRedTeamScore->mVisible = false;
		mBlueTeamScore->mVisible = false;
		if( gGameType == GameType::Deathmatch ) {
			mRedTeamScore->mVisible = true;
			String::Format( mRedTeamScore->mText, "%d of %d", firstPlaceScore, gFragLimit );		
		}
	}

	bool endGame = (gRoundOver && gGameType != GameType::None);


	int secs = gGameTime / 60;
	int minutes = secs / 60;
	String::Format( mGameTime->mText, "Elapsed time %dm:%ds", minutes % 60, secs % 60 );
	mGameTime->mVisible = true;

	if( gGameType != GameType::None ) {
		if( !gRoundOver ) {
			--gGameTime;
		}
	}

	if( gGameTime < 0 ) {
		gGameTime = 0;
		gRoundOver = true;
	}

	

	if( gGameType == GameType::CaptureTheFlag ) {
		if( gBlueTeamScore >= gFlagLimit || gRedTeamScore >= gFlagLimit ) {
			gRoundOver = true;
		}
	}
	if( gGameType == GameType::TeamDeathmatch ) {
		if( gBlueTeamScore >= gFragLimit || gRedTeamScore >= gFragLimit ) {
			gRoundOver = true;
		}
	}
	if( gGameType == GameType::Deathmatch ) {
		if( firstPlaceScore >= gFragLimit ) {
			gRoundOver = true;
		}
	}

	

	mPressToContinue->mVisible = endGame;
	mTeamWins->mVisible = endGame;
	if( endGame ) {
		if( gGameType == GameType::CaptureTheFlag || gGameType == GameType::TeamDeathmatch ) {
			if( gBlueTeamScore > gRedTeamScore ) {
				String::Format( mTeamWins->mText, "Blue wins" );
			}
			if( gRedTeamScore > gBlueTeamScore  ) {
				String::Format( mTeamWins->mText, "Red wins" );
			}
			if( gRedTeamScore == gBlueTeamScore  ) {
				String::Format( mTeamWins->mText, "Draw... boooring" );
			}
		} 
		if( gGameType == GameType::Deathmatch ) {
			if( firstPlaceActor ) {
				String::Format( mTeamWins->mText, "Winner: %s", firstPlaceActor->mName );
			}
		} 
	}
	
	if( GetAsyncKeyState( VK_TAB ) || endGame ) {
		while( mScoreLines.mCount < Actor::msCount ) {
			GUIText * text = new GUIText( mFont, "" );
			text->SetLocalPosition( 0, 200 + mScoreLines.mCount * 16 );
			text->SetSize( 1024, 16 );
			text->mAlignment = GUIText::Alignment::Center;
			mScoreLines.Append( text );			
		}
		int i = 0;
		auto actor = Actor::msRoot;
		while( actor ) {
			mScoreLines[i]->mVisible = true;
			if( actor == Player::msCurrent ) {
				mScoreLines[i]->mColor = Math::Color( 0, 255, 0, 255 );
			}
			String::Format( mScoreLines[i]->mText, "%-20s\t%8d\t%8d", actor->mName, actor->mFrags, actor->mDeaths );
			actor = actor->mNext;
			++i;
		}
		String::Format( mScoreCaption->mText, "%s\t\t\t\t\t\t\t%s\t\t\t\t%s", "Name", "Frags", "Deaths" );
		mScoreCaption->mVisible = true;
	} else {
		for( int i = 0; i < mScoreLines.mCount; ++i ) {
			mScoreLines[i]->mVisible = false;
		}
		mScoreCaption->mVisible = false;
	}

	--mRemoveTimer;
	if( mInfoLines.mCount ) {
		if( mRemoveTimer < 0 ) {
			delete mInfoLines[0];
			mInfoLines.Remove( mInfoLines[0] );
			mRemoveTimer = 40;
			for( int i = 0; i < mInfoLines.mCount; ++i ) {
				mInfoLines[i]->mGY -= 16 * gGUIGlobalScale.y;
			}
		}
	}

	if( gRoundOver ) {
		mTeam->mVisible = false;
	} else {
		mTeam->mVisible = true;
	}
	if( (gRoundOver && gGameType != GameType::None) ) {
		if( GetAsyncKeyState( VK_SPACE )) {
			TotalCleanup();
			gMenu->SetVisible( true );
			for( int i = 0; i < mScoreLines.mCount; ++i ) {
				mScoreLines[i]->mVisible = false;
			}
			mPressToContinue->mVisible = false;
			mTeamWins->mVisible = false;
			mScoreCaption->mVisible = false;
		}
	}
}

GUIText * HUD::AddInfoLine() {
	int lastY = 520;
	for( int i = 0; i < mInfoLines.mCount; ++i ) {
		if( mInfoLines[i]->mGY / gGUIGlobalScale.y > lastY ) {
			lastY = mInfoLines[i]->mGY / gGUIGlobalScale.y;
		}
	}
	if( mInfoLines.mCount == 0 ) {
		mRemoveTimer = 40;
	}
	GUIText * line = new GUIText( mFont, "" );
	line->SetLocalPosition( 20, lastY + 16 );
	mInfoLines.Append( line );
	return line;
}
