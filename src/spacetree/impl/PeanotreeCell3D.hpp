// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#ifndef PRECICE_SPACETREE_PEANOTREECELL3D_HPP_
#define PRECICE_SPACETREE_PEANOTREECELL3D_HPP_

#include "spacetree/Spacetree.hpp"
#include "mesh/Group.hpp"
#include "utils/PointerVector.hpp"
#include "tarch/logging/Log.h"
#include "utils/Helpers.hpp"
#include "query/FindVoxelContent.hpp"
#include "utils/Dimensions.hpp"

namespace precice {
namespace spacetree {
namespace impl {

const precice::utils::DynVector PEANO_FINE_CELL_CENTER_POSITIONS_3D[27] =
{
  utils::DynVector(utils::Vector3D(-1.0/3.0, -1.0/3.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D(     0.0, -1.0/3.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0, -1.0/3.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0,      0.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D(     0.0,      0.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0,      0.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0,  1.0/3.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D(     0.0,  1.0/3.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0,  1.0/3.0, -1.0/3.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0, -1.0/3.0,      0.0)),
  utils::DynVector(utils::Vector3D(     0.0, -1.0/3.0,      0.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0, -1.0/3.0,      0.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0,      0.0,      0.0)),
  utils::DynVector(utils::Vector3D(     0.0,      0.0,      0.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0,      0.0,      0.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0,  1.0/3.0,      0.0)),
  utils::DynVector(utils::Vector3D(     0.0,  1.0/3.0,      0.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0,  1.0/3.0,      0.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0, -1.0/3.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D(     0.0, -1.0/3.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0, -1.0/3.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0,      0.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D(     0.0,      0.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0,      0.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D(-1.0/3.0,  1.0/3.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D(     0.0,  1.0/3.0,  1.0/3.0)),
  utils::DynVector(utils::Vector3D( 1.0/3.0,  1.0/3.0,  1.0/3.0))
};

class PeanotreeCell3D
{
public:

  PeanotreeCell3D();

  ~PeanotreeCell3D();

  bool isLeaf() const
  {
    return _content != NULL;
  }

  mesh::Group& content()
  {
    assertion(_content != NULL);
    return *_content;
  }

  bool needsRefinement (
    const utils::DynVector& cellHalflengths,
    double                  refinementLimit );

  int getPosition() const
  {
    return _position;
  }

  void setPosition ( int position )
  {
    _position = position;
  }

  void refine (
    const utils::DynVector& cellCenter,
    const utils::DynVector& cellHalflengths );

  int getChildCount()
  {
    return _childs.size();
  }

  PeanotreeCell3D& child ( int index )
  {
    assertion2 ( (index >= 0) && (index < (int)_childs.size()), index, _childs.size() );
    return _childs[index];
  }

  int getChildIndex (
    const utils::DynVector& searchPoint,
    const utils::DynVector& cellCenter,
    const utils::DynVector& cellHalflengths );

  void getChildData (
    int                     childIndex,
    const utils::DynVector& cellCenter,
    const utils::DynVector& cellHalflengths,
    utils::DynVector&       childCenter,
    utils::DynVector&       childHalflengths );

  void accept (
    Spacetree::Visitor& visitor,
    const utils::DynVector& center,
    const utils::DynVector& halflengths );

  void clear();

private:

  static tarch::logging::Log _log;

  mesh::Group* _content;

  int _position;

  static const double _oneThird;

  utils::ptr_vector<PeanotreeCell3D> _childs;
};

}}} // namespace precice, spacetree, impl

#endif /* PRECICE_SPACETREE_PEANOTREECELL3D_HPP_ */
