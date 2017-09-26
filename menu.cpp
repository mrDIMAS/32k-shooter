#include "precompiled.h"
#include "menu.h"
#include "World.h"
#include "projectile.h"
#include "gib.h"
#include "actor.h"
#include "player.h"
#include "item.h"

Menu * gMenu;

void TotalCleanup() {
	gSkySphere = nullptr;
	gGameType = GameType::None;
	delete World::msCurrent;
	Player::msCurrent = nullptr;
	Item::DeleteAll();
	SpawnPoint::DeleteAll();
	Projectile::DeleteAll();
	Actor::DeleteAll();
	Flag::DeleteAll();
	Gib::DeleteAll();
	SceneNode::DeleteAll();
	Collision::Body::DeleteAll();
	gEngine->CreateLightSphere();
}

void Menu::Start() {
	gBlueTeamScore = 0;
	gRedTeamScore = 0;
	gGameTime = mTimeLimit;
	gFlagLimit = mFlagLimit;
	gFragLimit = mFragLimit;
	gRoundOver = false;
	new World( gMenu->mSelectedMap );
	gMenu->SetVisible( false );
	gMenu->mPage = Page::Main;
}

void Menu::OnDeathmatchClick( ) {
	gGameType = GameType::Deathmatch;
	gMenu->Start();
}

void Menu::OnExitClick( ) {
	gRunning = false;
}

void Menu::OnCTFClick() {
	gGameType = GameType::CaptureTheFlag;
	gMenu->Start();
}

void Menu::OnTDMClick() {
	gGameType = GameType::TeamDeathmatch;
	gMenu->Start();
}

void Menu::OnContinueClick() {
	gMenu->SetVisible( false );
}

void Menu::OnStopMatch() {
	TotalCleanup();
}

void Menu::OnStartGame() {
	gMenu->mPage = Page::MapSelect;
}

void Menu::OnBack() {
	gMenu->mPage = Page::Main;	
}

void Menu::OnNextMap() {
	++gMenu->mSelectedMap;
	if( gMenu->mSelectedMap >= GetMapCount() ) {
		--gMenu->mSelectedMap;
	}
}

void Menu::OnPrevMap()
{
	--gMenu->mSelectedMap;
	if( gMenu->mSelectedMap < 0 ) {
		gMenu->mSelectedMap = 0;
	}
}

void Menu::OnSettings()
{
	gMenu->mPage = Page::Settings;
}

void Menu::OnControls()
{
	gMenu->mPage = Page::Controls;
}

void Menu::OnDecreaseFov()
{
	gFOV -= 1.0f;
	if( gFOV < 60.0f ) {
		gFOV = 60.0f;
	}
}

void Menu::OnIncreaseFov()
{
	gFOV += 1.0f;
	if( gFOV > 110.0f ) {
		gFOV = 110.0f;
	}
}

void Menu::OnDecreaseSens()
{
	gMouseSens -= 0.01f;
	if( gMouseSens < 0.02f ) {
		gMouseSens = 0.02f;
	}
}

void Menu::OnIncreaseSens()
{
	gMouseSens += 0.01f;
	if( gMouseSens > 0.5f ) {
		gMouseSens = 0.5f;
	}
}

void Menu::OnInverseMouse()
{
	gInverseMouse = !gInverseMouse;
}

void Menu::OnIncreaseTime()
{
	gMenu->mTimeLimit += 60 * 60; // 1 minute
	if( gMenu->mTimeLimit > 45 * 60 * 60 ) { // 45 minutes
		gMenu->mTimeLimit = 45 * 60 * 60;
	}
}

void Menu::OnDecreaseTime()
{
	gMenu->mTimeLimit -= 60 * 60; // 1 minute
	if( gMenu->mTimeLimit < 5 * 60 * 60 ) { // 5 minutes
		gMenu->mTimeLimit = 5 * 60 * 60;
	}
}

void Menu::OnIncreaseFrag()
{
	++gMenu->mFragLimit; 
	if( gMenu->mFragLimit > 200 ) { 
		gMenu->mFragLimit = 200;
	}
}

void Menu::OnDecreaseFrag()
{
	--gMenu->mFragLimit; 
	if( gMenu->mFragLimit < 10 ) { 
		gMenu->mFragLimit = 10;
	}
}

void Menu::OnIncreaseFlag()
{
	++gMenu->mFlagLimit; 
	if( gMenu->mFlagLimit > 10 ) { 
		gMenu->mFlagLimit = 10;
	}
}

void Menu::OnDecreaseFlag()
{
	--gMenu->mFlagLimit; 
	if( gMenu->mFlagLimit < 1 ) { 
		gMenu->mFlagLimit = 1;
	}
}

void Menu::SetVisible( bool state ) {
	mVisible = state;
	mBackground->mVisible = state;
}

Menu::Menu() : mPage( Page::Main ), mSelectedMap( 0 ) {	
	Math::Vector2 backSize( 400, 600 );
	Math::Vector2 backCenter = backSize / 2.0f;
	mBackground = new GUIRect( 312, 84, 400, 600, Math::Color( 44, 65, 95, 255 ) );
	mBackground->mStroke = true;

	mFont = new GUIFont( 16 );

	Math::Color buttonColor( 68, 142, 109, 255 );
	float bw = 140, bh = 32;
	float x = backCenter.x - bw / 2.0f;

	// Page main
	mMainPage = new GUINode;
	mMainPage->AttachTo( mBackground );	

	mContinue = new GUIButton( x, bh, bw, bh, buttonColor, "Continue", mFont );
	mContinue->AttachTo( mMainPage );
	mContinue->OnClick = OnContinueClick;

	mStartGame = new GUIButton( x, bh * 2.5, bw, bh, buttonColor, "Start game", mFont );
	mStartGame->AttachTo( mMainPage );
	mStartGame->OnClick = OnStartGame;

	mStopMatch = new GUIButton( x, bh * 2.5, bw, bh, buttonColor, "Abort match", mFont );
	mStopMatch->AttachTo( mMainPage );
	mStopMatch->OnClick = OnStopMatch;

	mSettings = new GUIButton( x, bh * 4.0, bw, bh, buttonColor, "Settings", mFont );
	mSettings->AttachTo( mMainPage );
	mSettings->OnClick = OnSettings;

	mShowControls = new GUIButton( x, bh * 5.5, bw, bh, buttonColor, "Controls", mFont );
	mShowControls->AttachTo( mMainPage );
	mShowControls->OnClick = OnControls;
	
	mExit = new GUIButton( x, bh * 7.0, bw, bh, buttonColor, "Exit", mFont );
	mExit->AttachTo( mMainPage );
	mExit->OnClick = OnExitClick;

	// Page map select
	mMapSelectPage = new GUINode;
	mMapSelectPage->AttachTo( mBackground );	

	mSelectMap = new GUIText( mFont, "Select map, configure match and select game type" );
	mSelectMap->AttachTo( mMapSelectPage );
	mSelectMap->SetSize( backSize.x, 32 );
	mSelectMap->mAlignment = GUIText::Alignment::Center;
	mSelectMap->SetLocalPosition( 0, 32 );
	
	mMap = new GUIText( mFont, "" );
	mMap->AttachTo( mMapSelectPage );
	mMap->SetSize( backSize.x, 32 );
	mMap->mAlignment = GUIText::Alignment::Center;
	mMap->SetLocalPosition(0, bh * 2.5 );
	

	mPrevMap = new GUIButton( backCenter.x - 64 - 32, bh * 2.5, 32, bh, buttonColor, "<", mFont );
	mPrevMap->AttachTo( mMapSelectPage );
	mPrevMap->OnClick = OnPrevMap;

	mNextMap = new GUIButton(backCenter.x + 64, bh * 2.5, 32, bh, buttonColor, ">", mFont );
	mNextMap->AttachTo( mMapSelectPage );
	mNextMap->OnClick = OnNextMap;

	mFlagSelect = new GUISelect( mMapSelectPage, backCenter.x, bh * 4.0, backSize.x, 32, buttonColor, "", mFont );
	mFlagSelect->SetCallbacks( OnDecreaseFlag, OnIncreaseFlag );
	mFragSelect = new GUISelect( mMapSelectPage, backCenter.x, bh * 5.5, backSize.x, 32, buttonColor, "", mFont );
	mFragSelect->SetCallbacks( OnDecreaseFrag, OnIncreaseFrag );
	mTimeSelect = new GUISelect( mMapSelectPage, backCenter.x, bh * 7.0, backSize.x, 32, buttonColor, "", mFont );
	mTimeSelect->SetCallbacks( OnDecreaseTime, OnIncreaseTime );
	mDifficultySelect = new GUISelect( mMapSelectPage, backCenter.x, bh * 8.5, backSize.x, 32, buttonColor, "", mFont );
	mDifficultySelect->SetCallbacks( OnDecreaseDifficulty, OnIncreaseDifficulty );
	

	mDeathmatch = new GUIButton( x, bh * 10.0, bw, bh, buttonColor, "Deathmatch!", mFont );
	mDeathmatch->AttachTo( mMapSelectPage );
	mDeathmatch->OnClick = OnDeathmatchClick;

	mCTF = new GUIButton( x, bh * 11.5, bw, bh, buttonColor, "Capture the Flag!", mFont );
	mCTF->AttachTo( mMapSelectPage );
	mCTF->OnClick = OnCTFClick;

	mTeamDeathmatch = new GUIButton( x, bh * 13.0, bw, bh, buttonColor, "Team Deathmatch!", mFont );
	mTeamDeathmatch->AttachTo( mMapSelectPage );
	mTeamDeathmatch->OnClick = OnTDMClick;

	// Common elements
	mCaption = new GUIText( mFont, "Shooter32k" );
	mCaption->AttachTo( mBackground );
	mCaption->SetSize( backSize.x, 32 );
	mCaption->mAlignment = GUIText::Alignment::Center;

	mBack = new GUIButton( x, bh * 14.5, bw, bh, buttonColor, "Back", mFont );
	mBack->AttachTo( mBackground );
	mBack->OnClick = OnBack;

	mControls = new GUIText( mFont, 
		"== Controls ==\n"
		"[W][S][A][D] - run\n" 
		"[Left mouse button] - shoot\n"
		"[Left shift] - walk\n"
		"[Right mouse button] - zoom\n"
		"[Tab] - stats\n"
		"[1][2][3][4][5][6] - switch weapon\n"
		"[Mouse wheel] - switch weapon\n"
		);
	mControls->AttachTo( mBackground );
	mControls->SetSize( backSize.x, backSize.y / 2.5 );
	mControls->mAlignment = GUIText::Alignment::Center;
	mControls->mVisible = false;

	mCopyrigth = new GUIText( mFont, "Stepanov Dmitriy aka mr.DIMAS - 2016" );
	mCopyrigth->AttachTo( mBackground );
	mCopyrigth->SetSize( backSize.x, 32 );
	mCopyrigth->SetLocalPosition( 0, backSize.y - 32 );
	mCopyrigth->mAlignment = GUIText::Alignment::Center;

	// Settings page
	mSettingsPage = new GUINode;
	mSettingsPage->AttachTo( mBackground );

	mInverseMouse = new GUIButton( x, bh * 2.5, bw, bh, buttonColor, "", mFont );
	mInverseMouse->AttachTo( mSettingsPage );
	mInverseMouse->OnClick = OnInverseMouse;
	
	mFov = new GUIText( mFont, "" );
	mFov->AttachTo( mSettingsPage );
	mFov->SetSize( backSize.x, 32 );
	mFov->mAlignment = GUIText::Alignment::Center;
	mFov->SetLocalPosition(0, bh * 4.0 );

	mDecreaseFov = new GUIButton( backCenter.x - 64 - 32, bh * 4.0, 32, bh, buttonColor, "<", mFont );
	mDecreaseFov->AttachTo( mSettingsPage );
	mDecreaseFov->OnClick = OnDecreaseFov;

	mIncreaseFov = new GUIButton(backCenter.x + 64, bh * 4.0, 32, bh, buttonColor, ">", mFont );
	mIncreaseFov->AttachTo( mSettingsPage );
	mIncreaseFov->OnClick = OnIncreaseFov;

	mMouseSens = new GUIText( mFont, "" );
	mMouseSens->AttachTo( mSettingsPage );
	mMouseSens->SetSize( backSize.x, 32 );
	mMouseSens->mAlignment = GUIText::Alignment::Center;
	mMouseSens->SetLocalPosition(0, bh * 5.5 );
	
	mDecreaseMouseSens = new GUIButton( backCenter.x - 64 - 32, bh * 5.5, 32, bh, buttonColor, "<", mFont );
	mDecreaseMouseSens->AttachTo( mSettingsPage );
	mDecreaseMouseSens->OnClick = OnDecreaseSens;

	mIncreaseMouseSens = new GUIButton(backCenter.x + 64, bh * 5.5, 32, bh, buttonColor, ">", mFont );
	mIncreaseMouseSens->AttachTo( mSettingsPage );
	mIncreaseMouseSens->OnClick = OnIncreaseSens;


	mVisible = true;

	gMenu = this;

	mFlagLimit = 5;
	mFragLimit = 30;
	mTimeLimit = 15 * 60 * 60;
}

Menu::~Menu() {

}

void Menu::Update() {
	if( mPage == Page::Main ) {
		mMainPage->mVisible = true;
		mMapSelectPage->mVisible = false;
		if( gGameType != GameType::None ) {
			mContinue->mVisible = true;
			mStopMatch->mVisible = true;
			mStartGame->mVisible = false;
		} else {
			mContinue->mVisible = false;
			mStopMatch->mVisible = false;
			mStartGame->mVisible = true;
		}
		mBack->mVisible = false;
		mControls->mVisible = false;
		mSettingsPage->mVisible = false;
	} else if( mPage == Page::MapSelect ) {
		mMainPage->mVisible = false;
		mMapSelectPage->mVisible = true;
		mBack->mVisible = true;
		mSelectMap->mVisible = true;
		mControls->mVisible = false;
		mSettingsPage->mVisible = false;
		String::Format( mMap->mText, "%d: %s", mSelectedMap, GetMapName( mSelectedMap ) );
		String::Format( mTimeSelect->mText->mText, "Time Limit: %d min", mTimeLimit / 3600 );
		String::Format( mFragSelect->mText->mText, "Frag Limit: %d", mFragLimit );
		String::Format( mFlagSelect->mText->mText, "Flag Limit: %d", mFlagLimit );
		char * difficulty;
		if( gDifficulty == Difficulty::Easy ) {
			difficulty = "Easy";
		}
		if( gDifficulty == Difficulty::Medium ) {
			difficulty = "Medium";
		}
		if( gDifficulty == Difficulty::Hard ) {
			difficulty = "Hard";
		}
		if( gDifficulty == Difficulty::Nightmare ) {
			difficulty = "Nightmare";
		}
		String::Format( mDifficultySelect->mText->mText, "Difficulty: %s", difficulty );
	} else if( mPage == Page::Settings ) {
		mMapSelectPage->mVisible = false;
		mMainPage->mVisible = false;	
		mBack->mVisible = true;
		mControls->mVisible = false;
		mSettingsPage->mVisible = true;
		String::Format( mFov->mText, "FOV: %.1f", gFOV );
		String::Format( mMouseSens->mText, "Mouse sens: %.2f", gMouseSens );
		String::Format( mInverseMouse->mText->mText, "Mouse inverse: %s", gInverseMouse ? "Yes" : "No" );

	} else if( mPage == Page::Controls ) {
		mMapSelectPage->mVisible = false;
		mMainPage->mVisible = false;		
		mBack->mVisible = true;
		mControls->mVisible = true;
		mSettingsPage->mVisible = false;
	}

	gMouse.mFree = mVisible;
}

