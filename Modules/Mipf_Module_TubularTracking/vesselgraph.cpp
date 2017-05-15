#include "VesselGraph.h"
#include <map>

void copyBoostGraph(boostGraph& targetGraph, const Vertex targetInsertVertex, const boostGraph& sourceGraph, const Vertex sourceRoot)
{

  // Check if the target graph is empty. In such case the targetInsertVertex will not be used
  bool targetIsEmpty = (boost::num_vertices(targetGraph) == 0)? true : false;

  // Find all source vertexes
  std::vector<Vertex> sourceVertexes(0);
  boost::breadth_first_search(sourceGraph, sourceRoot, boost::visitor(GetVertexes(&sourceVertexes)));

  // Create vertexes in target graph
  unsigned int nbrOfVertexes = sourceVertexes.size();
  std::map<Vertex,Vertex> vertexMapping;
  for(unsigned int i=0; i<nbrOfVertexes; ++i) {
    // Add new vertex to target graph
    Vertex newVertex = boost::add_vertex(targetGraph);

    // Copy data from source vertex to target vertex
    targetGraph[newVertex] = sourceGraph[sourceVertexes[i]];

    // Associate target and source vertexes
    vertexMapping[sourceVertexes[i]] = newVertex;
  }

  // The newly inserted vertexes are not yet connected to the other vertexes in the target graph.
  // If the target graph was not empty from the beginning, add this connection
  if(!targetIsEmpty) {
    boost::add_edge(targetInsertVertex,vertexMapping[sourceRoot], targetGraph);
  }

  // Add edges between the newly created vertexes in the target graph
  for(unsigned int i=0; i<nbrOfVertexes; ++i) {

      // Get iterator over the out edges of the current vertex
      std::pair<itOutEdge, itOutEdge> itOutEdgePair = boost::out_edges(sourceVertexes[i],sourceGraph);

      // Loop over the out edges
      for( ; itOutEdgePair.first != itOutEdgePair.second; ++itOutEdgePair.first)
      {

        // Get prev and succ vertexes in the source graph
        Vertex prevVertex = boost::source(*itOutEdgePair.first,sourceGraph);
        Vertex succVertex = boost::target(*itOutEdgePair.first,sourceGraph);

        // Add a corresponding edge in the target graph
        boost::add_edge(vertexMapping[prevVertex], vertexMapping[succVertex], targetGraph);
      }
  }
}
