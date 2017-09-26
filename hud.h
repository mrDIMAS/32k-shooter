#pragma once

#include "gui.h"

class HUD {
public:
	GUIFont * mFont;
	GUIFont * mLargeFont;

	GUIText * mWeaponBulletCount;
	GUIText * mWeaponNameNumber;
	GUIText * mRespawn;
	GUIText * mHealth;
	GUIText * mArmor;
	
	GUIText * mCrosshair;

	GUIText * mFirstPlace;
	GUIText * mSecondPlace;
	GUIText * mTeam;

	GUIText * mRedTeamScore;
	GUIText * mBlueTeamScore;
	GUIText * mGameTime;
	GUIText * mPressToContinue;
	GUIText * mTeamWins;

	GUIText * mScoreCaption;
	Array<GUIText*> mScoreLines;
	Array<GUIText*> mInfoLines;

	static HUD * msCurrent;

	int mRemoveTimer;

	HUD();

	void Update();
	GUIText * AddInfoLine( );
};