/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    IRISSliceWindow.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __IRISSliceWindow_h_
#define __IRISSliceWindow_h_

#include "GenericSliceWindow.h"
#include "PolygonDrawing.h"

// Forward references to interaction modes that work with this window
class PolygonInteractionMode;
class RegionInteractionMode;

/**
 * \class IRISSliceWindow
 * \brief 2D slice window used in the IRIS part of the application.
 * 
 * These windows allow polygon editing and region of interest selection.
 */
class IRISSliceWindow : public GenericSliceWindow 
{
public:
  
  IRISSliceWindow(int x,int y,int w,int h,const char *l=0);
  virtual ~IRISSliceWindow();

  /** Enter the polygon editing mode of operation */
  void EnterPolygonMode();

  /** Enter the region of interest mode of operation */
  void EnterRegionMode();

  /** The parent's register method is overridden */
  void Register(int id,UserInterfaceLogic *parentUI);

  /**
   * The initialize method extends the parent's version, sets up some 
   * polygon drawing attributes
   */
  void InitializeSlice(IRISImageData *imageData);

  /**
   * CachedPolygon()
   *
   * purpose:
   * returns the m_CachedPolygon flag of the polygon drawing object
   *
   * post:
   * return value is 1 if polygon drawing has a cached polygon, 0 otherwise
   */
  int  CachedPolygon();

  /**
   * AcceptPolygon()
   *
   * purpose:
   * the gui calls this when the user presses the accept polygon button; this
   * means the polygon that was being edited will be rasterized into the voxel
   * data set according to the current drawing color and painting mode
   *
   * pre:
   * Register() and InitializeSlice() have been called
   * vox data has data with extents matching this window's width and height
   *
   * post:
   * if drawing lock held and polygon_drawing state was EDITING_STATE,
   *   voxels interior to polygon and "writable" are set with the current 
   *   color obtained from m_GlobalState->GetDrawingColor(); the three coverage
   *   modes determine which voxels inside the polygon are written over:
   *   PAINT_OVER_ALL:    all voxels
   *   PAINT_OVER_COLORS: all voxels that aren't labeled clear
   *   PAINT_OVER_ONE:    all voxels of the color obtained from
   *                      m_GlobalState->GetOverWriteColor()
   *   drawing lock released and polygon_drawing state is INACTIVE_STATE
   * else state not changed.
   */
  void AcceptPolygon();

  /**
   * PastePolygon()
   *
   * purpose:
   * brings the last drawn polygon back for reuse
   *
   * pre:
   * Register() and InitializeSlice() have been called
   *
   * post:
   * if polygon drawing state previously INACTIVE_STATE, drawing lock
   * obtainable, & polygon_drawing has a cached polygon,
   *   cached polygon becomes the edited polygon,
   *   polygon drawing state is EDITING_STATE
   *   drawing lock is held
   * else state does not change
   */
  void PastePolygon();
  
  // Allow friendly access by interactors
  friend class RegionInteractionMode;
  friend class ZoomPanInteractionMode;

protected:

  /** Interaction mode used to select the region of interest */
  RegionInteractionMode *m_RegionMode;

  /** Interaction mode used to position the crosshairs */
  PolygonInteractionMode *m_PolygonMode;

  /** polygon drawing object */
  PolygonDrawing m_PolygonDrawing;

  // Type definition for the slice used for polygon rendering
  typedef LabelImageWrapper::DisplaySliceType PolygonSliceType;
  typedef itk::SmartPointer<PolygonSliceType> PolygonSlicePointer;
  
  /** Slice used for polygon drawing and merging */
  PolygonSlicePointer m_PolygonSlice;

  /** Draw the region and polygon interactors */
  void DrawOverlays();
};

#endif


