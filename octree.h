#pragma once

#include "mathlib.h"

#define OCTREE_INDEX_PER_LEAF 192

namespace Collision {
	class Triangle;
}

class OctreeNode {
public:
	int * mIndices;
	int mIndexCount;
	bool mSplit;
	Math::Vector3 mMin;
	Math::Vector3 mMax;
	OctreeNode * mChildren[8];
	OctreeNode();
	~OctreeNode();
	void Split();
	bool IsContainPoint( const Math::Vector3 & point );
	bool IsContainAnyPointOfTriangle( const Collision::Triangle & triangle );
	bool IsIntersectSphere( const Math::Vector3 & position, float radius );
};

class Octree {
private:
	void BuildRecursive( OctreeNode * node, Collision::Triangle * triangles, int triangleCount, int * faceIndices, int faceIndexCount );
	void TraceSphereRecursive( OctreeNode * node, const Math::Vector3 & position, float radius );
	void TracePointRecusive( OctreeNode * node, const Math::Vector3 & point );
	void TraceRayRecursive( OctreeNode * node, const Math::Ray & ray );
public:
	OctreeNode * mRoot;
	Octree( Collision::Triangle * triangles, int triangleCount );
	~Octree();

	void TraceSphere( const Math::Vector3 & position, float radius );
	void TracePoint( const Math::Vector3 & point );
	void TraceRay( const Math::Ray & ray );
	void CountLeafsRecursive( OctreeNode * node, int & counter );
	int GetLeafCount();

	OctreeNode ** mTraceBuffer;
	int mTraceBufferSize;
};