#include "precompiled.h"
#include "weapon.h"
#include "particlesystem.h"
#include "actor.h"

Weapon::Weapon( Type type, Collision::Body * owner ) :  mType( type ), mOwnerBody( owner ), mUseWiggle( true ), mName( nullptr ) {
	mNode = new SceneNode;

	mLight = new Light( 3, Math::Vector3( 1, 1, 1 ));
	mLight->AttachTo( mNode );

	mShootTimer = 0;
	mEnergyContainer = nullptr;

	mAngle = 0.0f;
	mTurnSpeed = 0.0f;
	mShooting = false;

	if( mType == Type::GrenadeLauncher ) {
		mProjectileType = Projectile::Type::Grenade;
		mShootInterval = 40;

		SceneNode * barrel = new SceneNode;
		barrel->AddSurface( Surface::MakeCylinder( 25, 0.05, 0.25 ));		
		barrel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		barrel->AttachTo( mNode );

		mBulletCount = 15;

		mName = "Grenade Launcher";
	} else if( mType == Type::Minigun ) {
		mProjectileType = Projectile::Type::Bullet;
		mShootInterval = 1;

		const float dTheta = 2.0f * Math::Pi / 6.0f;
		float r = 0.1;
		for( int i = 0; i < 6; i++ ) {
			SceneNode * barrel = new SceneNode;
			barrel->AddSurface( Surface::MakeCylinder( 16, 0.05, 0.4 ));		
			barrel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
			barrel->SetLocalPosition( Math::Vector3( r * Math::Cos( dTheta * i ), r * Math::Sin( dTheta * i ), 0.0f ));
			barrel->AttachTo( mNode );
		}

		SceneNode * disc = new SceneNode;
		disc->AddSurface( Surface::MakeCylinder( 25, 0.17, 0.02 ));		
		disc->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		disc->SetLocalPosition( Math::Vector3( 0, 0, 0.3f ));
		disc->AttachTo( mNode );

		SceneNode * disc2 = new SceneNode;
		disc2->AddSurface( Surface::MakeCylinder( 25, 0.17, 0.02 ));		
		disc2->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		disc2->SetLocalPosition( Math::Vector3( 0, 0, 0.23f ));
		disc2->AttachTo( mNode );

		mNode->SetLocalScale( Math::Vector3( 0.3f, 0.3f, 1.0f ));

		mName = "Minigun";

		mBulletCount = 250;
	} else if( mType == Type::RocketLauncher ) {
		mProjectileType = Projectile::Type::Rocket;
		mShootInterval = 40;

		SceneNode * barrel = new SceneNode;
		barrel->AddSurface( Surface::MakeCylinder( 25, 0.025, 0.25 ));		
		barrel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		barrel->AttachTo( mNode );

		mName = "Rocket Launcher";

		mBulletCount = 15;
	} else if( mType == Type::PlasmaGun ) {
		mProjectileType = Projectile::Type::Plasma;
		mShootInterval = 14;

		SceneNode * barrel = new SceneNode;
		barrel->AddSurface( Surface::MakeCylinder( 10, 0.035, 0.3 ));		
		barrel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		barrel->AttachTo( mNode );
		barrel->SetLocalPosition( Math::Vector3( 0.0f, 0.0f, 0.08f ));

		mEnergyContainer = new SceneNode;
		mEnergyContainer->AddSurface( Surface::MakeCylinder( 16, 0.032, 0.05 ));		
		mEnergyContainer->SetLocalScale( Math::Vector3( 1.2, 1, 1 ));
		mEnergyContainer->AttachTo( mNode );
		mEnergyContainer->SetLocalPosition( Math::Vector3( 0.0f, 0.0f, 0.2f ));
		
		mEnergyContainer->mAlbedo = 0.5f;

		mName = "Plasma Gun";
		
		mBulletCount = 100;
	} else if( mType == Type::Shotgun ) {
		mProjectileType = Projectile::Type::ShotgunBullet;
		mShootInterval = 35;

		for( int i = 0; i < 2; i++ ) {
			SceneNode * barrel = new SceneNode;
			barrel->AddSurface( Surface::MakeCylinder( 16, 0.015f, 0.32f ));
			barrel->AttachTo( mNode );
			barrel->SetLocalPosition( Math::Vector3( -0.025f + i * 0.025f, 0.0f, 0.0f ));
			barrel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		}

		mName = "Shotgun";

		mBulletCount = 25;
	} else if( mType == Type::RailGun ) {
		mProjectileType = Projectile::Type::Rail;
		mShootInterval = 65;


		SceneNode * barrel = new SceneNode;
		barrel->AddSurface( Surface::MakeCylinder( 10, 0.035, 0.3 ));		
		barrel->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		barrel->AttachTo( mNode );
		barrel->SetLocalPosition( Math::Vector3( 0.0f, 0.0f, 0.08f ));

		SceneNode * barrel2 = new SceneNode;
		barrel2->AddSurface( Surface::MakeCylinder( 10, 0.035, 0.2 ));		
		barrel2->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		barrel2->AttachTo( mNode );
		barrel2->SetLocalPosition( Math::Vector3( -0.018f, -0.04f, 0.08f ));

		SceneNode * barrel3 = new SceneNode;
		barrel3->AddSurface( Surface::MakeCylinder( 10, 0.035, 0.2 ));		
		barrel3->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		barrel3->AttachTo( mNode );
		barrel3->SetLocalPosition( Math::Vector3( 0.018f, -0.04f, 0.08f ));

		mEnergyContainer = new SceneNode;
		mEnergyContainer->AddSurface( Surface::MakeCylinder( 16, 0.032, 0.05 ));		
		mEnergyContainer->SetLocalScale( Math::Vector3( 1.2, 1, 1 ));
		mEnergyContainer->AttachTo( mNode );
		mEnergyContainer->SetLocalPosition( Math::Vector3( 0.0f, 0.0f, 0.2f ));
		mEnergyContainer->mAlbedo = 0.5f;
		 
		mName = "Rail Gun";

		mBulletCount = 2; 
	}
}

Weapon::~Weapon() {
	delete mNode;
}

void Weapon::Shoot( Actor * owner ) {
	mShooting = true;
	if( mType == Type::Minigun ) {
		mTurnSpeed += 0.25f;
		if( mTurnSpeed > 12.0f ) {
			mTurnSpeed = 12.0f;
		}
	}

	bool canShoot = mShootTimer <= 0;
	if( mType == Type::Minigun ) {
		canShoot &= mTurnSpeed > 6.0f;
	}

	if( mBulletCount > 0 ) {
		if( canShoot ) {
			mLight->mRadius = 5;

			Projectile * proj = new Projectile( mProjectileType, mNode->GetGlobalPosition(), mNode->mGlobalTransform, owner );

			if( proj->mPhysicalBody ) {
				proj->mPhysicalBody->mIgnoreBodies.Append( mOwnerBody );
				mOwnerBody->mIgnoreBodies.Append( proj->mPhysicalBody );
			}
			 
			mShootTimer = mShootInterval;
			if( mType == Type::RocketLauncher ) {
				mShotOffset = Math::Vector3( 0.0f, 0.015f, -0.2f );
			}
			if( mType == Type::GrenadeLauncher ) {
				mShotOffset = Math::Vector3( 0.0f, 0.025f, -0.2f );
			}
			if( mType == Type::Shotgun ) {
				mShotOffset = Math::Vector3( 0.0f, 0.07f, -0.15f );
			}
			if( mType == Type::PlasmaGun ) {
				mShotOffset = Math::Vector3( 0.0f, 0.02f, -0.15f );
			}
			if( mType == Type::RailGun ) {
				mShotOffset = Math::Vector3( 0.0f, 0.0f, -0.2f );
			}

			--mBulletCount;
		}	
	}


}

void Weapon::Update() {
	mLight->mRadius -= 0.25f;
	if( mLight->mRadius < 0 ) {
		mLight->mRadius = 0;
	}		
	mShootTimer--;	
	if( !mShooting ) {
		mTurnSpeed -= 0.095f;
		if( mTurnSpeed < 0 ) {
			mTurnSpeed = 0;
		}
	}
	mShooting = false;

	mAngle += mTurnSpeed;
	mNode->SetLocalRotation( Math::Quaternion( Math::Vector3( 0, 0, 1 ), mAngle ));

	const float u = 0.05f;

	mShotOffset.x += (mShotOffsetTo.x - mShotOffset.x) * u;
	mShotOffset.y += (mShotOffsetTo.y - mShotOffset.y) * u;
	mShotOffset.z += (mShotOffsetTo.z - mShotOffset.z) * u;

	if( mUseWiggle ) {
		mNode->SetLocalPosition( mShotOffset + mShakeOffset );
	}

	if( mEnergyContainer ) {
		float k = mShootTimer;
		if( k < 0 ) {
			k = 0;
		}
		float clr = 1.0f - k / (float)mShootInterval;
		if( mBulletCount <= 0 ) {
			clr = 0;
		}
		if( mType == Type::PlasmaGun ) {
			mEnergyContainer->mColor = Math::Vector3( 0.3, 0.8, 1 ) * clr;			
		} 
		if( mType == Type::RailGun ) {
			mEnergyContainer->mColor = Math::Vector3( 0.1, 0.8, 0.1 ) * clr;
		}

		mEnergyContainer->mAlbedo = clr;
	}
}
