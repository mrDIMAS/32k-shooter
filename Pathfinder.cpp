#include "Precompiled.h"

#include "Pathfinder.h"

void Pathfinder::BuildPath( GraphVertex * begin, GraphVertex * end, Array< GraphVertex* > & outPoints ) {    
	outPoints.Clear();
	
	if( begin == end ) {
		outPoints.Append( begin );
		return;
	}

    // clear state of all vertices
    for( int i = 0; i < mGraph.mCount; ++i ) {
        mGraph[i]->ClearState();
    }

    // set begin graph vertex
    begin->mDistanceFromBegin = 0;

    for( int i = 0; i < mGraph.mCount; i++ ) {
        // get nearest vertex
        GraphVertex * nearest = nullptr;

        for( int j = 0; j < mGraph.mCount; ++j ) {
            if( mGraph[j]->mUsed ) {
                continue;
            } else if( !nearest ) {
                nearest = mGraph[j];
            } else if( mGraph[j]->mDistanceFromBegin < nearest->mDistanceFromBegin ) {
                nearest = mGraph[j];
            }
        }

        if( nearest->mDistanceFromBegin >= Infinite ) {
            break;
        }

        nearest->mUsed = true;

        // relaxation
		for( int j = 0; j < nearest->mEdges.mCount; ++j ) {
			Edge * edge = &nearest->mEdges[j];
            if( nearest->mDistanceFromBegin + edge->mDistToDestVertex < edge->mpDestVertex->mDistanceFromBegin ) {
                edge->mpDestVertex->mDistanceFromBegin = nearest->mDistanceFromBegin + edge->mDistToDestVertex;
                edge->mpDestVertex->mAncestor = nearest;
            }
        }
    }

    // restore path to dest vertex
    for( GraphVertex * v = end; v != begin;  ) {
		if( v ) {
			outPoints.Append( v );
			v = v->mAncestor;
		} else {
			break;
		}		
    }

    outPoints.Append( begin );

    outPoints.Reverse();
}

int Pathfinder::GetPointCount() {
    return mGraph.mCount;
}

GraphVertex * Pathfinder::GetPoint( int i ) {
    if( i < 0 || i >= mGraph.mCount ) {
        return nullptr;
    } else {
        return mGraph[i];
    }
}

void Pathfinder::SetVertices( const Array< GraphVertex* > & vertices ) {
	for( int i = 0; i < vertices.mCount; ++i ) {
		mGraph.Append( vertices.mElements[i] );
	}
}

Pathfinder::~Pathfinder() {

}

Pathfinder::Pathfinder() {

}

GraphVertex * Pathfinder::GetVertexNearestTo( Math::Vector3 position, int * vertexNum ) {
    GraphVertex * nearest = mGraph[0];
    int nearestIndex = 0;
    for( int i = 0; i < mGraph.mCount; ++i ) {
		if( mGraph[i]->mPosition.SqrDistance( position ) < nearest->mPosition.SqrDistance( position )) {
			nearest = mGraph[i];
			nearestIndex = i;
		}        
    }
    if( vertexNum ) {
        *vertexNum = nearestIndex;
    }
    return nearest;
}

void GraphVertex::AddEdge( GraphVertex * vertex ) {
    mEdges.Append( Edge( vertex, DistanceToVertex( vertex )));

    // ??? vertex->mEdges.Append( Edge( this, DistanceToVertex( vertex )));
}

void GraphVertex::ClearState() {
    mAncestor = nullptr;
    mUsed = false;
    mDistanceFromBegin = Infinite;
}

GraphVertex::GraphVertex( Math::Vector3 pos ) : mPosition( pos ) {
    ClearState();
}

float GraphVertex::DistanceToVertex( GraphVertex * vertex ) {
    return ( mPosition - vertex->mPosition ).Length();
}

Edge::Edge( GraphVertex * destinationVertex, float distanceToDestinationVertex ) {
    mpDestVertex = destinationVertex;
    mDistToDestVertex = distanceToDestinationVertex;
}

Edge::Edge() {
    mpDestVertex = nullptr;
    mDistToDestVertex = Infinite;
}

