#pragma once

#include "octree.h"
#include "surface.h"
#include "sound.h"

class SceneNode {
public:
	MakeList( SceneNode );

	// cuz no RTTI
	enum class Type {
		Base,
		Camera,
		Light,
		ParticleSystem
	};

	Type mType;

	bool mInRenderChain;
	bool mVisible;
	Math::Vector3 mColor;

	float mAlbedo;

	Math::Vector3 mLocalPosition;
	Math::Quaternion mLocalRotation;
	Math::Vector3 mLocalScale;

	Math::Matrix mLocalTransform;
	Math::Matrix mGlobalTransform;

	SceneNode * mParent;
		
	Array<SceneNode*> mChildren;
	Array<Surface*> mSurfaces;
	Array<Sound*> mSounds;

	Surface * AddSurface( Surface * surf );
	int RemoveSurface( Surface * surf );
	void OptimizeSurfaces( );
	explicit SceneNode();
	virtual ~SceneNode();
	void CalculateLocalTransform();
	void CalculateGlobalTransform();
	void CalculateTransforms();
	void SetLocalScale( const Math::Vector3 & scale );
	void SetLocalPosition( const Math::Vector3 & pos );
	void SetLocalRotation( const Math::Quaternion & rot );
	virtual void Render( class Camera * pCamera );
	void AttachTo( SceneNode * parent );
	void RemoveChild( SceneNode * child );
	void DetachFromParent();
	Math::Vector3 GetGlobalPosition( ) const;
	bool IsVisible( ) const;
	static void DeleteAll();
	static void SyncSounds();
};

