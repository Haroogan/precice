// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#ifndef PRECICE_CPLSCHEME_COUPLINGDATA_HPP_
#define PRECICE_CPLSCHEME_COUPLINGDATA_HPP_

#include "mesh/SharedPointer.hpp"
#include "tarch/la/DynamicColumnMatrix.h"
#include "utils/Helpers.hpp"
#include "mesh/Data.hpp"
#include <vector>

namespace precice {
namespace cplscheme {

struct CouplingData
{
  typedef tarch::la::DynamicColumnMatrix<double> DataMatrix;

  /// @brief Data values of current iteration.
  utils::DynVector* values;

  /// @brief Data values of previous iteration (1st col) and previous timesteps.
  DataMatrix oldValues;

  mesh::PtrMesh mesh;

  /// @brief True, if the data values are initialized by a participant.
  bool initialize;

  /// @ dimension of one data value (scalar=1, or vectorial=interface-dimension)
  int dimension;

  /**
   * @brief Default constructor, not to be used!
   *
   * Necessary when compiler creates template code for std::map::operator[].
   */
  CouplingData ()
    {
      assertion ( false );
    }

  /**
   * @brief Constructor.
   */
  CouplingData (
    utils::DynVector* values,
    mesh::PtrMesh       mesh,
    bool              initialize,
    int               dimension)
    :
    values ( values ),
    oldValues (),
    mesh(mesh),
    initialize ( initialize ),
    dimension(dimension)
    {
      assertion ( values != NULL );
      assertion ( mesh.use_count()>0);
    }
};

}} // namespace precice, cplscheme

#endif /* PRECICE_CPLSCHEME_COUPLINGDATA_HPP_ */
