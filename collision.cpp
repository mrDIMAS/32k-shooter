#include "precompiled.h"
#include "collision.h"
#include <limits>

namespace Collision {

Body * Body::msRoot = nullptr;

Triangle::Triangle( const Math::Vector3 & a, const Math::Vector3 & b, const Math::Vector3 & c ) : 
	mA( a ), mB( b ), mC( c ), 	mMax( -FLT_MAX, -FLT_MAX, -FLT_MAX ), mMin( FLT_MAX, FLT_MAX, FLT_MAX ) 
{
	Math::VMinMax( mMin, mMax, mA );
	Math::VMinMax( mMin, mMax, mB );
	Math::VMinMax( mMin, mMax, mC );

	Math::Vector3 ba = b - a;
	Math::Vector3 ca = c - a;

	mNormal = (ba.Cross( ca )).Normalize();

	mAB = a - b;
	mBC = b - c;
	mCA = c - a;
	mBA = ba;

	mABRay = Math::Ray( a, b );
	mBCRay = Math::Ray( b, c );
	mCARay = Math::Ray( c, a );

	mCADotCA = mCA.Dot( mCA );
	mCADotBA = mCA.Dot( mBA );
	mBADotBA = mBA.Dot( mBA );

	mInvDenom = 1.0f / ( mCADotCA * mBADotBA - mCADotBA * mCADotBA );

	mDistance = -mA.Dot( mNormal );
}

bool Triangle::IsIntersectAABB( const Math::Vector3 & bbMin, const Math::Vector3 & bbMax ) {
	Math::Vector3 cA = (mMax + mMin) * 0.5f;
	Math::Vector3 rA = (mMax - mMin) * 0.5f;
	Math::Vector3 cB = (bbMax + bbMin) * 0.5f;
	Math::Vector3 rB = (bbMax - bbMin) * 0.5f;

	if ( Math::FAbs( cA.x - cB.x ) > (rA.x + rB.x) ) return false;
	if ( Math::FAbs( cA.y - cB.y ) > (rA.y + rB.y) ) return false;
	if ( Math::FAbs( cA.z - cB.z ) > (rA.z + rB.z) ) return false;

	return true;
}

bool Triangle::CheckPoint( const Math::Vector3 & point ) const {
	Math::Vector3 vp = point - mA;
	float dot02 = mCA.Dot( vp );
	float dot12 = mBA.Dot( vp );
	float u = ( mBADotBA * dot02 - mCADotBA * dot12 ) * mInvDenom;
	float v = ( mCADotCA * dot12 - mCADotBA * dot02 ) * mInvDenom;
	return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}

bool Triangle::IsIntersectRay( const Math::Ray & ray, Math::Vector3 & intPoint ) {
	return Math::Plane( mNormal, mDistance ).Intersects( ray, intPoint ) && CheckPoint( intPoint );
}

Math::Vector3 ProjectPointOnLine( const Math::Vector3 & point, const Math::Vector3 & a, const Math::Vector3 & b ) {
	Math::Vector3 ab = b - a;    
	float div = ab.SqrLength();
	if( div < Math::Epsilon ) {
		return Math::Vector3( 0.0f, 0.0f, 0.0f );
	}
	return a + ab * (( point - a ).Dot( ab ) / div);
}

Math::Vector3 ProjectVectorOnPlane( const Math::Vector3 & a, const Math::Vector3 & planeNormal ) {
	float nlen = planeNormal.SqrLength();

	// normal vector is degenerated 
	if( nlen < Math::Epsilon ) {
		return Math::Vector3( 0.0f, 0.0f, 0.0f );
	}

	float t = a.Dot( planeNormal ) / nlen;   

	return a - planeNormal * t;	
}

bool IsPointInsideAABB( const Math::Vector3 & point, const Math::Vector3 & a, const Math::Vector3 & b ) {
	// simply check, if point lies in bounding box (a,b), means that point is on line 
	
	Math::Vector3 min = a;
	Math::Vector3 max = b;

	// swap coordinates if needed 
	float temp;
	if( min.x > max.x ) {
		temp = min.x;
		min.x = max.x;
		max.x = temp;
	}
	if( min.y > max.y ) {
		temp = min.y;
		min.y = max.y;
		max.y = temp;
	}
	if( min.z > max.z ) {
		temp = min.z;
		min.z = max.z;
		max.z = temp;
	}
	
	return point.x <= max.x && point.y <= max.y && point.z <= max.z &&
		   point.x >= min.x && point.y >= min.y && point.z >= min.z;
}

Body::Body( Type type ) : mPosition( 0.0f, 0.0f, 0.0f ), mLinearVelocity( 0.0f, 0.0f, 0.0f ), mContactCount( 0 ),
	mType( type ), mTriangleCount( 0 ), mTriangles( nullptr ), mRadius( 0.5f ), mNext( nullptr ), mPrev( nullptr ),
	mOctree( nullptr ), mGravity( 0.0f, -0.0033f, 0.0f )
{	
	LinkedList::Append( msRoot, this );
}

void Body::AddTriangles( const Vertex * vertices, int vertexCount ) {
	int last = mTriangleCount;
	mTriangleCount += vertexCount / 3;
	mTriangles = (Triangle*)Realloc( mTriangles, mTriangleCount * sizeof( *mTriangles ) );
	for( int i = 0; i < vertexCount; i += 3 ) {
		mTriangles[ i / 3 + last] = Triangle( vertices[i].position, vertices[i+1].position, vertices[i+2].position );
	}
}

Body::~Body() {
	LinkedList::Extract( msRoot, this );
	delete mOctree;
}

void Body::DeleteAll() {
	while( msRoot ) {
		delete msRoot;
	}
}

void Body::AddContact( const Math::Vector3 & point, const Math::Vector3 & normal, Body * body ){
	mContacts[ mContactCount ].mNormal = normal;
	mContacts[ mContactCount ].mPosition = point;
	mContacts[ mContactCount ].mBody = body;
	if( mContactCount < MaxContacts - 1 ) {
		mContactCount++;
	}
}

bool Body::IsSphereRayIntersection( const Math::Ray & ray, Math::Vector3 * closestIntersectionPoint ) {
	Math::Vector3 d = ray.mBegin - mPosition;
	float a = ray.mDir.SqrLength();
	float b = 2.0f * ray.mDir.Dot( d );
	float c = d.SqrLength() - mRadius * mRadius;
	float discriminant = b * b - 4 * a * c;

	if( closestIntersectionPoint && discriminant > 0.0f ) {
		float discrRoot = Math::Sqrt( discriminant );
		
		float r1 = (-b + discrRoot) / ( 2.0f * a );
		float r2 = (-b - discrRoot) / ( 2.0f * a );
		
		Math::Vector3 p1 = ray.mBegin + ray.mDir * r1;
		Math::Vector3 p2 = ray.mBegin + ray.mDir * r2;

		if( (p1 - ray.mBegin).Length() > (p2 - ray.mBegin ).Length() ) {
			*closestIntersectionPoint = p2;
		} else {
			*closestIntersectionPoint = p1;
		}
	}

	return discriminant > 0.0f;
}

bool Body::IsSpherePointIntersection( const Math::Vector3 & point, Math::Vector3 & intersectionPoint, bool & isEdgeIntersection ) {
	if( point.SqrDistance( mPosition ) < mRadius * mRadius ) {
		isEdgeIntersection = true;
		intersectionPoint = point;
		return true;
	}
	isEdgeIntersection = false;
	return false; 
}

bool Body::IsSphereEdgeIntersection( const Math::Ray & edge, Math::Vector3 & intersectionPoint, bool & isEdgeIntersection ) {
	if( IsSphereRayIntersection( edge ) ) {
		intersectionPoint = ProjectPointOnLine( mPosition, edge.mBegin, edge.mEnd );
		if( IsPointInsideAABB( intersectionPoint, edge.mBegin, edge.mEnd ) ) {
			isEdgeIntersection = true;
			return true;
		}
	}
	isEdgeIntersection = false;
	return false;
}

bool Body::IsSphereTriangleIntersection( const Triangle & triangle, Math::Vector3 & intersectionPoint, bool & isEdgeIntersection ) {
	Math::Plane plane( triangle.mNormal, triangle.mDistance );

	float distance = plane.Distance( mPosition );

	if( distance <= mRadius ) {
		intersectionPoint = mPosition - plane.mNormal * distance;
		return triangle.CheckPoint( intersectionPoint ) ||
				IsSpherePointIntersection( triangle.mA, intersectionPoint, isEdgeIntersection ) ||
				IsSpherePointIntersection( triangle.mB, intersectionPoint, isEdgeIntersection ) ||
				IsSpherePointIntersection( triangle.mC, intersectionPoint, isEdgeIntersection ) ||
			    IsSphereEdgeIntersection( triangle.mABRay, intersectionPoint, isEdgeIntersection ) ||
			    IsSphereEdgeIntersection( triangle.mBCRay, intersectionPoint, isEdgeIntersection ) ||
			    IsSphereEdgeIntersection( triangle.mCARay, intersectionPoint, isEdgeIntersection );
    
	}

	return false;
}

void Body::SolveSpherePolygonCollision( Body * polygon ) {
	polygon->mOctree->TraceSphere( mPosition, mRadius );

	for( int i = 0; i < polygon->mOctree->mTraceBufferSize; i++ ) {
		OctreeNode * node = polygon->mOctree->mTraceBuffer[ i ];
		
		for( int k = 0; k < node->mIndexCount; k++ ) {
			Triangle & triangle = polygon->mTriangles[ node->mIndices[k] ];
			
			Math::Vector3 intersectionPoint;
			bool isEdgeIntersection;
			if( IsSphereTriangleIntersection( triangle, intersectionPoint, isEdgeIntersection ) ) {
				float length = 0.0f;

				Math::Vector3 direction = ( mPosition - intersectionPoint ).NormalizeEx( length );

				if( length > Math::Epsilon ) {
					float penetrationDepth = mRadius - length;
										
					if( penetrationDepth < 0 ) {
						continue;                
					}

					mPosition += direction * penetrationDepth;
					
					if( isEdgeIntersection ) {
						mLinearVelocity = ProjectVectorOnPlane( mLinearVelocity, direction );
					} else {
						mLinearVelocity = ProjectVectorOnPlane( mLinearVelocity, triangle.mNormal );
					}
					
					// write contact info 
					AddContact( intersectionPoint, direction, polygon );
				}
			}   
		}
	}
}

bool Body::IsSphereSphereIntersection( Body * sphere, float & outPenetrationDepth ) {
	Math::Vector3 delta = mPosition - sphere->mPosition;
	float sqrDistance = delta.SqrLength();
	float sqrRadius = (mRadius + sphere->mRadius) * (mRadius + sphere->mRadius);
	outPenetrationDepth = ( sqrRadius - sqrDistance ) / 2.0f;
	return sqrDistance <= sqrRadius;
}

void Body::SolveSphereSphereCollision( Body * sphere ) {
	float penetrationDepth;
	if( IsSphereSphereIntersection( sphere, penetrationDepth )) {
		Math::Vector3 middle = mPosition - sphere->mPosition;
		if( middle.SqrLength() > Math::Epsilon ) { 
			Math::Vector3 direction = middle.Normalized();
			Math::Vector3 offset = direction * (penetrationDepth / 2.0f);
			mPosition += offset;
			sphere->mPosition -= offset;

			// project linear velocities on a fake plane 
			mLinearVelocity = ProjectVectorOnPlane( mLinearVelocity, direction );
			sphere->mLinearVelocity = ProjectVectorOnPlane( sphere->mLinearVelocity, direction );

			// fill contact information 
			AddContact( middle, direction, sphere );
			sphere->AddContact( middle, direction, this );
		}
	}
}

void Body::SolveCollisions() {
	auto body = msRoot;
	while( body ) {
		if( body->mType != Type::Polygon ) {
			body->mLinearVelocity += body->mGravity;
			
			if( body->mContactCount > 0 ) {
				body->mLinearVelocity *= 0.968f; // friction, damping and etc.
			}

			// Medic!
			if( body->mPosition.IsNaN() ) {
				body->mPosition = Math::Vector3( 0.0f, 0.0f, 0.0f ); // позиция не болей
			}
			if( body->mLinearVelocity.IsNaN() ) {
				body->mLinearVelocity = Math::Vector3( 0.0f, 0.0f, 0.0f ); // скорость не болей
			}

			body->mPosition += body->mLinearVelocity;
			body->mContactCount = 0; 			
		}

		auto otherBody = msRoot;
		while( otherBody ) {
			if( otherBody != body ) {
				bool ignore = false;
				for( int i = 0; i < body->mIgnoreBodies.mCount; i++ ) {
					if( body->mIgnoreBodies[i] == otherBody ) {
						ignore = true;
						break;
					}
				}
				if( !ignore ) {
					if( body->mType == Type::Sphere ) {
						if( otherBody->mType == Type::Polygon ) {
							body->SolveSpherePolygonCollision( otherBody );
						}
						if( otherBody->mType == Type::Sphere ) {
							body->SolveSphereSphereCollision( otherBody );
						}
					}
				}
			}
			otherBody = otherBody->mNext;
		}

		body = body->mNext;
	}
}

void Body::PushBodies( const Math::Vector3 & point, float radius, float strength ) {
	auto body = msRoot;
	while( body ) {
		Math::Vector3 k = body->mPosition - point;
		float d2 = k.SqrLength();
		float r2 = radius * radius;
		if( d2 < r2 ) {
			body->mLinearVelocity += k * ( 1.0f - d2 / r2 ) * strength;				
		}
		body = body->mNext;
	}
}

bool TraceRay( const Math::Ray & ray, RayTraceResult & out, class Body * ignoreBody ) {
	float nearestDist = 99999.0f;
	Math::Vector3 nearest( 99999.0f, 99999.0f, 99999.0f );
	bool anyHit = false;
	auto body = Body::msRoot;
	while( body ) {
		if( ignoreBody != body ) {
			if( body->mType == Body::Type::Polygon ) {
				body->mOctree->TraceRay( ray );
				for( int i = 0; i < body->mOctree->mTraceBufferSize; i++ ) {
					OctreeNode * node = body->mOctree->mTraceBuffer[i];
					for( int k = 0; k < node->mIndexCount; k++ ) {
						Triangle & triangle = body->mTriangles[ node->mIndices[k]];
						if( triangle.IsIntersectRay( ray, out.mPosition )) {
							float d = (out.mPosition - ray.mBegin).Length();
							if( d < nearestDist ) {
								nearestDist = d;
								nearest = out.mPosition;
								out.mNormal = triangle.mNormal;
								out.mBody = body;
								anyHit = true;
							}
						}
					}
				}
			}
			if( body->mType == Body::Type::Sphere ) {
				if( body->IsSphereRayIntersection( ray, &out.mPosition )) {
					float d = (out.mPosition - ray.mBegin).Length();
					if( d < nearestDist ) {
						nearestDist = d;
						nearest = out.mPosition;
						out.mNormal = -ray.mDir.Normalized();
						out.mBody = body;
						anyHit = true;
					}
				}
			}
		}
		body = body->mNext;
	}
	out.mPosition = nearest;
	return anyHit;
}

void Body::BuildOctree() {
	mOctree = new Octree( mTriangles, mTriangleCount );
}

}