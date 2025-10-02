#ifndef BALTAZAR_DAG_API
#define BALTAZAR_DAG_API

#include "../../src/dag/dag.hpp"

namespace baltazar {

using INode = dag::INode;

template <size_t NUM_OF_EDGES, typename FUNCTOR>
using Node = dag::Node<NUM_OF_EDGES, FUNCTOR>;

using SortType = dag::SortType;

template <size_t NUM_OF_NODES> using NodeList = dag::NodeList<NUM_OF_NODES>;

} // namespace baltazar

#endif
