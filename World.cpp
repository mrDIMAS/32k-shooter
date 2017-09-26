#include "precompiled.h"
#include "World.h"
#include "models.h"
#include "light.h"
#include "player.h"
#include "item.h"

World * World::msCurrent;
SceneNode * gSkySphere;

enum class MapType : UByte {
	Cave,
	Space,
	Forest
};

struct Map {
	MapType mType;
	const char * mName;
	UByte mWidth, mHeight;
	float mLightColorRed, mLightColorGreen, mLightColorBlue, mLightRadius;
	char * mElements; 
};


//////////////////////////////////////////////////////////////////////////////
//
// Map syntax:
//	# - wall
//	^ -	column (available only on MT_Cave, on other defines 'any other')
//	* - light (color and radius defined by map properties)
//	P - player spawn position
//  + - medium health-pack
//  $ - large health-pack
//  S - small health-pack
//	% - armor
//  B - blue flag
//  N - red flag
//  Z - red spawn point
//  X - blue spawn point
//	1 - ammo-belt
//	2 - grenades
//  3 - rockets
//  4 - rails
//  5 - plasma
//  6 - shells
//	any other - floor and ceiling on closed maps and only floor on open maps
//
//////////////////////////////////////////////////////////////////////////////

static Map maps[] = 
{
	// Test map
	{
		MapType::Cave,	// Type 
		"Cave Arena",	// Name
		20,				// Width
		20,				// Height
		1.0f,			// Light red color
		0.72f,			// Light green color
		0.54f,			// Light blue color
		6.0f,			// Light radius
		"####################"
		"#13  4  ^ 666^    P#"
		"#23*   *   *   #+% #"
		"# ^   ^  $ ^   #N+ #"
		"#   ^    Z   ^ #####"
		"# ^   ^  ^ ^       #"
		"# Z        321   Z #"
		"# 4    #######     #"
		"# $  5 #%   %*   $ #"
		"######### * ########"
		"# $    *%   %# 5 $ #"
		"# ^ ^  #######  ^  #"
		"#      123     4   #"
		"#X  ^   ^   ^   ^ X#"
		"#  4  ^   ^   ^    #"
		"#####   ^ X ^   ^  #"
		"# +B# ^   $   5    #"
		"# %+#   *   *   *23#"
		"#X     ^  666 ^  13#"
		"####################"
	},
	// Another map
	{
		MapType::Space, // Type 
			"Space Arena",	// Name
			20,				// Width
			25,				// Height
			0.8f,			// Light red color
			0.8f,			// Light green color
			1.0f,			// Light blue color
			6.0f,			// Light radius
			"####################"
			"#54  @##%+%##@   54#"
			"# X  @#$123$#@  X  #"
			"# 2  *   B   *  3  #"
			"#    #   *   #     #"
			"#    #^^^^^^^#     #"
			"# 3  @@@@@@@@@  2  #"
			"# X  *  *  * *  X  #"
			"#                  #"
			"#6*123 *+  +*321* 6#"
			"#@@@@@@@@  @@@@@@@@#"
			"#@@@@@@@@  @@@@@@@@#"
			"# *123 *+  +*321*  #"
			"#6                6#"
			"#SSS            SSS#"
			"#####          #####"
			"#@@@@   ###    @@@@#"
			"#6Z  *  *  * *  Z 6#"
			"#    @@@@@@@@@     #"
			"# 3  #^^^^^^^#  2  #"
			"#    #P  *   #     #"
			"#    *   N   *     #"
			"#42  @#$123$#@  3  #"
			"#5Z  @##%+%##@  Z45#"
			"####################"
		},
	// Another map
	{
		MapType::Forest, // Type 
		"Forest Arena",	// Name
		30,				// Width
		30,				// Height
		1.0f,			// Light red color
		0.9f,			// Light green color
		0.9f,			// Light blue color
		8.0f,			// Light radius
		"@@@@######################@@@@"
		"@@@@#12 Z % + N + %  Z 34#@@@@"
		"$@@@  *   ^ ^ ^ ^ ^    *  @@@$"
		"@@@@#   Z S$ 56 $S   Z   #@@@@"
		"##### *  ############  * #####"
		"#4^           *             4#"
		"# 5  ^ ^      P      ^ ^   5 #"
		"#  6        1 2 3         6  #"
		"#     *       *              #"
		"#  ^    $ ^ ^ ^ ^ ^ $   ^    #"
		"#                            #"
		"#    *  ^  ^  *    ^    *    #"
		"#  @@@@   ##### #####  @@@@  #"
		"#   *     #@@@@@@@@@#    *   #"
		"###       #@@@$S%@@@#  ^   ###"
		"###    ^  #@@@444@@@#  ^   ###"
		"#   *     #@@@@@@@@@#    *   #"
		"#  @@@@   ##### #####  @@@@  #"
		"#    *        *     ^   *    #"
		"#       ^                    #"
		"#   ^   $ ^ ^ ^ ^ ^ $   ^    #"		
		"#             *              #"
		"#  6        1 2 3   ^     6  #"
		"# 5  ^ ^      X      ^ ^   5 #"
		"#4       ^    *             4#"
		"##### *  ############  * #####"
		"@@@@#   X S$ 56 $S   X   #@@@@"
		"$@@@  *   ^ ^ ^ ^ ^    *  @@@$"
		"@@@@#12 X % + B + %  X 34#@@@@"
		"@@@@######################@@@@"
	},

	
};

const char * GetMapName( int i )
{
	return maps[i].mName;
}

int GetMapCount() {
	return ArraySize( maps );
}

World::World( int mapNum ) : mGridStep( 1.0f ) {
	SceneNode * node = new SceneNode;
	Map & map = maps[mapNum];
	
	if( map.mType == MapType::Cave ) {
		mScale = Math::Vector3( 1.85, 4.5, 1.85 );
	} else if( map.mType == MapType::Space ) {
		mScale = Math::Vector3( 1.85, 1, 1.85 );
	} else if( map.mType == MapType::Forest ) {
		mScale = Math::Vector3( 1.85, 1, 1.85 );
	}

	for( int i = 0; i < map.mHeight; ++i ) {
		for( int j = 0; j < map.mWidth; j++ ) {
			Surface * surf = nullptr;
			Light * light;
			Item * item;
			Item::Type itemType;
			Player * player;
			SpawnPoint * spawnPoint;
			Math::Vector3 position( j * mGridStep * mScale.x, 0.0f, i * mGridStep * mScale.z );
			Math::Vector3 surfacePosition( j * mGridStep, 0.0f, i * mGridStep );

			char s = map.mElements[ i * map.mWidth + j ];
			// Prepare for a lot (!) of goto's :)
			switch( s ) {
			case '#':
				if( map.mType == MapType::Cave ) {
					surf = new Surface( wallVertices, ArraySize( wallFaces ), wallFaces, wallTexCoords, surfacePosition, mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Mud];
				} else if( map.mType == MapType::Space ) {
					surf = Surface::MakeCube( 1, 4, 1 );
					surf->Transform( surfacePosition + Math::Vector3( 0.0f, 2.0f, 0.0f ), mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];
				} else if( map.mType == MapType::Forest ) {
					surf = Surface::MakeCube( 1, 4, 1 );
					surf->Transform( surfacePosition + Math::Vector3( 0.0f, 2.0f, 0.0f ), mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Mud];
				}
				break;		
			case '^':
				if( map.mType == MapType::Cave ) {
					surf = new Surface( cellFilledVertices, ArraySize( cellFilledFaces ), cellFilledFaces, cellFilledTexCoords, surfacePosition, mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Mud];
				} else if( map.mType == MapType::Space ) {
					surf = Surface::MakeCube( 1, 0.2f, 1 );
					surf->Transform( surfacePosition - Math::Vector3( 0.0f, 0.1f, 0.0 ), mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Metal ];

					Surface * column = Surface::MakeCylinder( 10, 0.25, 3 );
					column->Transform( surfacePosition + Math::Vector3( 0.0f, 3.0f, 0.0f ), mScale, Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0 ), 90.0f ));
					column->mTexture = Texture::msLibrary[(int)Texture::Type::Metal];

					surf->Merge( column );

					delete column;
				} else if( map.mType == MapType::Forest ) {
					surf = Surface::MakeCube( 1, 0.2f, 1 );
					surf->Transform( surfacePosition - Math::Vector3( 0.0f, 0.1f, 0.0 ), mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Grass ];

					Surface * column = Surface::MakeCylinder( 10, 0.1, 1 );
					column->Transform( surfacePosition + Math::Vector3( 0.0f, 1.0f, 0.0f ), mScale, Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0 ), 90.0f ));
					column->mTexture = Texture::msLibrary[(int)Texture::Type::Trunk];
					node->AddSurface( column );

					for( int i = 0; i < 4; ++i ) {
						Surface * cone = Surface::MakeCone( 10, 0.5 - i / 8.0f, 1 );
						cone->Transform( surfacePosition + Math::Vector3( 0.0f, 2.0f + 0.5 * i, 0.0f ), mScale, Math::Quaternion( Math::Vector3( 1.0f, 0.0f, 0.0 ), 90.0f ));
						cone->mTexture = Texture::msLibrary[(int)Texture::Type::Grass];

						node->AddSurface( cone );
					}					
				}
				break;
			case '*':
				light = new Light( map.mLightRadius, Math::Vector3( map.mLightColorRed, map.mLightColorGreen, map.mLightColorBlue ));
				light->SetLocalPosition( position + Math::Vector3( 0.0f, 0.5f, 0.0f ));
				goto InitFreeCell;
			case 'N':
				new Flag( Flag::Type::Red, position );
				goto InitFreeCell;
			case 'B':
				new Flag( Flag::Type::Blue, position );
				goto InitFreeCell;
			case 'P':
				player = new Player( position + Math::Vector3( 0.0f, 2.5f, 0.0f ));
				goto InitFreeCell;
			case 'Z':
				spawnPoint = new SpawnPoint( position + Math::Vector3( 0.0f, 0.15f, 0.0f ), Team::Red );
				goto InitFreeCell;
			case 'X':
				spawnPoint = new SpawnPoint( position + Math::Vector3( 0.0f, 0.15f, 0.0f ), Team::Blue );
				goto InitFreeCell;
			case '+':
				itemType = Item::Type::MediumHealth;
				goto CreateItem;
			case '%':
				itemType = Item::Type::YellowArmor;
				goto CreateItem;
			case '$':
				itemType = Item::Type::LargeHealth;
				goto CreateItem;
			case 'S':
				itemType = Item::Type::SmallHealth;
				goto CreateItem;
			case '1':
				itemType = Item::Type::AmmoBelt;
				goto CreateItem;
			case '2':
				itemType = Item::Type::Grenades;
				goto CreateItem;
			case '3':
				itemType = Item::Type::Rockets;
				goto CreateItem;
			case '4':
				itemType = Item::Type::Rails;
				goto CreateItem;
			case '5':
				itemType = Item::Type::Plasma;
				goto CreateItem;
			case '6':
				itemType = Item::Type::Shells;
				goto CreateItem;
			case '@':
				surf = nullptr;
				break;

			// Fffffus ro dah! So much goto's
			CreateItem:
				item = new Item( itemType );
				item->mNode->SetLocalPosition( position + Math::Vector3( 0.0f, 0.15f, 0.0f ) );
				goto InitFreeCell;

			default:
			InitFreeCell:
				if( map.mType == MapType::Cave ) {
					surf = new Surface( cellFreeVertices, ArraySize( cellFreeFaces ), cellFreeFaces, cellFreeTexCoords, surfacePosition, mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Mud];
				} else if( map.mType == MapType::Space ) {
					surf = Surface::MakeCube( 0.95, 0.2f, 0.95 );
					surf->Transform( surfacePosition - Math::Vector3( 0.0f, 0.1f, 0.0 ), mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Metal ];
				} else if( map.mType == MapType::Forest ) {
					surf = Surface::MakeCube( 1.0, 0.2f, 1.0 );
					surf->Transform( surfacePosition - Math::Vector3( 0.0f, 0.1f, 0.0 ), mScale );
					surf->mTexture = Texture::msLibrary[(int)Texture::Type::Grass ];
				}
			}

			if( surf ) {
				node->AddSurface( surf );
			}
		}
	}

	if( map.mType == MapType::Forest ) {
		gSkySphere = new SceneNode;
		gSkySphere->AddSurface( Surface::MakeSphere( 30, 30, -200 ));
		gSkySphere->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Sky ];
		gSkySphere->mAlbedo = 0.8f;
		gSkySphere->mColor = Math::Vector3( 0.0f, 0.6f, 0.95 );
		gAmbientColor = Math::Vector3( 0.5, 0.5, 0.5 );
	}

	if( map.mType == MapType::Space ) {
		gSkySphere = new SceneNode;
		gSkySphere->AddSurface( Surface::MakeSphere( 30, 30, -200 ));
		gSkySphere->mSurfaces[0]->mTexture = Texture::msLibrary[(int)Texture::Type::Space ];
		gSkySphere->mAlbedo = 0.1f;
		gSkySphere->mColor = Math::Vector3( 0.0f, 0.6f, 0.95 );
		gAmbientColor = Math::Vector3( 0.2, 0.2, 0.2 );
	}

	// merge surfaces with same texture into one surface
	node->OptimizeSurfaces();

	// for each surface
	for( int i = 0; i < node->mSurfaces.mCount; i++ ) {
		// create physical body 
		Collision::Body * body = new Collision::Body( Collision::Body::Type::Polygon );
		body->AddTriangles( node->mSurfaces[i]->mVertices, node->mSurfaces[i]->mVertexCount );
		body->BuildOctree();
		// calculate normals

		if( map.mType !=  MapType::Space && map.mType != MapType::Forest ) {
			node->mSurfaces[i]->CalculateNormals( body->mOctree );
		}
	}

	// build way map
	for( int i = 0; i < map.mHeight; ++i ) {
		for( int j = 0; j < map.mWidth; ++j ) {
			mWayMap.Append( new GraphVertex( Math::Vector3( j * mGridStep * mScale.x, 0.1f, i * mGridStep * mScale.z )));
		}
	}

	// link way map vertices
	for( int i = 0; i < map.mHeight; ++i ) {
		for( int j = 0; j < map.mWidth; ++j ) {
			int index = i * map.mWidth + j;
			// do not link walls and colums
			if( map.mElements[ index ] != '#' && map.mElements[ index ] != '^' && map.mElements[ index ] != '@' ) {
				auto v = mWayMap[ index ];

				// up
				if( i - 1 > 0 ) {
					int index2 = (i - 1) * map.mWidth + j ;
					if( map.mElements[ index2 ] != '#' && map.mElements[ index2 ] != '^'  && map.mElements[ index ] != '@') {
						v->AddEdge( mWayMap[ index2 ] );
					}
				}
				// bottom
				if( i + 1 < map.mHeight - 1 ) {
					int index2 = (i + 1) * map.mWidth + j ;
					if( map.mElements[ index2 ] != '#' && map.mElements[ index2 ] != '^'  && map.mElements[ index ] != '@') {
						v->AddEdge( mWayMap[ index2 ] );
					}
				}
				// left
				if( j - 1 > 0 ) {
					int index2 = i * map.mWidth + ( j - 1 ) ;
					if( map.mElements[ index2 ] != '#' && map.mElements[ index2 ] != '^'  && map.mElements[ index ] != '@') {
						v->AddEdge( mWayMap[ index2 ] );
					}
				}
				// right
				if( j + 1 < map.mWidth - 1 ) {
					int index2 = i * map.mWidth + ( j + 1 ) ;
					if( map.mElements[ index2 ] != '#' && map.mElements[ index2 ] != '^'  && map.mElements[ index ] != '@' ) {
						v->AddEdge( mWayMap[ index2 ] );
					}
				}
			}
		}
	}

	for( int i = 0; i < mWayMap.mCount;  ) {
		if( mWayMap[i]->mEdges.mCount > 0 ) {
			++i;
		} else {
			i = mWayMap.Remove( mWayMap[i] );
		}
	}

	msCurrent = this;

	gRoundOver = false;
}

World::~World() {
	msCurrent = nullptr;

	for( int i = 0; i < mWayMap.mCount; ++i ) {
		delete mWayMap[i];
	}
}
