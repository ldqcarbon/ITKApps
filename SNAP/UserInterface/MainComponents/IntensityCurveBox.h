/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    IntensityCurveBox.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __IntensityCurveBox_h_
#define __IntensityCurveBox_h_

#include <FL/Fl_Gl_Window.H>

#include <IRISTypes.h>
#include <IntensityCurveInterface.h>
#include <FLTKCanvas.h>
#include <InteractionMode.h>

#include <vector>

class IntensityCurveUILogic;
class GreyImageWrapper;

/**
 * \class IntensityCurveBox
 * \brief An FLTK Box (Gl_Window) used to paint intensity mapping curves.
 */
class IntensityCurveBox : public FLTKCanvas {
public:
  IntensityCurveBox(int x,int y,int w,int h,const char *label);  

  /**
   * Handle displaying the curve
   */
  void draw();

  /** Compute the histogram given an image wrapper */
  void ComputeHistogram(GreyImageWrapper *source);

  // Get/set the intensity curve
  irisGetMacro(Curve,IntensityCurveInterface *);
  irisSetMacro(Curve,IntensityCurveInterface *);

  // Get/set the parent object
  irisGetMacro(Parent,IntensityCurveUILogic *);
  irisSetMacro(Parent,IntensityCurveUILogic *);

  /**
   * The resolution of the curve displayed on the screen
   * TODO: Control over curve resolution
   */
  static unsigned int CURVE_RESOLUTION;

private:

  /**
   * Check if a control point is close to another point (i.e. mouse position)
   */
  int GetControlPointInVincinity(float x, float y, int pixelRadius); 

  /** The intensity mapping curve */
  IntensityCurveInterface *m_Curve;

  /** Parent object */
  IntensityCurveUILogic *m_Parent;

  /** Histogram of the image */
  std::vector<unsigned int> m_Histogram;   

  /** Max frequency in the histogram */
  unsigned int m_HistogramMax;

  /**
   * Interaction handler for control point manipulation
   */
  class DefaultHandler : public InteractionMode {
  public:
    DefaultHandler(IntensityCurveBox *parent);

    int OnMousePress(const FLTKEvent &event);
    int OnMouseRelease(const FLTKEvent &event, const FLTKEvent &pressEvent);
    int OnMouseDrag(const FLTKEvent &event, const FLTKEvent &pressEvent);
    int OnMouseEnter(const FLTKEvent &event);
    int OnMouseLeave(const FLTKEvent &event);
    int OnMouseMotion(const FLTKEvent &event);

  private:
    // Pointer to the parent canvas
    IntensityCurveBox *m_Parent;

    // Set cursor depending on selected point
    void SetCursor(int iControlPoint);

    // Update a control point to reflect mouse motion to p
    bool UpdateControl(const Vector3f &p);

    // Control point being currently edited
    int m_MovingControlPoint;

  } m_DefaultHandler;

  // Allow access to private data
  friend class DefaultHandler;
};

#endif // __IntensityCurveBox_h_

