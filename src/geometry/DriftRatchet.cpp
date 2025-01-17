// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "DriftRatchet.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Vertex.hpp"
#include "mesh/Edge.hpp"
#include "mesh/Triangle.hpp"
#include "mesh/PropertyContainer.hpp"
#include "tarch/la/Scalar.h"

namespace precice {
namespace geometry {

using namespace tarch::la;

const int DriftRatchet::MAX_RECURSION_DEPTH = 100;

const int DriftRatchet::INFLOW_GEO_ID = 1;

const int DriftRatchet::OUTFLOW_GEO_ID = 3;

const int DriftRatchet::WALL_GEO_ID = 2;


double DriftRatchet:: getCharacteristicLength
(
  double maxRadius )
{
  return maxRadius / 2.40 * 8.4;
}


double DriftRatchet::getDefaultMinRadius
(
  double maxRadius )
{
  return maxRadius / 2.40 * 1.25;
}


double DriftRatchet:: getDefaultShapeParameter ()
{
  return 0.61;
}


double DriftRatchet:: getDefaultMaxRadius ()
{
  return 2.40;
}


DriftRatchet:: DriftRatchet
(
  const utils::DynVector& offset,
  double                  discretizationWidth,
  double                  maxRadius,
  double                  minRadius,
  double                  shapeParameter,
  double                  length,
  double                  pores,
  int                     wallIndex,
  int                     inflowIndex,
  int                     outflowIndex )
:
   Geometry ( offset ),
   _discretizationWidth ( discretizationWidth ),
   _maxRadius ( maxRadius ),
   _minRadius ( minRadius ),
   _shapeParameter ( shapeParameter ),
   _length ( length ),
   _pores ( pores )
{}


DriftRatchet:: ~DriftRatchet () {}

int DriftRatchet:: getNumberOfVerticesPerCut ( double h ) const
{
  if ( getOffset().size() == 2 ){
    return 2;
  }
  else {
    double maxCircumference = 2.0 * tarch::la::PI * _maxRadius;
    return int(std::ceil( maxCircumference / h ));
  }
}


int DriftRatchet:: getCutsAlongXAxis
(
  double discretisationWidth ) const
{
  return int (std::ceil( _length / discretisationWidth ));
}

void DriftRatchet:: specializedCreate
(
  mesh::Mesh& seed )
{
   using namespace mesh;
   std::string nameSubID ( seed.getName() + "-side-" );
   PropertyContainer* leftWall = NULL;
   PropertyContainer* rightWall = NULL;
   if ( seed.getNameIDPairs().count(nameSubID + "0") ) {
      leftWall = & seed.getPropertyContainer ( nameSubID + "0" );
   }
   if ( seed.getNameIDPairs().count(nameSubID + "1") ) {
      rightWall = & seed.getPropertyContainer ( nameSubID + "1" );
   }
   int count = getNumberOfVerticesPerCut ( _discretizationWidth );
   mesh::Vertex** cutVertices = new mesh::Vertex* [count];
   count = getOffset().size() == 3 ? count : 0; // Only used in 3D
   mesh::Edge ** cutEdges = new mesh::Edge * [count];
   createLeftWall ( seed, leftWall, cutVertices, cutEdges );
   createBodyWall ( seed, NULL, cutVertices, cutEdges );
   createRightWall ( seed, rightWall, cutVertices, cutEdges );
   delete[] cutVertices;
   delete[] cutEdges;
}

void DriftRatchet:: createLeftWall
(
  mesh::Mesh&              mesh,
  mesh::PropertyContainer* propertyContainer,
  mesh::Vertex*            cutVertices[],
  mesh::Edge*              cutEdges[]  )
{
  int dimensions = mesh.getDimensions();
  if ( dimensions == 2 ){
    using utils::Vector2D;
    double radius = getRadius ( 0.0 );
    Vector2D center ( 0.0 );
    mesh::Vertex& centerVertex = mesh.createVertex ( center );
    if ( propertyContainer != NULL ) {
       centerVertex.addParent ( *propertyContainer );
    }
    Vector2D upperPoint ( center(0), center(1) + radius );
    cutVertices[0] = & mesh.createVertex ( upperPoint );
    Vector2D lowerPoint ( center(0), center(1) - radius);
    cutVertices[1] = & mesh.createVertex ( lowerPoint );
    mesh::Edge& e0 = mesh.createEdge ( *cutVertices[0], centerVertex);
    mesh::Edge& e1 = mesh.createEdge ( centerVertex, *cutVertices[1] );
    if ( propertyContainer != NULL ) {
      e0.addParent ( *propertyContainer );
      e1.addParent ( *propertyContainer );
    }
  }
  else {
    assertion1 ( dimensions == 3, dimensions );
    using utils::Vector3D;
    double currentAngle = 0.0;
    int vertexCount = getNumberOfVerticesPerCut ( _discretizationWidth );
    double angle = 2.0 * tarch::la::PI / static_cast<double>(vertexCount);
    double radius = getRadius ( 0.0 );
    Vector3D center(0.0);
    mesh::Vertex& centerVertex = mesh.createVertex ( center );
    if ( propertyContainer != NULL ) {
       centerVertex.addParent ( *propertyContainer );
    }

    // Create vertices
    for ( int i=0; i < vertexCount; i++ ) {
      Vector3D currentPoint ( center );
      double y = std::cos(currentAngle) * radius;
      currentPoint(1) += y;
      double z = std::sin(currentAngle) * radius;
      currentPoint(2) += z;
      currentAngle += angle;
      cutVertices[i] = & mesh.createVertex ( currentPoint );
      if ( propertyContainer != NULL ) {
        cutVertices[i]->addParent ( *propertyContainer );
      }
    }

    // Create edges and triangles
    mesh::Edge & firstCenterEdge = mesh.createEdge ( centerVertex,
                                                     *cutVertices[0] );
    mesh::Edge * oldCenterEdge = & firstCenterEdge;
    mesh::Edge * newCenterEdge = NULL;
    for ( int i=0; i < vertexCount - 1; i++ ) {
      newCenterEdge = & mesh.createEdge ( centerVertex, *cutVertices[i+1] );
      cutEdges[i] = & mesh.createEdge ( *cutVertices[i], *cutVertices[i+1] );
      mesh::Triangle & t = mesh.createTriangle ( *oldCenterEdge, *cutEdges[i], *newCenterEdge );
      if ( propertyContainer != NULL ) {
        cutEdges[i]->addParent ( *propertyContainer );
        oldCenterEdge->addParent ( *propertyContainer );
        t.addParent ( *propertyContainer );
      }
      oldCenterEdge = newCenterEdge;
    }
    cutEdges[vertexCount-1] = & mesh.createEdge ( *cutVertices[vertexCount-1],
                                                  *cutVertices[0] );
    mesh::Triangle & t = mesh.createTriangle (
      *oldCenterEdge, *cutEdges[vertexCount-1], firstCenterEdge );
    if ( propertyContainer != NULL ) {
      cutEdges[vertexCount-1]->addParent ( *propertyContainer );
      t.addParent ( *propertyContainer );
    }
  }
}

void DriftRatchet:: createBodyWall
(
   mesh::Mesh&              mesh,
   mesh::PropertyContainer* propertyContainer,
   mesh::Vertex*            cutVertices[],
   mesh::Edge*              cutEdges[] )
{
   int sectionCount = getCutsAlongXAxis ( _discretizationWidth );
   double currentXInImage = 0.0;
   double currentXInPreImage = 0.0;
   double xStepInImage = _length / (double)sectionCount;
   double xStepInPreImage = (_pores * getCharacteristicLength(_maxRadius)) / (double)sectionCount;
   int dimensions = mesh.getDimensions();
   utils::DynVector currentCenter ( dimensions, 0.0 );
   currentXInImage    += xStepInImage;
   currentXInPreImage += xStepInPreImage;
   for ( int i=0; i < sectionCount; i++ ) {
      currentCenter(0) = currentXInImage;
      double radius = getRadius ( currentXInPreImage );
      createBodyWallSection ( mesh, propertyContainer, cutVertices, cutEdges,
                              currentCenter, radius );
      currentXInImage    += xStepInImage;
      currentXInPreImage += xStepInPreImage;
   }
}

void DriftRatchet:: createBodyWallSection
(
   mesh::Mesh&              mesh,
   mesh::PropertyContainer* propertyContainer,
   mesh::Vertex*            cutVertices[],
   mesh::Edge*              cutEdges[],
   const utils::DynVector&  center,
   double                   radius )
{
  int dimensions = mesh.getDimensions();
  if ( dimensions == 2 ){
    utils::Vector2D upperPoint ( center[0], center[1] + radius );
    utils::Vector2D lowerPoint ( center[0], center[1] - radius );
    mesh::Vertex& upperVertex = mesh.createVertex ( upperPoint );
    mesh::Vertex& lowerVertex = mesh.createVertex ( lowerPoint );
    mesh::Edge& e0 = mesh.createEdge ( upperVertex, *cutVertices[0] );
    mesh::Edge& e1 = mesh.createEdge ( *cutVertices[1], lowerVertex );
    cutVertices[0] = & upperVertex;
    cutVertices[1] = & lowerVertex;
    if ( propertyContainer != NULL ) {
      e0.addParent ( *propertyContainer );
      e1.addParent ( *propertyContainer );
      cutVertices[0]->addParent ( *propertyContainer );
      cutVertices[1]->addParent ( *propertyContainer );
    }
  }
  else {
    assertion1 ( dimensions == 3, dimensions );
    double currentAngle = 0.0;
    int vertexCount = getNumberOfVerticesPerCut ( _discretizationWidth );
    double angle = 2.0 * tarch::la::PI / static_cast<double>(vertexCount);

    // Create vertices
    mesh::Vertex ** newCutVertices = new mesh::Vertex * [vertexCount];
    for ( int i=0; i < vertexCount; i++ ) {
      utils::Vector3D currentCenter ( center );
      double y = std::cos(currentAngle) * radius;
      currentCenter(1) += y;
      double z = std::sin(currentAngle) * radius;
      currentCenter(2) += z;
      currentAngle += angle;
      newCutVertices[i] = & mesh.createVertex ( currentCenter );
      if ( propertyContainer != NULL ) {
        newCutVertices[i]->addParent ( *propertyContainer );
      }
    }

    // Create edges and triangles
    mesh::Edge** newCutEdges = new mesh::Edge * [vertexCount];
    mesh::Edge* initialEdge = & mesh.createEdge ( *cutVertices[0],
                                                  *newCutVertices[0] );
    if ( propertyContainer != NULL ) {
      initialEdge->addParent ( *propertyContainer );
    }
    mesh::Edge* firstEdge = initialEdge;
    mesh::Edge* crossingEdge = NULL;
    mesh::Edge* secondEdge = NULL;
    for ( int i=0; i < vertexCount - 1; i++ ) {
      crossingEdge = & mesh.createEdge ( *cutVertices[i],
                                         *newCutVertices[i+1] );
      newCutEdges[i] = & mesh.createEdge ( *newCutVertices[i],
                                           *newCutVertices[i+1] );
      secondEdge = & mesh.createEdge ( *cutVertices[i+1],
                                       *newCutVertices[i+1] );
      mesh::Triangle& t0 = mesh.createTriangle ( *firstEdge, *newCutEdges[i], *crossingEdge );
      mesh::Triangle& t1 = mesh.createTriangle ( *crossingEdge, *secondEdge, *cutEdges[i] );
      if ( propertyContainer != NULL ) {
        t0.addParent ( *propertyContainer );
        t1.addParent ( *propertyContainer );
        crossingEdge->addParent ( *propertyContainer );
        secondEdge->addParent ( *propertyContainer );
      }
      firstEdge = secondEdge;
    }
    crossingEdge = & mesh.createEdge ( *cutVertices[vertexCount - 1],
                                       *newCutVertices[0] );
    newCutEdges[vertexCount - 1] =
      & mesh.createEdge ( *newCutVertices[vertexCount - 1], *newCutVertices[0] );
    secondEdge = initialEdge;
    mesh::Triangle & t0 =
      mesh.createTriangle ( *firstEdge, *newCutEdges[vertexCount -1], *crossingEdge );
    mesh::Triangle & t1 =
      mesh.createTriangle ( *crossingEdge, *secondEdge, *cutEdges[vertexCount - 1] );
    if ( propertyContainer != NULL ) {
      crossingEdge->addParent ( *propertyContainer );
      newCutEdges[vertexCount - 1]->addParent ( *propertyContainer );
      t0.addParent ( *propertyContainer );
      t1.addParent ( *propertyContainer );
    }

    // Transfer cutting vertices and edges
    for ( int i=0; i < vertexCount; i++ ) {
      cutVertices[i] = newCutVertices[i];
      cutEdges[i] = newCutEdges[i];
    }
    delete[] newCutVertices;
    delete[] newCutEdges;
  }
}

void DriftRatchet:: createRightWall
(
  mesh::Mesh&              mesh,
  mesh::PropertyContainer* propertyContainer,
  mesh::Vertex*            cutVertices[],
  mesh::Edge*              cutEdges[]  )
{
  int dimensions = mesh.getDimensions();
  if ( dimensions == 2 ){
    utils::Vector2D center ( 0.0 );
    center(0) += _length;
    mesh::Vertex& centerVertex = mesh.createVertex ( center );
    if ( propertyContainer != NULL ) {
      centerVertex.addParent ( *propertyContainer );
    }
    mesh::Edge& e0 = mesh.createEdge ( centerVertex, *cutVertices[0] );
    mesh::Edge& e1 = mesh.createEdge ( *cutVertices[1], centerVertex );
    if ( propertyContainer != NULL ) {
      e0.addParent ( *propertyContainer );
      e1.addParent ( *propertyContainer );
    }
  }
  else {
    assertion1 ( dimensions == 3, dimensions );
    utils::Vector3D center ( 0.0 );
    center(0) += _length;
    mesh::Vertex& centerVertex = mesh.createVertex ( center );
    if ( propertyContainer != NULL ) {
      centerVertex.addParent ( *propertyContainer );
    }
    int vertexCount = getNumberOfVerticesPerCut ( _discretizationWidth );

    // Create edges and triangles
    mesh::Edge& firstCenterEdge = mesh.createEdge ( centerVertex, *cutVertices[0] );
    if ( propertyContainer != NULL ) {
      firstCenterEdge.addParent ( *propertyContainer );
    }
    mesh::Edge* oldCenterEdge = & firstCenterEdge;
    mesh::Edge* newCenterEdge = NULL;
    for ( int i=0; i < vertexCount - 1; i++ ) {
      newCenterEdge = & mesh.createEdge ( centerVertex, *cutVertices[i+1] );
      mesh::Triangle& t = mesh.createTriangle (
          *cutEdges[i], *oldCenterEdge, *newCenterEdge );
      if ( propertyContainer != NULL ) {
        newCenterEdge->addParent ( *propertyContainer );
        t.addParent ( *propertyContainer );
      }
      oldCenterEdge = newCenterEdge;
    }
    mesh::Triangle& t = mesh.createTriangle ( *cutEdges[vertexCount-1],
                                              *oldCenterEdge, firstCenterEdge );
    if ( propertyContainer != NULL ) {
      t.addParent ( *propertyContainer );
    }
  }
}

double DriftRatchet:: getG
(
  double normalisedX,
  int remainingIterations ) const
{
  double result = 0.0;
  if (remainingIterations==0) {
    result = std::sin( 2.0 * PI * normalisedX );
  }
  else {
    double recursion = getG(normalisedX, remainingIterations-1 );
    assertion( recursion <=  1.0 );
    assertion( recursion >= -1.0 );
    result = std::sin( 2.0 * PI * normalisedX - _shapeParameter * recursion );
  }
  return result;
}

double DriftRatchet:: getRadius
(
  double normalisedX ) const
{
   assertion( _maxRadius > _minRadius );
   normalisedX = normalisedX - getExtremeCoordinateInAxisDirection(0);
   double recursionParameter = normalisedX / getCharacteristicLength(_maxRadius);
   while ( greater(recursionParameter,1.0) ) recursionParameter -= 1.0;
   while ( smaller(recursionParameter,0.0) ) recursionParameter += 1.0;
   double g = getG(recursionParameter,MAX_RECURSION_DEPTH);
   assertion ( not smaller(g,-1.0) );
   assertion ( not greater(g, 1.0) );
   double result = _minRadius + 0.5 * (_maxRadius-_minRadius) * (1.0+g);
   assertion( result >= _minRadius );
   assertion( result <= _maxRadius );
   return result;
}

double DriftRatchet:: getExtremeCoordinateInAxisDirection
(
  int n ) const
{
   assertion ( n >= 0 );
   double signum = (n & 1)!=0 ? -1.0 : 1.0;
   return ( signum*_shapeParameter/2.0/PI + 0.25 + n/2.0 ) *
          getCharacteristicLength(_maxRadius);
}

}} // namespace precice, geometry
