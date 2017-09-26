#pragma once

#include "scenenode.h"

class Camera : public SceneNode {
public:
	static Camera * msCurrentCamera;

	float mFov;

	Math::Matrix mViewMatrix;
	Math::Matrix mProjectionMatrix;
	Math::Matrix mInvViewProjMatrix;
	Math::Matrix mViewProjectionMatrix;

	float frustum[6][4];

	Camera( float fov );
	~Camera() {
		if( msCurrentCamera == this ) {
			msCurrentCamera = nullptr;
		}
	}

	// COPY-PASTE!!!
	void ExtractFrustum();

	bool IsSphereInFrustum( const Math::Vector3 & pos, float radius ) {
		for( int p = 0; p < 6; p++ ) {
			if( frustum[p][0] * pos.x + frustum[p][1] * pos.y + frustum[p][2] * pos.z + frustum[p][3] <= -radius ) {
				return false;
			}
		}
		return true;
	}

	void Update();

};
