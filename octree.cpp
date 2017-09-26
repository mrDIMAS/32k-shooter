#include "precompiled.h"
#include "octree.h"
#include "collision.h"
#include <limits>

Octree::Octree( Collision::Triangle * triangles, int triangleCount ) {
	mRoot = new OctreeNode;

	for( int i = 0; i < triangleCount; i++ ) {
		Collision::Triangle & triangle = triangles[i];
		Math::VMinMax( mRoot->mMin, mRoot->mMax, triangle.mA );
		Math::VMinMax( mRoot->mMin, mRoot->mMax, triangle.mB );
		Math::VMinMax( mRoot->mMin, mRoot->mMax, triangle.mC );
	}

	int * faceIndices = new int[ triangleCount ];
	for( int i = 0; i < triangleCount; i++ ) {
		faceIndices[i] = i;
	}

	BuildRecursive( mRoot, triangles, triangleCount, faceIndices, triangleCount );

	mTraceBuffer = new OctreeNode*[ GetLeafCount() ];
	mTraceBufferSize = 0;

	delete [] faceIndices;
}

void Octree::TraceSphereRecursive( OctreeNode * node, const Math::Vector3 & position, float radius ) {
	if( node->IsIntersectSphere( position, radius )) {
		if( node->mSplit ) {
			for( int i = 0; i < 8; i++ ) {
				TraceSphereRecursive( node->mChildren[i], position, radius );
			}
		} else {
			mTraceBuffer[ mTraceBufferSize++ ] = node;			
		}
	}
}

void Octree::TracePointRecusive( OctreeNode * node, const Math::Vector3 & point ) {
	if( node->IsContainPoint( point )) {
		if( node->mSplit ) {
			for( int i = 0; i < 8; i++ ) {
				TracePointRecusive( node->mChildren[i], point );
			}
		} else {
			mTraceBuffer[ mTraceBufferSize++ ] = node;
		}
	}
}

void Octree::BuildRecursive( OctreeNode * node, Collision::Triangle * triangles, int triangleCount, int * faceIndices, int faceIndexCount ) {
	if( faceIndexCount < OCTREE_INDEX_PER_LEAF ) {
		node->mIndexCount = faceIndexCount;
		node->mIndices = new int[ node->mIndexCount ];
		for( int i = 0; i < node->mIndexCount; ++i ) {
			node->mIndices[i] = faceIndices[i];
		}		
	} else {
		node->Split();

		for( int childNum = 0; childNum < 8; childNum++ ) {
			OctreeNode * child = node->mChildren[childNum];

			int leafTriangleCount = 0;
			int * leafIndices = nullptr;
			for( int i = 0; i < triangleCount; i++ ) {
				if( triangles[i].IsIntersectAABB( child->mMin, child->mMax )) {
					leafTriangleCount++;
					leafIndices = (int*)Realloc( leafIndices, leafTriangleCount * sizeof( *leafIndices ));
					leafIndices[ leafTriangleCount - 1 ] = i;					
				}			
			}

			BuildRecursive( child, triangles, triangleCount, leafIndices, leafTriangleCount );

			delete leafIndices;
		}
	}
}

Octree::~Octree() {
	delete mRoot;
	delete [] mTraceBuffer;
}

int Octree::GetLeafCount() {
	int counter = 0;
	CountLeafsRecursive( mRoot, counter );
	return counter;
}

void Octree::CountLeafsRecursive( OctreeNode * node, int & counter ) {
	if( node->mSplit ) {
		for( int i = 0; i < 8; i++ ) {
			CountLeafsRecursive( node->mChildren[i], counter );
		}
	} else {
		++counter;
	}
}

void Octree::TracePoint( const Math::Vector3 & point ) {
	mTraceBufferSize = 0;
	TracePointRecusive( mRoot, point );
}

void Octree::TraceSphere( const Math::Vector3 & position, float radius ) {
	mTraceBufferSize = 0;
	TraceSphereRecursive( mRoot, position, radius );
}

void Octree::TraceRayRecursive( OctreeNode * node, const Math::Ray & ray ) {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	if( ray.mDir.x >= 0 ) {
		tmin = (node->mMin.x - ray.mBegin.x) / ray.mDir.x;
		tmax = (node->mMax.x - ray.mBegin.x) / ray.mDir.x;
	} else {
		tmin = (node->mMax.x - ray.mBegin.x) / ray.mDir.x;
		tmax = (node->mMin.x - ray.mBegin.x) / ray.mDir.x;
	}
	if( ray.mDir.y >= 0 ) {
		tymin = (node->mMin.y - ray.mBegin.y) / ray.mDir.y;
		tymax = (node->mMax.y - ray.mBegin.y) / ray.mDir.y;
	} else {
		tymin = (node->mMax.y - ray.mBegin.y) / ray.mDir.y;
		tymax = (node->mMin.y - ray.mBegin.y) / ray.mDir.y;
	}
	if( (tmin > tymax) || (tymin > tmax) ) {
		return;
	}
	if( tymin > tmin ) {
		tmin = tymin;
	}
	if( tymax < tmax ) {
		tmax = tymax;
	}
	if( ray.mDir.z >= 0 ) {
		tzmin = (node->mMin.z - ray.mBegin.z) / ray.mDir.z;
		tzmax = (node->mMax.z - ray.mBegin.z) / ray.mDir.z;
	} else {
		tzmin = (node->mMax.z - ray.mBegin.z) / ray.mDir.z;
		tzmax = (node->mMin.z - ray.mBegin.z) / ray.mDir.z;
	}
	if( (tmin > tzmax) || (tzmin > tmax) ) {
		return;
	}
	if( tzmin > tmin ) {
		tmin = tzmin;
	}
	if( tzmax < tmax ) {
		tmax = tzmax;
	}
	if( (tmin < 1.0f) && (tmax > 0.0f)) {
		if( node->mSplit ) {
			for( int i = 0; i < 8; i++ ) {
				TraceRayRecursive( node->mChildren[i], ray );
			}
		} else {
			mTraceBuffer[ mTraceBufferSize++ ] = node;
		}
	}
}

void Octree::TraceRay( const Math::Ray & ray ) {
	mTraceBufferSize = 0;
	TraceRayRecursive( mRoot, ray );
}

bool OctreeNode::IsIntersectSphere( const Math::Vector3 & position, float radius ) {
	float r2 = radius * radius;
	float dmin = 0;

	if( position.x < mMin.x ) {
		dmin += Math::Squared( position.x - mMin.x );
	} else if( position.x > mMax.x ) {
		dmin += Math::Squared( position.x - mMax.x );
	}

	if( position.y < mMin.y ) {
		dmin += Math::Squared( position.y - mMin.y );
	} else if( position.y > mMax.y ) {
		dmin += Math::Squared( position.y - mMax.y );
	}

	if( position.z < mMin.z ) {
		dmin += Math::Squared( position.z - mMin.z );
	} else if( position.z > mMax.z ) {
		dmin += Math::Squared( position.z - mMax.z );
	}

	bool sphereInside = 
		(position.x >= mMin.x) && (position.x <= mMax.x) &&
		(position.y >= mMin.y) && (position.y <= mMax.y) &&
		(position.z >= mMin.z) && (position.z <= mMax.z);

	return dmin <= r2 || sphereInside;
}

bool OctreeNode::IsContainAnyPointOfTriangle( const Collision::Triangle & triangle ) {
	return IsContainPoint( triangle.mA ) || IsContainPoint( triangle.mB ) || IsContainPoint( triangle.mC );
}

bool OctreeNode::IsContainPoint( const Math::Vector3 & point ) {
	return	point.x >= mMin.x && point.x <= mMax.x &&
		point.y >= mMin.y && point.y <= mMax.y &&
		point.z >= mMin.z && point.z <= mMax.z;
}

void OctreeNode::Split() {
	Math::Vector3 center = (mMax + mMin) * 0.5f;

	for(int i = 0; i < 8; i++) {
		mChildren[i] = new OctreeNode;
	}
		
	mChildren[0]->mMin = Math::Vector3( mMin.x, mMin.y, mMin.z );
	mChildren[0]->mMax = Math::Vector3( center.x, center.y, center.z );

	mChildren[1]->mMin = Math::Vector3( center.x, mMin.y, mMin.z );
	mChildren[1]->mMax = Math::Vector3( mMax.x, center.y, center.z );

	mChildren[2]->mMin = Math::Vector3( mMin.x, mMin.y, center.z );
	mChildren[2]->mMax = Math::Vector3( center.x, center.y, mMax.z );

	mChildren[3]->mMin = Math::Vector3( center.x, mMin.y, center.z );
	mChildren[3]->mMax = Math::Vector3( mMax.x, center.y, mMax.z );

	mChildren[4]->mMin = Math::Vector3( mMin.x, center.y, mMin.z );
	mChildren[4]->mMax = Math::Vector3( center.x, mMax.y, center.z );

	mChildren[5]->mMin = Math::Vector3( center.x, center.y, mMin.z );
	mChildren[5]->mMax = Math::Vector3( mMax.x, mMax.y, center.z );

	mChildren[6]->mMin = Math::Vector3( mMin.x, center.y, center.z );
	mChildren[6]->mMax = Math::Vector3( center.x, mMax.y, mMax.z );

	mChildren[7]->mMin = Math::Vector3( center.x, center.y, center.z );
	mChildren[7]->mMax = Math::Vector3( mMax.x, mMax.y, mMax.z );
	
	mSplit = true;
}

OctreeNode::OctreeNode() : mMin( FLT_MAX, FLT_MAX, FLT_MAX ), mMax( -FLT_MAX, -FLT_MAX, -FLT_MAX ), mIndices( nullptr ),
	mIndexCount( 0 ), mSplit( false )
{
	for( int i = 0; i < 8; i++ ) {
		mChildren[i] = nullptr;
	}
}

OctreeNode::~OctreeNode() {
	delete [] mIndices;
	for( int i = 0; i < 8; i++ ) {
		delete mChildren[i];
	}
}
