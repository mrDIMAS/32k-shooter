#include "precompiled.h"
#include "scenenode.h"
#include "camera.h"
#include "octree.h"
#include "light.h"

SceneNode * SceneNode::msRoot = nullptr;

SceneNode::SceneNode() : 
	mType( Type::Base ), mLocalScale( 1, 1, 1 ), mLocalRotation( 0, 0, 0, 1 ), mLocalPosition( 0, 0, 0 ),
	mNext( nullptr ), mPrev( nullptr ), mParent( nullptr ), mInRenderChain( true ) , mVisible( true ), 
	mColor( Math::Vector3( 1.0f, 1.0f, 1.0f )), mAlbedo( 0.0f )
{	
	LinkedList::Append( msRoot, this );
}

void SceneNode::Render( Camera * pCamera ) {
	if( !mInRenderChain ) {
		return;
	}

	Math::Matrix modelView = mGlobalTransform * pCamera->mViewMatrix;

	GLCall( glMatrixMode( GL_MODELVIEW ));
	GLCall( glLoadMatrixf( modelView.f ));
	
	for( int surfaceNum = 0; surfaceNum < mSurfaces.mCount; surfaceNum++ ) {
		mSurfaces[ surfaceNum ]->Render();
	}
}

void SceneNode::SetLocalPosition( const Math::Vector3 & pos ) {
	mLocalPosition = pos;
	CalculateTransforms();
}

void SceneNode::SetLocalScale( const Math::Vector3 & scale ) {
	mLocalScale = scale;
	CalculateTransforms();
}

void SceneNode::CalculateTransforms() {
	CalculateLocalTransform();
	CalculateGlobalTransform();
}

void SceneNode::CalculateGlobalTransform() {
	if( mParent ) {
		mGlobalTransform = mLocalTransform * mParent->mGlobalTransform;
	} else {
		mGlobalTransform = mLocalTransform;
	}

	for( int i = 0; i < mChildren.mCount; ++i ) {
		mChildren[i]->CalculateGlobalTransform();
	}
}

void SceneNode::CalculateLocalTransform() {
	Math::Matrix scale;
	scale.Scale( mLocalScale );
	mLocalTransform.SetRotationOrigin( mLocalRotation, mLocalPosition );
	mLocalTransform = scale * mLocalTransform;

	for( int i = 0; i < mChildren.mCount; ++i ) {
		mChildren[i]->CalculateGlobalTransform();
	}
}

SceneNode::~SceneNode() {
	DetachFromParent();

	LinkedList::Extract( msRoot, this );

	for( int i = 0; i < mSurfaces.mCount; i++ ) {
		delete mSurfaces[i];
	}

	for( int i = 0; i < mChildren.mCount; ++i ) {
		mChildren[i]->mParent = nullptr;
		delete mChildren[i];
	}

	for( int i = 0; i < mSounds.mCount; ++i ) {
		delete mSounds[i];
	}
}

void SceneNode::SetLocalRotation( const Math::Quaternion & rot ) {
	mLocalRotation = rot;
	CalculateTransforms();
}

void SceneNode::DetachFromParent() {
	if( mParent ) {
		mLocalPosition = mGlobalTransform.GetOrigin();
		mParent->RemoveChild( this ); 
		mParent = NULL;	
		CalculateTransforms();
	}
}

void SceneNode::RemoveChild( SceneNode * child ) {
	mChildren.Remove( child );
}

void SceneNode::AttachTo( SceneNode * parent ) {
	mParent = parent;
	parent->mChildren.Append( this );

	CalculateTransforms();
	parent->CalculateTransforms();
}

Surface * SceneNode::AddSurface( Surface * surf ) {
	mSurfaces.Append( surf );
	return surf;
}

int SceneNode::RemoveSurface( Surface * surf ) {
	return mSurfaces.Remove( surf );
}

void SceneNode::OptimizeSurfaces( ) {
	const int uselessFlag = 0xBADF00D;
	for( int i = 0; i < mSurfaces.mCount; i++ ) {
		auto * surf = mSurfaces[i];
		if( surf->mUserFlag != uselessFlag ) {
			for( int j = 0; j < mSurfaces.mCount; j++ ) {
				auto * otherSurf = mSurfaces[j];
				if( (otherSurf != surf) && (otherSurf->mUserFlag != uselessFlag) && (surf->mTexture == otherSurf->mTexture) ) {
					surf->Merge( otherSurf );
					otherSurf->mUserFlag = uselessFlag;
				}
			}
		}
	}
	for( int i = 0; i < mSurfaces.mCount; ) {
		if( mSurfaces[i]->mUserFlag == uselessFlag ) {
			delete mSurfaces[i];
			i = RemoveSurface( mSurfaces[i] );
		} else {
			mSurfaces[i]->mNeedUpdate = true;
			++i;
		}
	}
}

Math::Vector3 SceneNode::GetGlobalPosition() const {
	return Math::Vector3( mGlobalTransform.f[12], mGlobalTransform.f[13], mGlobalTransform.f[14] );
}

bool SceneNode::IsVisible() const {
	return mVisible & (mParent ? mParent->IsVisible() : true);
}

void SceneNode::DeleteAll() {
	while( msRoot ) {
		delete msRoot;
	}
}

void SceneNode::SyncSounds()
{
	auto node = msRoot;
	while( node ) {
		for( int i = 0; i < node->mSounds.mCount; ++i ) {
			node->mSounds[i]->SetPosition( node->GetGlobalPosition() );
		}
		node = node->mNext;
	}
}
