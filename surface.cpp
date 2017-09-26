#include "precompiled.h"
#include "surface.h"
#include "octree.h"

int Surface::msVBOCount;

#define MakeVertex( surf, cx, cy, cz, tx, ty )	\
	v = surf->AddVertex();						\
	v->position.x = cx;							\
	v->position.y = cy;							\
	v->position.z = cz;							\
	v->texCoord.x = tx;							\
	v->texCoord.y = ty;							\

#define MakeVertex2( surf, cx, cy, cz, tx, ty, nx, ny, nz )	\
	v = surf->AddVertex();									\
	v->position.x = cx;										\
	v->position.y = cy;										\
	v->position.z = cz;										\
	v->texCoord.x = tx;										\
	v->texCoord.y = ty;										\
	v->normal.x = nx;										\
	v->normal.y = ny;										\
	v->normal.z = nz;										\

Surface::Surface( int countFaces ) {
	Init( countFaces );
}

Surface::Surface( PackedVertex * packedVertices, int faceCount, Face * faces, PackedTexCoord * packedTexCoords, const Math::Vector3 & offset, const Math::Vector3 & scale ) {
	Init( faceCount );

	for( int i = 0, k = 0; i < mVertexCount; i += 3, k++  ) {
		auto & va = mVertices[i];
		auto & vb = mVertices[i+1];
		auto & vc = mVertices[i+2];

		// Unpack A
		va.position.x = ( ExpandToFloat( packedVertices[ faces[k].a ].x ) + offset.x ) * scale.x;
		va.position.y = ( ExpandToFloat( packedVertices[ faces[k].a ].y ) + offset.y ) * scale.y;
		va.position.z = ( ExpandToFloat( packedVertices[ faces[k].a ].z ) + offset.z ) * scale.z;
		va.texCoord.x = ExpandToFloat( packedTexCoords[ faces[k].ta ].x );
		va.texCoord.y = ExpandToFloat( packedTexCoords[ faces[k].ta ].y );
		va.color = Math::Color( 255, 255, 255, 255 );

		// Unpack B		
		vb.position.x = ( ExpandToFloat( packedVertices[ faces[k].b ].x ) + offset.x ) * scale.x;
		vb.position.y = ( ExpandToFloat( packedVertices[ faces[k].b ].y ) + offset.y ) * scale.y;
		vb.position.z = ( ExpandToFloat( packedVertices[ faces[k].b ].z ) + offset.z ) * scale.z;
		vb.texCoord.x = ExpandToFloat( packedTexCoords[ faces[k].tb ].x );
		vb.texCoord.y = ExpandToFloat( packedTexCoords[ faces[k].tb ].y );
		vb.color = Math::Color( 255, 255, 255, 255 );

		// Unpack C
		vc.position.x = ( ExpandToFloat( packedVertices[ faces[k].c ].x ) + offset.x ) * scale.x;
		vc.position.y = ( ExpandToFloat( packedVertices[ faces[k].c ].y ) + offset.y ) * scale.y;
		vc.position.z = ( ExpandToFloat( packedVertices[ faces[k].c ].z ) + offset.z ) * scale.z;
		vc.texCoord.x = ExpandToFloat( packedTexCoords[ faces[k].tc ].x );
		vc.texCoord.y = ExpandToFloat( packedTexCoords[ faces[k].tc ].y );
		vc.color = Math::Color( 255, 255, 255, 255 );

		va.normal = ((vc.position - va.position).Cross( vb.position - va.position )).Normalized();
		vb.normal = va.normal;
		vc.normal = va.normal;
	}

	UpdateVertexBuffer();
}

Surface::~Surface() {
	--Surface::msVBOCount;
	GLCall( gl.DeleteBuffersARB( 1, &mVBO ));
	delete mVertices;
}

void Surface::UpdateVertexBuffer() {
	if( mNeedUpdate ) {
		GLCall( gl.BindBufferARB( GL_ARRAY_BUFFER_ARB, mVBO ));
		GLCall( gl.BufferDataARB( GL_ARRAY_BUFFER_ARB, mVertexCount * sizeof( Vertex ), mVertices, GL_STATIC_DRAW_ARB ));
		GLCall( gl.BindBufferARB( GL_ARRAY_BUFFER_ARB, 0 ));
		mNeedUpdate = false;
	}
}

void Surface::CalculateNormals( Octree * octree ) {
	auto * tempNormals = new Math::Vector3[ mVertexCount / 3 ];

	for ( int i = 0; i < mVertexCount; i += 3 ) {    
		auto & vA = mVertices[ i].position;
		auto & vB = mVertices[ i + 1 ].position;
		auto & vC = mVertices[ i + 2 ].position;

		tempNormals[ i / 3 ] = (( vC - vA ).Cross( vB - vA )).Normalize();
	};


	for( int i = 0; i < mVertexCount; i++ ) {
		Math::Vector3 sum( 0.0f, 0.0f, 0.0f );
			
		if( octree ) {
			octree->TracePoint( mVertices[i].position );
			for( int j = 0; j < octree->mTraceBufferSize; j++ ) {
				OctreeNode * node = octree->mTraceBuffer[j];
				for( int k = 0; k < node->mIndexCount; k++ ) {
					if( mVertices[i].position.Equals( mVertices[node->mIndices[k] * 3].position ) ||
						mVertices[i].position.Equals( mVertices[node->mIndices[k] * 3 + 1].position ) ||
						mVertices[i].position.Equals( mVertices[node->mIndices[k] * 3 + 2].position )) {
							sum += tempNormals[ node->mIndices[k] ];
					}			
				}
			}
		} else {
			for( int j = 0; j < mVertexCount; j += 3 ) {
				if( mVertices[i].position.Equals( mVertices[ j ].position ) ||
					mVertices[i].position.Equals( mVertices[ j + 1 ].position ) ||
					mVertices[i].position.Equals( mVertices[ j + 2 ].position )) {
						sum += tempNormals[ j / 3 ];
				}			
			}
		}

		mVertices[i].normal = sum.Normalize();
	}

	delete [] tempNormals;
}

void Surface::Merge( Surface * other ) {
	int last = mVertexCount;
	mVertexCount += other->mVertexCount;
	mVertices = (Vertex*)Realloc( mVertices, mVertexCount * sizeof( *mVertices ) );
	for( int i = 0; i < other->mVertexCount; i++ ) {
		mVertices[i + last] = other->mVertices[i];
	}
}

void Surface::Render() {
	if( !mNoTextures ) {
		if( mTexture ) {
			mTexture->Bind();
		} else {
			mTexture->msLibrary[(int)Texture::Type::White]->Bind();
		}
	}

	GLCall( glEnableClientState( GL_VERTEX_ARRAY ));
	GLCall( glEnableClientState( GL_NORMAL_ARRAY ));
	GLCall( glEnableClientState( GL_COLOR_ARRAY ));

	UpdateVertexBuffer();

	GLCall( gl.BindBufferARB( GL_ARRAY_BUFFER_ARB, mVBO ));
	GLCall( glVertexPointer( 3, GL_FLOAT, sizeof( Vertex ), 0 ));

	GLCall( glNormalPointer( GL_FLOAT, sizeof( Vertex ), (void*)offsetof( Vertex, normal )));

	GLCall( gl.ClientActiveTextureARB( GL_TEXTURE0_ARB ));
	GLCall( glEnableClientState( GL_TEXTURE_COORD_ARRAY ));
	GLCall( glTexCoordPointer( 2, GL_FLOAT, sizeof( Vertex ), (void*)offsetof( Vertex, texCoord )));

	GLCall( glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( Vertex ), (void*)offsetof( Vertex, color )));

	GLCall( glDrawArrays( GL_TRIANGLES, 0, mVertexCount ));		

	GLCall( gl.BindBufferARB( GL_ARRAY_BUFFER_ARB, 0 ));
	GLCall( glDisableClientState( GL_VERTEX_ARRAY ));
	GLCall( glDisableClientState( GL_NORMAL_ARRAY ));
	GLCall( glDisableClientState( GL_COLOR_ARRAY ));
}

Surface* Surface::MakeSphere( int slices, int stacks, float r ) {
	Vertex * v;

	auto * surf = new Surface;

	using namespace Math;

	float dTheta = Pi / slices;
	float dPhi = 2.0f * Pi / stacks;
	float dTCy = 1.0f / stacks;
	float dTCx = 1.0f / slices;

	for( int i = 0; i < stacks; ++i ) {
		for( int j = 0; j < slices; ++j ) {

			int nj = j+1;
			int ni = i+1;

			float k0 = r * Sin( dTheta * i );
			float k1 = Cos( dPhi * j );
			float k2 = Sin( dPhi * j );
			float k3 = r * Cos( dTheta * i );

			float k4 = r * Sin( dTheta * ni );
			float k5 = Cos( dPhi * nj );
			float k6 = Sin( dPhi * nj );
			float k7 = r * Cos( dTheta * ni );

			if( i != (stacks - 1)) {
				MakeVertex( surf, k0 * k1, k0 * k2, k3, dTCx * j, dTCy * i );
				MakeVertex( surf, k4 * k1, k4 * k2, k7, dTCx * j, dTCy * ni );
				MakeVertex( surf, k4 * k5, k4 * k6, k7, dTCx * nj, dTCy * ni );
			}

			if( i != 0 ) {
				MakeVertex( surf, k4 * k5, k4 * k6, k7, dTCx * nj, dTCy * ni );
				MakeVertex( surf, k0 * k5, k0 * k6, k3, dTCx * nj, dTCy * i );
				MakeVertex( surf, k0 * k1, k0 * k2, k3, dTCx * j, dTCy * i );
			}
		}
	}

	surf->CalculateNormals( nullptr );

	return surf;
}

Surface* Surface::MakeCylinder( int sides, float r, float h ) {
	Vertex * v;

	auto * surf = new Surface;

	using namespace Math;

	float dPhi = 2.0f * Pi / sides;
	float dT = 1.0f / sides;

	for( int i = 0; i < sides; ++i ) {
		float nx0 = Cos( dPhi * i );
		float ny0 = Sin( dPhi * i );
		float nx1 = Cos( dPhi * (i+1) );
		float ny1 = Sin( dPhi * (i+1) );

		float x0 = r * nx0;
		float y0 = r * ny0;
		float x1 = r * nx1;
		float y1 = r * ny1;
		float tx0 = dT * i;
		float tx1 = dT * (i+1);

		// front cap
		MakeVertex2( surf, 0.0f, 0.0f, h, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f );
		MakeVertex2( surf, x0, y0, h, tx0, 1.0f, 0.0f, 0.0f, -1.0f );
		MakeVertex2( surf, x1, y1, h, tx1, 1.0f, 0.0f, 0.0f, -1.0f );

		// back cap
		MakeVertex2( surf, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );
		MakeVertex2( surf, x1, y1, 0.0f, tx0, 1.0f, 0.0f, 0.0f, 1.0f );
		MakeVertex2( surf, x0, y0, 0.0f, tx1, 1.0f, 0.0f, 0.0f, 1.0f );

		// sides
		MakeVertex2( surf, x1, y1, 0.0f, tx1, 0.0f, -nx1, -ny1, 0.0f );
		MakeVertex2( surf, x0, y0, h, tx0, 1.0f, -nx0, -ny0, 0.0f );
		MakeVertex2( surf, x0, y0, 0.0f, tx0, 0.0f, -nx0, -ny0, 0.0f );

		MakeVertex2( surf, x1, y1, h, tx1, 1.0f, -nx1, -ny1, 0.0f );
		MakeVertex2( surf, x0, y0, h, tx0, 1.0f, -nx0, -ny0, 0.0f );
		MakeVertex2( surf, x1, y1, 0.0f, tx1, 0.0f, -nx1, -ny1, 0.0f );
	}

	return surf;
}

Surface* Surface::MakeCone( int sides, float r, float h ) {
	Vertex * v;

	auto * surf = new Surface;

	using namespace Math;

	float dPhi = 2.0f * Pi / sides;
	float dT = 1.0f / sides;

	for( int i = 0; i < sides; ++i ) {
		float nx0 = Cos( dPhi * i );
		float ny0 = Sin( dPhi * i );
		float nx1 = Cos( dPhi * (i+1) );
		float ny1 = Sin( dPhi * (i+1) );

		float x0 = r * nx0;
		float y0 = r * ny0;
		float x1 = r * nx1;
		float y1 = r * ny1;
		float tx0 = dT * i;
		float tx1 = dT * (i+1);

		// back cap
		MakeVertex2( surf, 0.0f, 0.0f, h, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f );
		MakeVertex2( surf, x0, y0, h, tx0, 1.0f, 0.0f, 0.0f, -1.0f );
		MakeVertex2( surf, x1, y1, h, tx1, 1.0f, 0.0f, 0.0f, -1.0f );

		// sides
		MakeVertex2( surf, 0.0f, 0.0f, 0.0f, tx1, 0.0f, 0.0f, 0.0f, 1.0f );
		MakeVertex2( surf, x1, y1, h, tx0, 1.0f, -nx1, -ny1, 0.0f );
		MakeVertex2( surf, x0, y0, h, tx0, 0.0f, -nx0, -ny0, 0.0f );
	}


	return surf;
}

Surface * Surface::MakeCube( float w, float h, float d ) {
	Vertex * v;

	PackedVertex cubeVertices[] = {
		{ -10, -10, 10 },
		{ -10, -10, -10 },
		{ 10, -10, -10 },
		{ 10, -10, 10 },
		{ -10, 10, 10 },
		{ 10, 10, 10 },
		{ 10, 10, -10 },
		{ -10, 10, -10 },
	};

	PackedTexCoord cubeTexCoords[] = {
		{ 20, 0 },
		{ 20, 20 },
		{ 0, 20 },
		{ 0, 0 },
	};

	Face cubeFaces[] = {
		{ 0, 1, 2, 0, 1, 2 },
		{ 2, 3, 0, 2, 3, 0 },
		{ 4, 5, 6, 3, 0, 1 },
		{ 6, 7, 4, 1, 2, 3 },
		{ 0, 3, 5, 3, 0, 1 },
		{ 5, 4, 0, 1, 2, 3 },
		{ 3, 2, 6, 3, 0, 1 },
		{ 6, 5, 3, 1, 2, 3 },
		{ 2, 1, 7, 3, 0, 1 },
		{ 7, 6, 2, 1, 2, 3 },
		{ 1, 0, 4, 3, 0, 1 },
		{ 4, 7, 1, 1, 2, 3 },
	};

	return new Surface( cubeVertices, ArraySize( cubeFaces ), cubeFaces, cubeTexCoords, Math::Vector3( 0.0f, 0.0f, 0.0f ), Math::Vector3( w, h, d ));
}

/*
Surface* Surface::MakePyramid( float dw, float uw, float dd, float ud, float h ) {

}
*/
Vertex * Surface::AddVertex() {
	++mVertexCount;
	mVertices = (Vertex*)Realloc( mVertices, mVertexCount * sizeof( *mVertices ));
	return &mVertices[mVertexCount-1];
}

void Surface::Transform( const Math::Vector3 & offset, const Math::Vector3 & scale, const Math::Quaternion & rotation ) {
	Math::Matrix transform;
	transform.SetRotationOrigin( rotation, offset );
	Math::Matrix mScale;
	mScale.Scale( scale );
	transform = transform * mScale;
	for( int i = 0; i < mVertexCount; ++i ) {
		mVertices[i].position = transform.TransformVector( mVertices[i].position );
	}
}

void Surface::Init( int countFaces ) {
	mNeedUpdate = true;
	GLCall( gl.GenBuffersARB( 1, &mVBO ));

	mVertexCount = countFaces * 3;

	if( countFaces ) {
		mVertices = new Vertex[ mVertexCount ];
	} else {
		mVertices = nullptr;
	}

	mNoTextures = false;
	mTexture = nullptr;
	mUserFlag = 0;


	++Surface::msVBOCount;
}

