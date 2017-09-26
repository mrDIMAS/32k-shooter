#pragma once


#include "texture.h"

class Surface {
private:
	GLuint mVBO;
	void Init( int countFaces );
public:
	int mVertexCount;
	Vertex * mVertices;
	Texture * mTexture;
	bool mNeedUpdate;
	int mUserFlag;
	bool mNoTextures;
	static int msVBOCount;
	

	Surface( int countFaces = 0 );
	Surface( PackedVertex * packedVertices, int faceCount, Face * faces, PackedTexCoord * packedTexCoords, const Math::Vector3 & offset, const Math::Vector3 & scale );
	~Surface();
	void Merge( Surface * other );
	void CalculateNormals( class Octree * octree );
	void UpdateVertexBuffer();
	void Render();
	Vertex * AddVertex();
	void Transform( const Math::Vector3 & offset, const Math::Vector3 & scale, const Math::Quaternion & rotation = Math::Quaternion( 0, 0, 0, 1 ));
	static Surface* MakeCube( float w, float h, float d );
	static Surface* MakeCylinder( int sides, float r, float h );
	static Surface* MakeCone( int sides, float r, float h );
	static Surface* MakeSphere( int slices, int stacks, float r );
	/*
	static Surface* MakePyramid( float dw, float uw, float dd, float ud, float h );*/
};