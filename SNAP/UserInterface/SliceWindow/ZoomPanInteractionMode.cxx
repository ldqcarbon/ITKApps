/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    ZoomPanInteractionMode.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "ZoomPanInteractionMode.h"
#include "UserInterfaceLogic.h"
#include "SlicewindowCoordinator.h"

#include <cmath>
#include <iostream>

ZoomPanInteractionMode
::ZoomPanInteractionMode(GenericSliceWindow *parent)
: GenericSliceWindow::EventHandler(parent)
{
  m_NeedUIUpdateOnRepaint = false;
}

int
ZoomPanInteractionMode
::OnMousePress(const FLTKEvent &irisNotUsed(event))
{
  // Record the current zoom and view position
  m_StartViewZoom = m_Parent->GetViewZoom();
  m_StartViewPosition = m_Parent->m_ViewPosition;

  // Done
  return 1;
}

int 
ZoomPanInteractionMode
::OnMouseDrag(const FLTKEvent &event,const FLTKEvent &dragEvent)
{
  if(dragEvent.Button == FL_LEFT_MOUSE)
  {
    // Compute the start and end point in slice coordinates
    Vector3f xStart = m_Parent->MapWindowToSlice(dragEvent.XSpace.extract(2));
    Vector3f xEnd = m_Parent->MapWindowToSlice(event.XSpace.extract(2));
    Vector2f xOffset(xEnd[0] - xStart[0],xEnd[1] - xStart[1]);

    // Remove the scaling by spacing
    xOffset(0) *= m_Parent->m_SliceSpacing(0);
    xOffset(1) *= m_Parent->m_SliceSpacing(1);
    
    // Under the left button, the tool changes the view_pos by the
    // distance traversed
    m_Parent->m_ViewPosition = m_StartViewPosition - xOffset;
  }
  else 
  {
    // Under the right button, the tool causes us to zoom based on the vertical
    // motion
    float zoom = 
      m_StartViewZoom * 
      pow(1.02f,(float)(event.XSpace(1) - dragEvent.XSpace(1)));

    // Clamp the zoom factor to reasonable limits
    zoom = m_ParentUI->GetSliceCoordinator()->ClampZoom(m_Parent->m_Id,zoom);

    // Make sure the zoom factor is an integer fraction
    zoom = m_Parent->GetOptimalZoom() * 
      ((int)(zoom / m_Parent->GetOptimalZoom() * 100)) / 100.0f;
    
    // Set the zoom factor using the window coordinator
    m_Parent->SetViewZoom(zoom);
    m_ParentUI->GetSliceCoordinator()->OnZoomUpdateInWindow(
      m_Parent->m_Id,zoom);
      
    // Schedule an update of the zoom percentage display in the parent
    m_NeedUIUpdateOnRepaint = true;
  }

  // Redraw the screen
  m_Parent->redraw();

  // Nothing to do here.
  return 1;
}

int 
ZoomPanInteractionMode
::OnMouseRelease(const FLTKEvent &irisNotUsed(event),
                 const FLTKEvent &irisNotUsed(dragEvent))
{
  // Nothing to do here!
  return 1;
}

void
ZoomPanInteractionMode
::OnDraw()
{
  if(m_NeedUIUpdateOnRepaint)
    {
    m_ParentUI->OnZoomUpdate();
    m_NeedUIUpdateOnRepaint = false;
    }   
}

