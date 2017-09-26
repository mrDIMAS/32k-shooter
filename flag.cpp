#include "precompiled.h"
#include "flag.h"
#include "hud.h"

Flag * Flag::msRed;
Flag * Flag::msBlue;

Flag::Flag( Type type, const Math::Vector3 & position ) : mAngle( 0.0f ), mType( type ), mPosition( position ), mGrabbed( false )
{
	mNode = new SceneNode;

	SceneNode * stick = new SceneNode;
	stick->AddSurface( Surface::MakeCylinder(6, 0.01, 1.0f ));
	stick->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0, 0.0 ), -90.0f ));
	stick->AttachTo( mNode );

	SceneNode * canvas = new SceneNode;
	canvas->AddSurface( Surface::MakeCube( 0.4, 0.2, 0.01f ));
	canvas->SetLocalPosition( Math::Vector3( 0.2f, 0.8f, 0.0f ));
	canvas->AttachTo( mNode );

	Math::Vector3 color( 1.0f, 0.0, 0.0f );

	if( type == Type::Blue ) {
		color = Math::Vector3( 0.0f, 0.0, 1.0f );
		msBlue = this;
	} else {
		msRed = this;
	}


	canvas->mColor = color;

	Light * light = new Light( 3, color );
	light->AttachTo( mNode );
	light->SetLocalPosition( Math::Vector3( 0.0f, 0.7, 0.0 ));

	mNode->SetLocalPosition( mPosition );
}

void Flag::Return() {
	mGrabbed = false;
	mNode->DetachFromParent();
	mNode->SetLocalPosition( mPosition );
}

void Flag::Update() {
	mNode->mVisible = gGameType == GameType::CaptureTheFlag;
	mAngle += 1.0f;
	mNode->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), mAngle ));

	if( mNode->GetGlobalPosition().y < -24.0f ) {
		Return();
	}
}

void Flag::UpdateAll() {
	if( msRed ) {
		msRed->Update();
	}
	if( msBlue ) {
		msBlue->Update();
	}
}
