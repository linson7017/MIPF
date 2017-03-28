#ifndef __VesselTrackingGraph_H
#define __VesselTrackingGraph_H

#include "VesselData.h"

// Includes from boost library
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>

// Create a graph type where each vertex contains a templateData and each
// edge a double (for example the length). // TODO: How to define empty data structure on edges
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VesselData, int> boostGraph;
typedef boost::graph_traits<boostGraph>::vertex_descriptor Vertex;
typedef boost::graph_traits<boostGraph>::edge_descriptor Edge;
typedef boost::graph_traits<boostGraph>::out_edge_iterator itOutEdge;
typedef std::vector<std::pair<Vertex,double> >  VertexScorePairs;




// A boost graph visitor that simply gathers all vertexes in a list.
// Is typically used to find all vertexes in a subgraph.
class GetVertexes : public boost::default_bfs_visitor
{
public:
  GetVertexes(std::vector<Vertex>* vertexes) : _vertexes(vertexes) {_vertexes->clear(); }

  void discover_vertex(Vertex v, const boostGraph& /*g*/)
  {
    _vertexes->push_back(v);
  }

private:
  std::vector<Vertex>* _vertexes;
};


// Declaration
void copyBoostGraph(boostGraph& targetGraph, const Vertex targetRoot, const boostGraph& sourceGraph, const Vertex sourceRoot);

// The following Matlab code is used to generate the
// "hot" colorscale:
// >> h = hot(50);h=h(11:end-10,:);imagesc([1:30]),colormap(h)
const int NBROFCOLORS = 30;
const double hot[NBROFCOLORS][3] = {{0.6111, 0, 0}, {0.6667, 0, 0},
{0.7222,         0,         0},
{0.7778,         0,         0},
{0.8333,         0,         0},
{0.8889,         0,         0},
{0.9444,         0,         0},
{1.0000,         0,         0},
{1.0000,    0.0556,         0},
{1.0000,    0.1111,         0},
{1.0000,    0.1667,         0},
{1.0000,    0.2222,         0},
{1.0000,    0.2778,         0},
{1.0000,    0.3333,         0},
{1.0000,    0.3889,         0},
{1.0000,    0.4444,         0},
{1.0000,    0.5000,         0},
{1.0000,    0.5556,         0},
{1.0000,    0.6111,         0},
{1.0000,    0.6667,         0},
{1.0000,    0.7222,         0},
{1.0000,    0.7778,         0},
{1.0000,    0.8333,         0},
{1.0000,    0.8889,         0},
{1.0000,    0.9444,         0},
{1.0000,    1.0000,         0},
{1.0000,    1.0000,    0.0714},
{1.0000,    1.0000,    0.1429},
{1.0000,    1.0000,    0.2143},
{1.0000,    1.0000,    0.2857}};


//! The FindVertexPaths class is used to find the paths from a root vertex to all
//! other vertexes connected to the root, i.e., the path of vertexes one has to pass
//! from the root to the vertexes. The paths for the vertexes are stored in a C++ map<>
//! structure, where the key is the leaf vertex and the associated value is a vector<>
//! of vertexes (that is, the path). See the graph boost documentation for more
//! information on breadth-first-search (bfs) visitors.
class FindVertexPaths : public boost::default_bfs_visitor
{
public:

  //! Constructor.
  FindVertexPaths(std::map<Vertex, std::vector<Vertex> >* vertexPaths, const Vertex& root) :
  _vertexPaths(vertexPaths) // Set private member variable pointer to the input map<> structure.
  {
    //! Add the path to the root vertex to _vertexPaths
    std::vector<Vertex> rootVertex(1);
    rootVertex[0] = root;
    _vertexPaths->insert(std::make_pair(root,rootVertex));
  }

  //! When a new edge is found in the graph, update the path
  //! between source vertex and target vertex.
  void tree_edge(Edge e, const boostGraph& g) {
    // Get path to source vertex.
    std::vector<Vertex> path = (*_vertexPaths)[boost::source(e,g)];
    //! Add target vertex to the path.
    path.push_back(boost::target(e,g));
    //! Add new entry in the map<> structure where the target
    //! vertex is associated with the updated path.
    _vertexPaths->insert(std::make_pair(boost::target(e,g),path));
  }

private:
  std::map<Vertex, std::vector<Vertex> >* _vertexPaths;
};



//! This class searches the track graph and find the leafs
//! (i.e., endpoints). In addition, the score along the paths
//! to the leafs are calculated.
class ActiveLeafRecorder : public boost::default_bfs_visitor
{
public:
  ActiveLeafRecorder(std::vector<std::pair<Vertex,double> >* leafs, int* depth) :
      _leafs(leafs), _depth(depth), _depthSet(false) {}

  void discover_vertex(Vertex v, const boostGraph& g) {

    // Are the _vertexSumScore and _vertexDepth empty? In
    // such case is v the start vertex and it should be added
    // to these maps.
    if(_vertexSumScore.size() == 0) {
      _vertexSumScore.insert(std::make_pair(v,g[v].getScore()));
      _vertexDepth.insert(std::make_pair(v,0));
    }

    // Get the sumScore of this vertex
    double sumScore = _vertexSumScore[v];

    // If the current vertex has no output edges and the terminator
    // flag is not marked, it is an active leaf that should be recorded.
    if( ((int)boost::out_degree(v, g) == 0) && (!g[v].isTerminator())) {

      // If this is the first leaf we encounter, _depthSet equals false,
      // and we record the depth of the leaf. If we have encountered
      // leafs previously, _depthSet is true, and we check if the previous
      // depths are the same as for the depth of the current leaf. This
      // can be used to check that the graph is correctly built.
      if(!_depthSet) {
        *_depth = _vertexDepth[v];
        _depthSet = true;
      }
      else if (*_depth != _vertexDepth[v]) {
        *_depth = -1;                         // Warning: Unequal depths, _depth is set to -1
      }

      // Add leaf to leaf-list�.
      _leafs->push_back(std::make_pair(v,sumScore));
    }

    // Otherwise, the vertex is an intermediate vertex and we then propagate
    // the sumScore of this vertex to the connected vertexes
    else {

      // Get iterator over the out edges of the current vertex
      std::pair<itOutEdge, itOutEdge> itOutEdgePair = boost::out_edges(v,g);

      // Loop over the out edges
      for( ; itOutEdgePair.first != itOutEdgePair.second; ++itOutEdgePair.first) {

        // Get depth of this vertex
        unsigned int depth = _vertexDepth[v];

        // Get the adjacent vertex connected via this edge
        Vertex connectedVertex = boost::target(*itOutEdgePair.first,g);

        // Get score attached to this vertex
        double score = g[connectedVertex].getScore();

        // Set the sumScore of the adjacent vertex to the sum of the sumScore
        // of the current vertex plus the score attaced to the edge.
        _vertexSumScore.insert(std::make_pair(connectedVertex, sumScore + score));

        // Set the depth of the adjacent vertex to the current depth + 1
        _vertexDepth.insert(std::make_pair(connectedVertex,depth+1));
      }
    }
  }

private:
  VertexScorePairs* _leafs;
  int* _depth;
  bool _depthSet;
  std::map<Vertex, double> _vertexSumScore;
  std::map<Vertex, int> _vertexDepth;
};


#endif
