#pragma once

#include "gui.h"

enum class Page {
	Main,
	MapSelect,
	Settings,
	Controls,
};

class Menu {
public:
	GUIRect * mBackground;

	GUINode * mMainPage;
	GUIButton * mExit;
	GUIButton * mContinue;
	GUIButton * mSettings;
	GUIButton * mStopMatch;
	GUIButton * mStartGame;
	GUIButton * mShowControls;

	GUINode * mMapSelectPage;
	GUIButton * mNextMap;
	GUIButton * mPrevMap;
	GUIText * mSelectMap;
	GUIText * mMap;
	GUIButton * mDeathmatch;
	GUIButton * mCTF;
	GUIButton * mTeamDeathmatch;
	GUISelect * mFlagSelect;
	GUISelect * mFragSelect;
	GUISelect * mTimeSelect;
	GUISelect * mDifficultySelect;

	int mFlagLimit;
	int mFragLimit;
	int mTimeLimit;

	GUINode * mSettingsPage;
	GUIButton * mInverseMouse;
	GUIText * mFov;
	GUIButton * mIncreaseFov;
	GUIButton * mDecreaseFov;
	GUIText * mMouseSens;
	GUIButton * mIncreaseMouseSens;
	GUIButton * mDecreaseMouseSens;

	GUIButton * mBack;
	GUIText * mControls;
	GUIText * mCaption;
		
	GUIFont * mFont;
	GUIText * mCopyrigth;

	bool mVisible;
	int mSelectedMap;
	
	Page mPage;
	void Start();

	static void OnDeathmatchClick( );
	static void OnExitClick(  );
	static void OnCTFClick();
	static void OnTDMClick();
	static void OnContinueClick();
	static void OnStopMatch();
	static void OnStartGame();
	static void OnBack();
	static void OnNextMap();
	static void OnPrevMap();
	static void OnSettings();
	static void OnControls();
	static void OnDecreaseFov();
	static void OnIncreaseFov();
	static void OnDecreaseSens();
	static void OnIncreaseSens();
	static void OnInverseMouse();
	static void OnIncreaseTime();
	static void OnDecreaseTime();
	static void OnIncreaseFrag();
	static void OnDecreaseFrag();
	static void OnIncreaseFlag();
	static void OnDecreaseFlag();
	static void OnIncreaseDifficulty() {
		int d = (int)gDifficulty;
		++d;
		if( d == (int)Difficulty::Count ) {
			--d;
		}
		gDifficulty = (Difficulty)d;		
	}
	static void OnDecreaseDifficulty() {
		int d = (int)gDifficulty;
		--d;
		if( d < 0) {
			++d;
		}
		gDifficulty = (Difficulty)d;
	}
	void SetVisible( bool state );
	Menu();
	~Menu();
	void Update();
};
