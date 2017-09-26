#include "precompiled.h"
#include "item.h"
#include "actor.h"
#include "hud.h"
#include "particlesystem.h"

Item * Item::msRoot;

Item::Item( Type type ) : mNext( nullptr ), mPrev( nullptr ), mType( type ), mAngle( 0.0f ), mSpawnTimer( 0 ) {
	mNode = new SceneNode;

	mPivot = new SceneNode;
	mPivot->AttachTo( mNode );

	mSpawnInterval = 360;
	mDesc = nullptr;

	Math::Vector3 lightColor( 1.0f, 1.0f, 1.0f );

	if( mType == Type::SmallHealth || mType == Type::MediumHealth || mType == Type::LargeHealth ) {		
		SceneNode * n2 = new SceneNode;
		n2->AddSurface( Surface::MakeCube( 0.05, 0.15, 0.05f ));
		n2->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Yellow];
		n2->AttachTo( mPivot );

		SceneNode * n1 = new SceneNode;
		n1->AddSurface( Surface::MakeCube( 0.048, 0.048, 0.15f ));
		n1->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Yellow];
		n1->AttachTo( mPivot );
				 
		Math::Vector3 color;

		if( mType == Type::SmallHealth ) {
			mContent = 20;
			color = Math::Vector3( 0, 1, 0 );
			mDesc = "Small Health";
		}
		if( mType == Type::MediumHealth ) {
			mContent = 40;
			color = Math::Vector3( 1, 1, 0 );
			mDesc = "Medium Health";
		}
		if( mType == Type::LargeHealth ) {
			mContent = 60;
			color = Math::Vector3( 1, 0, 0 );
			mDesc = "Large Health";
		}

		n1->mColor = color;
		n2->mColor = color;
		lightColor = color;
	}
	if( mType == Type::AmmoBelt ) {
		SceneNode * body = new SceneNode;
		body->AddSurface( Surface::MakeCylinder( 10, 0.1, 0.2 ));
		body->AttachTo( mPivot );

		mSpawnInterval = 180;
		mContent = 250;

		mDesc = "250 Bullets";
	} 
	if( mType == Type::Rockets ) {
		for( int i = -1; i <= 1; ++i ) {
			for( int j = -1; j <= 1; ++j ) {
				// Create rocket model (cylinder+cone) with light and fire
				SceneNode * body = new SceneNode;
				body->AddSurface( Surface::MakeCylinder( 10, 0.015f, 0.2f ) );
				body->AttachTo( mPivot );
				body->SetLocalRotation( Math::Quaternion( Math::Vector3( 1, 0, 0 ), 90 ));
				body->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
				body->SetLocalPosition( Math::Vector3( 0.12 * i, 0.0f, 0.12f * j ));
				SceneNode * top = new SceneNode;
				top->AddSurface( Surface::MakeCone( 10, 0.015f, 0.1f ));
				top->AttachTo( mPivot );
				top->SetLocalRotation( Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0f ), 90.0f ));
				top->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
				top->SetLocalPosition( Math::Vector3( 0.12 * i, 0.1f, 0.12f * j ));
			}
		}
		mContent = 9;

		mDesc = "Nine Rockets";
	} 

	if( mType == Type::Shells ) {
		for( int i = -1; i <= 1; ++i ) {
			for( int j = -1; j <= 1; ++j ) {
				SceneNode * body = new SceneNode;
				body->AddSurface( Surface::MakeCylinder( 10, 0.007f, 0.1f ) );
				body->AttachTo( mPivot );
				body->SetLocalRotation( Math::Quaternion( Math::Vector3( 1, 0, 0 ), 90 ));
				body->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
				body->SetLocalPosition( Math::Vector3( 0.12 * i, 0.0f, 0.12f * j ));
			}
		}
		mContent = 9;

		mDesc = "Nine Shells";
	} 

	if( mType == Type::Grenades ) {
		for( int i = -1; i <= 1; ++i ) {
			for( int j = -1; j <= 1; ++j ) {
				SceneNode * body = new SceneNode;
				body->AddSurface( Surface::MakeSphere( 6, 6, 0.05f ));
				body->AttachTo( mPivot );
				body->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
				body->SetLocalPosition( Math::Vector3( 0.12 * i, 0.0f, 0.12f * j ));
			}
		}

		mDesc = "Nine Grenades";
		mContent = 9;
	} 

	if( mType == Type::Rails ) {
		SceneNode * body = new SceneNode;
		body->AddSurface( Surface::MakeCylinder( 10, 0.1, 0.1 ));
		body->AttachTo( mPivot );
		body->SetLocalRotation( Math::Quaternion( Math::Vector3( 1, 0, 0 ), 90 ));
		body->mColor = Math::Vector3( 0, 1, 0 );

		mContent = 5;

		mDesc = "5 Rails";
	} 

	if( mType == Type::Plasma ) {
		SceneNode * body = new SceneNode;
		body->AddSurface( Surface::MakeCylinder( 10, 0.1, 0.1 ));
		body->AttachTo( mPivot );
		body->SetLocalRotation( Math::Quaternion( Math::Vector3( 1, 0, 0 ), 90 ));
		body->mColor = Math::Vector3( 0, 0.6, 0.95 );

		mContent = 50;

		mDesc = "50 Plasma";
	} 

	if( mType == Type::YellowArmor ) {
		SceneNode * n2 = new SceneNode;
		n2->AddSurface( Surface::MakeCube( 0.05f, 0.15f, 0.05f ));
		n2->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Yellow];
		n2->AttachTo( mPivot );
		
		SceneNode * n1 = new SceneNode;
		n1->AddSurface( Surface::MakeCube( 0.048, 0.048, 0.15f ));
		n1->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Yellow];
		n1->AttachTo( mPivot );
		n1->SetLocalPosition( Math::Vector3( 0.0f, 0.1f, 0.0f ));

		mContent = 30;
		mDesc = "Yellow Armor";

		lightColor = Math::Vector3( 1, 1, 0 );
	} 



	Light * light = new Light( 1.5, lightColor );
	light->AttachTo( mNode );

	LinkedList::Append( msRoot, this );
}

Item::~Item() {
	delete mNode;
	LinkedList::Extract( msRoot, this );
}

void Item::Update() {
	if( mSpawnTimer <= 0 ) {
		if( mSpawnTimer == 0 ) {
			ParticleSystem * ps = new ParticleSystem( 100, 1.5 );
			ps->mParticleSize = 0.1f;
			ps->SetLocalPosition( mNode->GetGlobalPosition() );
			ps->mEmitDirectionMin = Math::Vector3( -0.03, 0.02, -0.03 );
			ps->mEmitDirectionMax = Math::Vector3( 0.03, 0.2, 0.03 );
			ps->ResurrectParticles();
		}

		mNode->SetLocalRotation( Math::Quaternion( Math::Vector3( 0, 1, 0 ), mAngle ));
		mAngle += 0.85f;
		mPivot->SetLocalPosition( Math::Vector3( 0, 0.2, 0 ) * ( 1 + 0.1f * Math::Sin( mAngle / 20 )));
		mPivot->mVisible = true;

		auto actor = Actor::msRoot;
		while( actor ) {
			if( !actor->mDead && actor->mBody->mPosition.SqrDistance( mPivot->GetGlobalPosition() ) < 1.0 ) {
				if( mType == Type::SmallHealth || mType == Type::MediumHealth || mType == Type::LargeHealth ) {	
					mSpawnTimer = mSpawnInterval;				
					if( actor->mHealth < actor->mMaxHealth ) {
						mSpawnTimer = mSpawnInterval;
						actor->mHealth += mContent;
						if( actor->mHealth > actor->mMaxHealth ) {
							actor->mHealth = actor->mMaxHealth;
						}
					}
				}
				if( mType == Type::YellowArmor ) {
					if( actor->mArmor < actor->mMaxArmor ) {
						mSpawnTimer = mSpawnInterval;
						actor->mArmor += mContent;
						if( actor->mArmor > actor->mMaxArmor ) {
							actor->mArmor = actor->mMaxArmor;
						}
					}
				}
				for( int i = 0; i < actor->mWeapons.mCount; ++i ) {
					Weapon * wpn = actor->mWeapons[i];
					if( wpn->mType == Weapon::Type::GrenadeLauncher ) {
						if( mType == Type::Grenades ) {
							wpn->mBulletCount += mContent;
							mSpawnTimer = mSpawnInterval;
						}
					}
					if( wpn->mType == Weapon::Type::RocketLauncher ) {
						if( mType == Type::Rockets ) {
							wpn->mBulletCount += mContent;
							mSpawnTimer = mSpawnInterval;
						}
					}
					if( wpn->mType == Weapon::Type::Minigun ) {
						if( mType == Type::AmmoBelt ) {
							wpn->mBulletCount += mContent;
							mSpawnTimer = mSpawnInterval;
						}
					}
					if( wpn->mType == Weapon::Type::PlasmaGun ) {
						if( mType == Type::Plasma ) {
							wpn->mBulletCount += mContent;
							mSpawnTimer = mSpawnInterval;
						}
					}
					if( wpn->mType == Weapon::Type::RailGun ) {
						if( mType == Type::Rails ) {
							wpn->mBulletCount += mContent;
							mSpawnTimer = mSpawnInterval;
						}
					}
					if( wpn->mType == Weapon::Type::Shotgun ) {
						if( mType == Type::Shells ) {
							wpn->mBulletCount += mContent;
							mSpawnTimer = mSpawnInterval;
						}
					}
				}
				
				if( mSpawnTimer == mSpawnInterval ) {
					GUIText * info = HUD::msCurrent->AddInfoLine();
					String::Format( info->mText, "%s grabs %s", actor->mName, mDesc );			

					Sound * pickup = new Sound( gPickUpSound );
					pickup->SetPosition( mNode->GetGlobalPosition() );
					pickup->Play();
				}
				break;
			}
			actor = actor->mNext;
		}
	} else {
		mPivot->mVisible = false;
	}
	--mSpawnTimer;
}

void Item::UpdateAll() {
	auto item = msRoot;
	while( item ) {
		auto next = item->mNext;
		item->Update();
		item = next;
	}
}

void Item::DeleteAll()
{
	while( msRoot ) {
		delete msRoot;
	}
}
