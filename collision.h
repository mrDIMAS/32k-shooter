#pragma once

#include "mathlib.h"
#include "octree.h"

namespace Collision {

class Triangle {
public:
	// bounding box
	Math::Vector3 mMin;
	Math::Vector3 mMax;

	Math::Vector3 mNormal;
	// vertices 
	Math::Vector3 mA;
	Math::Vector3 mB;
	Math::Vector3 mC;
	// edges 
	Math::Vector3 mAB;
	Math::Vector3 mBC;
	Math::Vector3 mCA;
	// precalculated, for barycentric method 
	Math::Vector3 mBA;
	float mCADotCA;
	float mCADotBA;
	float mBADotBA;
	float mInvDenom;    
	// precalculated edge rays
	Math::Ray mABRay;
	Math::Ray mBCRay;
	Math::Ray mCARay;    
	// precalculated distance from center of coordinates
	float mDistance;  

	Triangle( const Math::Vector3 & a, const Math::Vector3 & b, const Math::Vector3 & c );
	bool CheckPoint( const Math::Vector3 & point ) const;
	bool IsIntersectAABB( const Math::Vector3 & bbMin, const Math::Vector3 & bbMax );
	bool IsIntersectRay( const Math::Ray & ray, Math::Vector3 & intPoint );
};

class Contact {
public:
	Math::Vector3 mNormal;
	Math::Vector3 mPosition;
	class Body * mBody;
};

Math::Vector3 ProjectPointOnLine( const Math::Vector3 & point, const Math::Vector3 & a, const Math::Vector3 & b );
Math::Vector3 ProjectVectorOnPlane( const Math::Vector3 & a, const Math::Vector3 & planeNormal );
bool IsPointInsideAABB( const Math::Vector3 & point, const Math::Vector3 & a, const Math::Vector3 & b );

class RayTraceResult {
public:
	Math::Vector3 mPosition;
	Math::Vector3 mNormal;
	class Body * mBody;
};

bool TraceRay( const Math::Ray & ray, RayTraceResult & out, class Body * ignoreBody = nullptr );

class Body {
public:
	static Body * msRoot;
	Body * mNext;
	Body * mPrev;

	enum class Type {
		Sphere,
		Polygon
	};

	Math::Vector3 mPosition;
	Math::Vector3 mLinearVelocity;
	Math::Vector3 mGravity;

	static const int MaxContacts = 4;
	Contact mContacts[MaxContacts];
	int mContactCount;

	Octree * mOctree;

	Type mType;

	// Sphere
	float mRadius;

	// Collision filtering
	Array<Body*> mIgnoreBodies;

	// Polygon
	Triangle * mTriangles;
	int mTriangleCount;

	Body( Type type );
	~Body();

	static void DeleteAll();
	void AddContact( const Math::Vector3 & point, const Math::Vector3 & normal, Body * body );
	void AddTriangles( const Vertex * vertices, int vertexCount );
	void BuildOctree( );
	bool IsSphereRayIntersection( const Math::Ray & ray, Math::Vector3 * closestIntersectionPoint = nullptr );
	bool IsSpherePointIntersection( const Math::Vector3 & point, Math::Vector3 & intersectionPoint, bool & isEdgeIntersection );
	bool IsSphereEdgeIntersection( const Math::Ray & edge, Math::Vector3 & intersectionPoint, bool & isEdgeIntersection );
	bool IsSphereTriangleIntersection( const Triangle & triangle, Math::Vector3 & intersectionPoint, bool & isEdgeIntersection );
	void SolveSpherePolygonCollision( Body * polygon );
	bool IsSphereSphereIntersection( Body * sphere, float & outPenetrationDepth );
	void SolveSphereSphereCollision( Body * sphere );
	static void SolveCollisions();
	static void PushBodies( const Math::Vector3 & point, float radius, float strength );
};

};