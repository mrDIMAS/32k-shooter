#include "precompiled.h"
#include "scenenode.h"
#include "camera.h"
#include "collision.h"
#include "texture.h"
#include "World.h"
#include "light.h"
#include "player.h"
#include "particlesystem.h"
#include "bot.h"
#include "bot_rover.h"
#include "gib.h"
#include "item.h"
#include "engine.h"
#include "cstring.h"
#include "hud.h"
#include "menu.h"

extern "C" int _fltused = 0;

bool gRunning;
float gAspect = 16.0f / 9.0f;
float gFOV = 80.0f;
bool gInverseMouse = false;
float gMouseSens = 0.1f;
Math::Vector3 gAmbientColor;

GameType gGameType = GameType::None;
Difficulty gDifficulty = Difficulty::Medium;
int gBlueTeamScore = 0;
int gRedTeamScore = 0;
int gGameTime = 15 * 60 * 60;
int gFlagLimit = 5;
int gFragLimit = 30;
bool gRoundOver = true;

int main() {
	gAmbientColor = Math::Vector3( 0.2, 0.2, 0.2 );

	Memory::LoadFunctions();
	Math::LoadFunctions();
	String::LoadFunctions();
	 
	//Engine * engine = new Engine( GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ) );
	Engine * engine = new Engine( 1024, 768 );

	Menu * menu = new Menu; 
	HUD * hud = new HUD;

	Timer gameTimer, fpsTimer;

	float fixedFPS = 60.0;
	float fixedTimeStep = 1.0 / fixedFPS;
	float gameClock = gameTimer.GetTimeInSeconds();
	
	int fps = 0;
	GUIText * info = new GUIText( menu->mFont, "" );
	info->SetLocalPosition( 0.0f, 16.0f );

	float skySphereAngle = 0.0f;

	while( gRunning ) {
		float dt = gameTimer.GetTimeInSeconds() - gameClock;
		while( dt >= fixedTimeStep ) {
			gMouse.wheelSpeed = 0;

			dt -= fixedTimeStep;
			gameClock += fixedTimeStep;

			engine->Update();

			engine->FlushMouse();
			engine->UpdateMessages();
			engine->UpdateMouse();		
			GUINode::FixedUpdateAll();
			SceneNode::SyncSounds();
			Sound::UpdateAll();
			menu->Update();
			
			if( !menu->mVisible ) {
				while( ShowCursor( false ) > 0 );
				if( GetAsyncKeyState( VK_ESCAPE )) {
					menu->SetVisible( true );
				}
				hud->Update();
				if( !gRoundOver ) {
					Flag::UpdateAll();
					Item::UpdateAll();
					SpawnPoint::UpdateAll();
					Actor::UpdateAll();
					Projectile::UpdateAll();
					Collision::Body::SolveCollisions();
					Gib::UpdateAll();

					skySphereAngle += 0.006f;

					if( gSkySphere ) {
						gSkySphere->SetLocalRotation( Math::Quaternion( Math::Vector3( 0, 1, 0 ), skySphereAngle ));
					}
				}
			} else {
				while( ShowCursor( true ) <= 0 );
			}
		}
		if( fpsTimer.GetElapsedTimeInSeconds() >= 1.0f ) {
			String::Format( info->mText, "FPS: %d, MA: %d", fps, gAllocatedMemoryBlocks );
			fpsTimer.Restart();
			fps = 0;
		}
		engine->RenderWorld();
		++fps;
	}

	TotalCleanup();
	SceneNode::DeleteAll();
	Texture::FreeLibrary();
	delete engine;

	if( gAllocatedMemoryBlocks ) {
		//DebugBreak();
	}

	ExitProcess( 0 );
}