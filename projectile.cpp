#include "precompiled.h"
#include "projectile.h"
#include "light.h"
#include "particlesystem.h"
#include "player.h"
#include "bot.h"

Projectile * Projectile::msRoot;

Projectile::Projectile( Type type, const Math::Vector3 & origin, const Math::Matrix & orientation, Actor * owner ) : mRayCount( 1 ),
	mType( type ), mPrev( nullptr ), mNext( nullptr ), mPhysicalBody( nullptr ), mDestroyed( false ), mOwner( owner ), mExplosionSound( nullptr )
{
	mNode = new SceneNode;

	Sound * snd = nullptr;

	if( mType == Type::Bullet ) {
		mSplashDamage = false;
		mDamage = 2.5f;
		mFlightSpeed = 0.0f;
		mLifeTime = 120;

		snd = new Sound( gMinigunSound );
	} else if( mType == Type::ShotgunBullet ) {
		mSplashDamage = false;
		mDamage = 10.0f;
		mFlightSpeed = 0.0f;
		mLifeTime = 120;
		mRayCount = 10;

		snd = new Sound( gShotgunSound );
	} else if( mType == Type::Rail ) {
		mSplashDamage = false;
		mDamage = 80.0f;
		mFlightSpeed = 0.0f;
		mLifeTime = 120;

		snd = new Sound( gRailSound );
	} else if( mType == Type::Grenade ) {
		mSplashDamage = true;
		mDamage = 30.0f;
		mFlightSpeed = 0.63f;
		mLifeTime = 150;

		// Create grenade as low-poly sphere (frag grenade)
		SceneNode * body = new SceneNode;
		body->AddSurface( Surface::MakeSphere( 6, 6, 0.1f ));
		body->AttachTo( mNode );
		body->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];

		mPhysicalBody = new Collision::Body( Collision::Body::Type::Sphere );
		mPhysicalBody->mRadius = 0.095f;
		mPhysicalBody->mLinearVelocity = orientation.GetLook() * 0.2f;

		Light * light = new Light( 0.5f, Math::Vector3( 1, 0.0f, 0.0f ));
		light->AttachTo( mNode );

		snd = new Sound( gRocketSound );
	} else if( mType == Type::Rocket ) {
		mSplashDamage = true;
		mDamage = 40.0f;
		mFlightSpeed = 0.13f;
		mLifeTime = 400;
		// Create rocket model (cylinder+cone) with light and fire
		SceneNode * body = new SceneNode;
		body->AddSurface( Surface::MakeCylinder( 10, 0.015f, 0.2f ) );
		body->AttachTo( mNode );
		body->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		SceneNode * top = new SceneNode;
		top->AddSurface( Surface::MakeCone( 10, 0.015f, 0.1f ));
		top->AttachTo( mNode );
		top->SetLocalRotation( Math::Quaternion( Math::Vector3( 0.0f, 1.0f, 0.0f ), 180.0f ));
		top->SetLocalPosition( Math::Vector3( 0.0f, 0.0f, 0.3f ));
		top->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
		Light * light = new Light( 2.0f, Math::Vector3( 1, 0.5f, 0.14f ));
		light->AttachTo( mNode );

		ParticleSystem * fire = ParticleSystem::CreateFire( 0.6f );
		fire->AttachTo( mNode );

		mPhysicalBody = new Collision::Body( Collision::Body::Type::Sphere );
		mPhysicalBody->mRadius = 0.085f;
		mPhysicalBody->mLinearVelocity = orientation.GetLook() * 0.2f;


		snd = new Sound( gRocketSound );
	} else if( mType == Type::Plasma ) {
		mSplashDamage = false;
		mDamage = 15.0f;
		mFlightSpeed = 0.18f;
		mLifeTime = 400;

		// Plasma ball
		ParticleSystem * plasma = new ParticleSystem( 12, 0.2f );
		plasma->AttachTo( mNode );
		plasma->mParticleSize = 0.13f;
		plasma->mAutoResurrection = true;
		plasma->mEmitDirectionMin = Math::Vector3( -0.03f, -0.03f, -0.03f );
		plasma->mEmitDirectionMax = Math::Vector3( 0.03f, 0.03f, 0.03f );
		plasma->mColor = Math::Color( 0, 162, 232, 255 );
		plasma->ResurrectParticles();

		Light * light = new Light( 2.0f, Math::Vector3( 0, 0.8f, 1.0f ));
		light->AttachTo( mNode );

		mPhysicalBody = new Collision::Body( Collision::Body::Type::Sphere );
		mPhysicalBody->mRadius = 0.085f;
		mPhysicalBody->mLinearVelocity = orientation.GetLook() * 0.2f;

		snd = new Sound( gPlasmaSound );
		
	} else {
		// Unknown 
	}

	LinkedList::Append( msRoot, this );

	mNode->SetLocalRotation( Math::Quaternion( orientation ));

	if( mOwner != Player::msCurrent ) {
		if( gDifficulty == Difficulty::Easy ) {
			mNode->SetLocalRotation( mNode->mLocalRotation * Math::Quaternion( Math::Vector3::RandomUnit(), Math::RandFloat( -20, 20 )));
		}
		if( gDifficulty == Difficulty::Medium ) {
			mNode->SetLocalRotation( mNode->mLocalRotation * Math::Quaternion( Math::Vector3::RandomUnit(), Math::RandFloat( -10, 10 )));
		}
		if( gDifficulty == Difficulty::Hard ) {
			mNode->SetLocalRotation( mNode->mLocalRotation * Math::Quaternion( Math::Vector3::RandomUnit(), Math::RandFloat( -5, 5 )));
		}
	} 

	if( mPhysicalBody ) {
		mPhysicalBody->mPosition = origin;	
	} else {
		mNode->SetLocalPosition( origin );
	}

	if( snd ) {
		snd->SetPosition( origin );
		snd->Play();
	}
}

Projectile::~Projectile() {
	LinkedList::Extract( msRoot, this );
	delete mPhysicalBody;
	delete mNode;
}

void Projectile::Update() {
	if( mPhysicalBody ) {
		if( mType == Type::Rocket || mType == Type::Plasma ) {
			mPhysicalBody->mLinearVelocity = mNode->mGlobalTransform.GetLook() * mFlightSpeed;
		} 
		mNode->SetLocalPosition( mPhysicalBody->mPosition );
	} else {
		mNode->SetLocalPosition( mNode->GetGlobalPosition() + mNode->mGlobalTransform.GetLook() * mFlightSpeed );	
	}

	// Bullet presented as a ray due to very large speed
	if( mType == Type::Bullet || mType == Type::ShotgunBullet || mType == Type::Rail ) {				
		for( int i = 0; i < mRayCount; ++i ) {
			Collision::RayTraceResult rtResult;
			Math::Vector3 randomDir = Math::Vector3::Random( Math::Vector3( -0.1, -0.1, -0.1 ), Math::Vector3( 0.1, 0.1, 0.1 ));
	
			if( (mType == Type::Rail && mOwner == Player::msCurrent)) {
				randomDir = Math::Vector3( 0.0f, 0.0f, 0.0f );
			}

			float trailLength = 50.0f;

			Math::Ray ray( mNode->GetGlobalPosition(), mNode->GetGlobalPosition() + ( mNode->mGlobalTransform.GetLook() + randomDir ) * 200 );
			

			if( Collision::TraceRay( ray, rtResult, mOwner->mBody )) {
				trailLength = (ray.mBegin - rtResult.mPosition).Length() * 2;

				bool hitBot = false;
				auto actor = Actor::msRoot;
				while( actor ) {
					if( actor != mOwner ) {
						if( actor->mBody == rtResult.mBody ) {
							actor->Damage( mDamage, mOwner );
							hitBot = true;
							break;
						}
					}
					actor = actor->mNext;
				}
								
				auto projectile = Projectile::msRoot;
				while( projectile ) {
					if( projectile != this ) {
						if( projectile->mType != Type::Plasma ) {
							if( !projectile->mDestroyed ) {
								if( projectile->mNode->GetGlobalPosition().SqrDistance( rtResult.mPosition ) < 2 ) {
									projectile->mLifeTime = 0;
								}							
							}
						}
					}
					projectile = projectile->mNext;
				}
				
				ParticleSystem * ps = new ParticleSystem( 64, 0.5 );
				ps->SetLocalPosition( rtResult.mPosition );
				Math::Vector3 r = Math::Vector3::Reflect( ray.mDir.Normalized(), rtResult.mNormal );
				ps->mEmitDirectionMin = r * 0.015f;
				ps->mEmitDirectionMax = r * 0.050f;
				if( hitBot ) {
					if( actor->mArmor > 0  ) {
						ps->mColor = Math::Color( 105, 224, 224, 255 );
					} else {
						ps->mColor = Math::Color( 255, 255, 0, 255 );
					}
					ps->mParticleSize = 0.008f;
				}
				if( mType == Type::Rail ) {
					ps->mParticleSize = 0.08;
					ps->mEmitDirectionMin *= 1.5;
					ps->mEmitDirectionMax *= 1.5;
					ps->mRadius *= 3;
				}
				ps->ResurrectParticles();
			}

			if( mType == Type::Rail ) {
				randomDir = Math::Vector3( 0.0f, 0.0f, 0.0f );
				ParticleSystem * trail = ParticleSystem::CreateTrail( 0.06, trailLength );
				trail->SetLocalPosition( mNode->GetGlobalPosition() );
				trail->SetLocalRotation( Math::Quaternion( mNode->mGlobalTransform ));
			}
		}
		mDestroyed = true; 
	} 

	if( mPhysicalBody ) {
		if( mPhysicalBody->mContactCount > 0 ) {
			if( mType == Type::Rocket || mType == Type::Plasma ) {
				Explode();
			}

			// Some projectiles detonates when contacts with actors's body
			if( mType == Type::Grenade || mType == Type::Plasma ) {
				auto actor = Actor::msRoot;
				while( actor ) {
					for( int i = 0; i < mPhysicalBody->mContactCount; ++i ) {
						if( actor->mBody == mPhysicalBody->mContacts[i].mBody ) {
							// Prevent self-damage
							if( mOwner != actor ) {
								if( mSplashDamage ) {
									Explode();
								} else {
									actor->Damage( mDamage, mOwner );
									mDestroyed = true;
								}
							}
						}
					}
					
					actor = actor->mNext;
				}
			}
		}
	}

	--mLifeTime;
	if( mLifeTime < 0 ) {
		Explode();		
	}
}

float Projectile::PushBody( Collision::Body * body ) {
	Math::Vector3 k = body->mPosition - mNode->GetGlobalPosition();
	float d2 = k.SqrLength();
	float dk = 0.0f;
	if( d2 < 4 ) {
		if( mType == Type::Rocket || mType == Type::Grenade ) {
			dk = ( 1.0f - d2 * 0.25f );
			//body->mLinearVelocity += k * dk * 0.25f;
		}
	}
	return dk;
}

void Projectile::DoSplashDamage() {
	Collision::Body::PushBodies( mNode->GetGlobalPosition(), 2, 0.125f );

	PushBody( Player::msCurrent->mBody );
	
	auto actor = Actor::msRoot;
	while( actor ) {
		// Prevent self-damage
		if( mOwner != actor ) {
			float damageCoeff = PushBody( actor->mBody );
			actor->Damage( mDamage * damageCoeff, mOwner );		
		}
		actor = actor->mNext;
	}

	// Explode other projectiles, if they are close enough (i.e. rocket detonates grenade)
	auto projectile = Projectile::msRoot;
	while( projectile ) {
		if( projectile != this ) {
			if( !projectile->mDestroyed ) {
				if( projectile->mType == Type::Grenade ) {
					if( projectile->mNode->GetGlobalPosition().SqrDistance( mNode->GetGlobalPosition() ) < 2 ) {
						projectile->mLifeTime = 0;
					}					 
				}
			}
		}
		projectile = projectile->mNext;
	}
}

void Projectile::DoContactDamage() {

}

void Projectile::Explode() {
	if( mType == Type::Grenade ) {
		ParticleSystem * ps = new ParticleSystem( 100, 1.5 );
		ps->mParticleSize = 0.1f;
		ps->SetLocalPosition( mNode->GetGlobalPosition() );
		ps->mEmitDirectionMin = Math::Vector3( -0.03, 0.02, -0.03 );
		ps->mEmitDirectionMax = Math::Vector3( 0.03, 0.2, 0.03 );
		ps->ResurrectParticles();

		Sound * snd = new Sound( gExplosionBuffer, true );
		snd->Play();
		snd->SetPosition( mNode->GetGlobalPosition() );

		DoSplashDamage();
	}
	if( mType == Type::Rocket ) {
		ParticleSystem * ps = new ParticleSystem( 100, 1.5 );
		ps->mParticleSize = 0.1f;
		ps->SetLocalPosition( mNode->GetGlobalPosition() );
		ps->mEmitDirectionMin = Math::Vector3( -0.03, 0.02, -0.03 );
		ps->mEmitDirectionMax = Math::Vector3( 0.03, 0.2, 0.03 );
		ps->ResurrectParticles();

		Sound * snd = new Sound( gExplosionBuffer, true );
		snd->Play();
		snd->SetPosition( mNode->GetGlobalPosition() );

		DoSplashDamage();
	}

	mDestroyed = true;
}

void Projectile::UpdateAll() {
	auto projectile = Projectile::msRoot;
	while( projectile ) {
		auto next = projectile->mNext;
		projectile->Update();
		if( projectile->mDestroyed ) {
			delete projectile;
		}
		projectile = next;
	}
}

void Projectile::DeleteAll()
{
	while( msRoot ) {
		delete msRoot;
	}
}
