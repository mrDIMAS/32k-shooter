#include "precompiled.h"	
#include "player.h"

Player * Player::msCurrent;

Player::Player( const Math::Vector3 & spawnPosition ) : mCameraShakeOffset( 0.0f, 0.0f, 0.0f ),
	mRun( false ), mMove( false ), mCameraShakeCoeff( 0.0f ), mYaw( 0.0f ), mPitch( 0.0f ), 
	mFov( 80 ), mFovTo( 80 ), mZoomFov( 35 ), mPath( 0.0f )
{	
	mMaxHealth = 150.0f;
	mMaxArmor = 150.0f;

	Player::msCurrent = this;

	mCamera = new Camera( mFov );
	mCamera->AttachTo( mPivot );
	mCamera->SetLocalPosition( Math::Vector3( 0, 0.5, 0 ));

	mBody = new Collision::Body( Collision::Body::Type::Sphere );
	mBody->mRadius = 0.45f;

	mFlashlight = new Light( 8.5f, Math::Vector3( 1, 1, 1 ));
	mFlashlight->AttachTo( mCamera );
	mFlashlight->mSpot = true;
	mFlashlight->mCosHalfAngle = Math::Cos( Math::Pi / 7 );
	mFlashlight->SetLocalPosition( Math::Vector3( -0.3, -0.1, -0.25 ));

	mWeaponPivot = new SceneNode;
	mWeaponPivot->AttachTo( mCamera );
	
	Resurrect();

	mBody->mPosition = spawnPosition;
	mIsPlayer = true;

	for( int i = 0; i < 4; ++i ) {
		mFootsteps[i] = new Sound( gFootstepsBuffer[i], false );
		mPivot->mSounds.Append( mFootsteps[i] );
	}

 	String::Copy( mName, "Player" );
}

Player::~Player() {
	msCurrent = nullptr;
}

void Player::Think() {
	Math::Vector3 speedVector;
	Math::Vector3 look = mPivot->mGlobalTransform.GetLook();
	Math::Vector3 right = mPivot->mGlobalTransform.GetRight();

	mMove = false;
	mRun = true;

	if( GetAsyncKeyState( 'W' )) {
		speedVector += look;
		mMove = true;
	}
	if( GetAsyncKeyState( 'S' )) {
		speedVector -= look;
		mMove = true;
	}
	if( GetAsyncKeyState( 'A' )) {
		speedVector += right;
		mMove = true;
	}
	if( GetAsyncKeyState( 'D' )) {
		speedVector -= right;
		mMove = true;
	}

	if( GetAsyncKeyState( VK_LBUTTON )) {
		mWeapons[mCurrentWeapon]->Shoot( this );	
	}

	if( mBody->mContactCount ) {
		mPath += speedVector.SqrLength();
	}	

	if( mPath > 16 ) {
		Sound * fs = mFootsteps[ Math::Rand() % 4 ];
		if( !fs->IsPlaying() ) {
			fs->Play();
		}
		mPath = 0;
	}

	mCurrentWeapon += gMouse.wheelSpeed;
	if( mCurrentWeapon < 0 ) {
		mCurrentWeapon = 0;
	}
	if( mCurrentWeapon >= mWeapons.mCount ) {
		mCurrentWeapon = mWeapons.mCount - 1;
	}

	for( int i = 0; i < mWeapons.mCount; ++i ) {
		mWeapons[i]->Update();
		mWeapons[i]->mNode->mVisible = false;		
	}

	mWeapons[mCurrentWeapon]->mNode->mVisible = true;

	if( speedVector.Length() > Math::Epsilon ) {
		speedVector.Normalize();
	}

	if( mWeapons.mCount ) {
		if( GetAsyncKeyState( '1' )) {
			mCurrentWeapon = 0;			
		}
		if( GetAsyncKeyState( '2' )) {
			mCurrentWeapon = 1;			
		}
		if( GetAsyncKeyState( '3' )) {
			mCurrentWeapon = 2;			
		}
		if( GetAsyncKeyState( '4' )) {
			mCurrentWeapon = 3;			
		}
		if( GetAsyncKeyState( '5' )) {
			mCurrentWeapon = 4;			
		}
		if( GetAsyncKeyState( '6' )) {
			mCurrentWeapon = 5;			
		}
	}

	mSpeed = 0.065f;

	if( GetAsyncKeyState( VK_LSHIFT )) {		
		mRun = false;
	} else {
		mSpeed *= 2.0f;
	}

	if( GetAsyncKeyState( VK_SPACE )) {
		Jump();
	}

	if( GetAsyncKeyState( VK_RBUTTON )) {
		mFovTo = mZoomFov;
	} else {
		mFovTo = gFOV;
	}

	mFov += ( mFovTo - mFov ) * 0.05f;
	mCamera->mFov = mFov;

	float fovSensCoeff = mFovTo / gFOV ;

	if( mMove ) {
		if( mRun ) {
			mCameraShakeCoeff += 0.25f;
		} else {
			mCameraShakeCoeff += 0.15f;
		}

		float yOffset = 0.02f * Math::Sin( mCameraShakeCoeff );
		float xOffset = 0.02f * Math::Cos( mCameraShakeCoeff / 2 );

		if( mRun ) {
			xOffset *= 1.75f;
			yOffset *= 2.0f;
		}

		mCameraShakeOffset = Math::Vector3( xOffset, yOffset, 0.0f );

		float wyOffset = 0.005f * Math::Sin( mCameraShakeCoeff / 1.4 );
		float wxOffset = 0.003f * Math::Cos( mCameraShakeCoeff / 2.4 );

		mWeaponPivot->SetLocalPosition( Math::Vector3( -0.075f + wxOffset, -0.1f + wyOffset, 0.0 ));
	}

	if( gInverseMouse ) {
		 mPitch -= gMouse.ySpeed * gMouseSens * fovSensCoeff;
	} else {
		mPitch += gMouse.ySpeed * gMouseSens * fovSensCoeff;
	}
	mYaw -= gMouse.xSpeed * gMouseSens * fovSensCoeff;

	if( mPitch > 90 ) {
		mPitch = 90;
	}
	if( mPitch < -90 ) {
		mPitch = -90;
	}

	Move( speedVector );

	mPivot->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), mYaw ));

	mCamera->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0f ), mPitch ));		
	mCamera->SetLocalPosition( mCameraShakeOffset + Math::Vector3( 0.0f, 0.40f, 0.0f ));

	mFlashlight->mSpotDirection = mCamera->mGlobalTransform.GetLook();
}

void Player::Resurrect() {
	Actor::Resurrect();
	AddWeapon( new Weapon( Weapon::Type::Shotgun, mBody ));
	AddWeapon( new Weapon( Weapon::Type::Minigun, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::GrenadeLauncher, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::RocketLauncher, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::PlasmaGun, mBody ) );
	AddWeapon( new Weapon( Weapon::Type::RailGun, mBody ) );
}

void Player::SetPosition( const Math::Vector3 & newPosition ) {
	mBody->mPosition = newPosition;
	mPivot->SetLocalPosition( mBody->mPosition );	
}

void Player::AddWeapon( Weapon * weapon ) {
	Actor::AddWeapon( weapon );
	weapon->mNode->AttachTo( mWeaponPivot );
}

