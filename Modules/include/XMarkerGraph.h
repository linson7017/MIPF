#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include "XMarker.h"

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, XMarker, int> XMarkerGraph;
typedef boost::graph_traits<boostGraph>::vertex_descriptor XMarkerVertex;
typedef boost::graph_traits<boostGraph>::edge_descriptor XMarkerEdge;
typedef boost::graph_traits<boostGraph>::out_edge_iterator XMarkeritOutEdge;
typedef std::vector<std::pair<Vertex,double> >  XMarkerVertexScorePairs;