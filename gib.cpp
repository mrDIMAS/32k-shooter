#include "precompiled.h"
#include "gib.h"

Gib * Gib::msRoot;


Gib::Gib( SceneNode * model, float size ) : mNext( nullptr ), mPrev( nullptr ), mMaxLifeTime( 500 ), mLifeTime( 500 )
{
	mModel = model;

	mBody = new Collision::Body( Collision::Body::Type::Sphere );
	mBody->mRadius = size;

	LinkedList::Append( msRoot, this );

	mBody->mPosition = model->GetGlobalPosition();

	mBody->mLinearVelocity = Math::Vector3::Random( Math::Vector3( -0.2f, 0.15f, -0.2f ), Math::Vector3( 0.2f, 0.25f, 0.2f ));
}

Gib::~Gib()
{
	delete mModel;
	delete mBody;

	LinkedList::Extract( msRoot, this );
}

void Gib::Update()
{
	mModel->SetLocalPosition( mBody->mPosition + mDisappearOffset );

	const float k = mMaxLifeTime / 4;
	if( mLifeTime < k ) {
		mDisappearOffset = Math::Vector3( 0.0f, -1.0f, 0.0f ) * ( 1.0f - mLifeTime / k );
	}

	--mLifeTime;
}

void Gib::UpdateAll()
{
	auto gib = Gib::msRoot;
	while( gib ) {
		auto next = gib->mNext;
		gib->Update();
		if( gib->mLifeTime < 0 ) {
			delete gib;
		}
		gib = next;
	}
}
