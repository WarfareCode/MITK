/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-04-23 13:50:34 +0200 (Do, 23 Apr 2009) $
Version:   $Revision: 16947 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigureMapper2D.h"
#include "mitkPlanarFigure.h"

#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkGL.h"


mitk::PlanarFigureMapper2D::PlanarFigureMapper2D()
{
}


mitk::PlanarFigureMapper2D::~PlanarFigureMapper2D()
{
}




void mitk::PlanarFigureMapper2D::Paint( mitk::BaseRenderer *renderer )
{
  if ( !this->IsVisible( renderer ) ) 
  {
    return;
  }

  // Get PlanarFigure from input
  mitk::PlanarFigure *planarFigure = const_cast< mitk::PlanarFigure * >(
    static_cast< const mitk::PlanarFigure * >( this->GetData() ) );

  // Get 2D geometry frame of PlanarFigure
  mitk::Geometry2D *planarFigureGeometry2D = 
    dynamic_cast< Geometry2D * >( planarFigure->GetGeometry( 0 ) );
  if ( planarFigureGeometry2D == NULL )
  {
    LOG_ERROR << "PlanarFigure does not have valid Geometry2D!";
    return;
  }

  // Get current world 2D geometry from renderer
  const mitk::Geometry2D *rendererGeometry2D = renderer->GetCurrentWorldGeometry2D();

  // If the PlanarFigure geometry is a plane geometry, check if 2D world
  // geometry and planar figure geometry describe the same plane (otherwise,
  // nothing is displayed)
  mitk::PlaneGeometry *planarFigurePlaneGeometry = 
    dynamic_cast< PlaneGeometry * >( planarFigureGeometry2D );
  const mitk::PlaneGeometry *rendererPlaneGeometry = 
    dynamic_cast< const PlaneGeometry * >( rendererGeometry2D );
  
  bool planeGeometry = false;
  if ( (planarFigurePlaneGeometry != NULL) && (rendererPlaneGeometry != NULL) )
  {
    planeGeometry = true;
    if ( !planarFigurePlaneGeometry->IsOnPlane( rendererPlaneGeometry ) )
    {
      return;
    }
  }


  // Get display geometry
  mitk::DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();
  assert( displayGeometry != NULL );


  // Apply visual appearance properties from the PropertyList
  this->ApplyProperties( renderer );


  //if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("Width")) != NULL)
  //  lineWidth = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("Width"))->GetValue();
  //glLineWidth(lineWidth);

  if ( planarFigure->IsClosed() )
  {
    glBegin( GL_LINE_LOOP );
  }
  else 
  {
    glBegin( GL_LINE_STRIP );
  }

  typedef mitk::PlanarFigure::VertexContainerType VertexContainerType;
  const VertexContainerType *polyLine = planarFigure->GetPolyLine();

  VertexContainerType::ConstIterator it;
  for ( it = polyLine->Begin(); it != polyLine->End(); ++it )
  {
    // Draw this 2D point as OpenGL vertex
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay( it->Value(), displayPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

    glVertex2f( displayPoint[0], displayPoint[1] );

  }

  glEnd();

  const VertexContainerType *controlPoints = planarFigure->GetControlPoints();
  for ( it = controlPoints->Begin(); it != controlPoints->End(); ++it )
  {
    // Draw markers at control points
    this->DrawMarker( it->Value(),
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );
  }

  glLineWidth( 1.0 );
}

void mitk::PlanarFigureMapper2D::TransformObjectToDisplay(
  const mitk::Point2D &point,
  mitk::Point2D &displayPoint,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point2D point2D;
  mitk::Point3D point3D;

  // Map circle point from local 2D geometry into 3D world space
  objectGeometry->IndexToWorld( point, point2D );
  objectGeometry->Map( point2D, point3D );

  // Project 3D world point onto display geometry
  rendererGeometry->Map( point3D, displayPoint );
  displayGeometry->WorldToDisplay( displayPoint, displayPoint );
}

void mitk::PlanarFigureMapper2D::DrawMarker(
  const mitk::Point2D &point,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point2D displayPoint;

  this->TransformObjectToDisplay(
    point, displayPoint,
    objectGeometry, rendererGeometry, displayGeometry );

  glBegin( GL_LINE_LOOP );

  glVertex2f( displayPoint[0] - 4, displayPoint[1] - 4 );
  glVertex2f( displayPoint[0] - 4, displayPoint[1] + 4 );
  glVertex2f( displayPoint[0] + 4, displayPoint[1] + 4 );
  glVertex2f( displayPoint[0] + 4, displayPoint[1] - 4 );

  glEnd();

}
