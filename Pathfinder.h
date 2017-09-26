#pragma once

const float Infinite = FLT_MAX;

class GraphVertex;

class Edge {
public:
    GraphVertex * mpDestVertex;
    float mDistToDestVertex;
    explicit Edge();
    explicit Edge( GraphVertex * destinationVertex, float distanceToDestinationVertex );
};

class GraphVertex {
private:
    bool mUsed;
    float mDistanceFromBegin;
    GraphVertex * mAncestor;    
    float DistanceToVertex( GraphVertex * vertex );
public:
	Array< Edge > mEdges;
    friend class Pathfinder;
    Math::Vector3 mPosition;
    explicit GraphVertex( Math::Vector3 pos );
    void ClearState( );
    void AddEdge( GraphVertex * vertex );
};

class Pathfinder {
private:
    Array< GraphVertex* > mGraph;
public:
    explicit Pathfinder();
    virtual ~Pathfinder();
    void SetVertices( const Array< GraphVertex* > & vertices );
    GraphVertex * GetPoint( int i );
    int GetPointCount( );
    void BuildPath( GraphVertex * begin, GraphVertex * end, Array<GraphVertex*> & outPoints );
    GraphVertex * GetVertexNearestTo( Math::Vector3 position, int * vertexNum = nullptr );
};
