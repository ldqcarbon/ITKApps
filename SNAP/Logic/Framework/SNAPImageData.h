/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    SNAPImageData.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __SNAPImageData_h_
#define __SNAPImageData_h_

#include <itkImage.h>
#include "IRISImageData.h"
#include "SnakeParameters.h"
#include "SpeedImageWrapper.h"
#include "LevelSetImageWrapper.h"

#include "EdgePreprocessingSettings.h"
#include "ThresholdSettings.h"

class SNAPLevelSetDriver;
template <class ImageType> class SNAPLevelSetFunction;

namespace itk {
  class Command;
  class EventObject;
}

/**
 * \class SNAPImageData
 * \brief Wrapper around the SNAP automatic segmentation pipelines.
 *
 * This class encapsulates several images used in the SNAP application, 
 * including the speed image, the bubble-initialization image and the
 * segmentation result image.
 */
class SNAPImageData : public IRISImageData 
{
public:

  // The type of the internal level set image
  typedef itk::Image<float,3> FloatImageType;
  typedef FloatImageType LevelSetImageType;

  // Command type for callbacks 
  typedef itk::SmartPointer<itk::Command> CommandPointer;

  SNAPImageData();
  ~SNAPImageData();

  /** 
   * Get the preprocessed (speed) image wrapper
   */
  SpeedImageWrapper* GetSpeed();

  /** A high level method to perform edge preprocessing on the grey image and
   * store the result in the speed image wrapper */
  void DoEdgePreprocessing(
    const EdgePreprocessingSettings &settings,itk::Command *progressCallback = 0);  

  /** A high level method to perform in-out preprocessing on the grey image and
   * store the result in the speed image wrapper */
  void DoInOutPreprocessing(
    const ThresholdSettings &settings,itk::Command *progressCallback = 0);

  /**
   * Initialize the Speed image wrapper to blank data
   */
  void InitializeSpeed();
  
  /**
   * Clear the preprocessed (speed) image (discard data, etc)
   */
  void ClearSpeed();

  /**
   * Check the preprocessed image for validity
   */
  bool IsSpeedLoaded();
  
  /** Get the current snake image wrapper */
  LevelSetImageWrapper* GetSnake();

  /**
   * Clear the current snake (discard data, etc)
   */
  void ClearSnake();

  /**
   * Check the current snake for validity
   */
  bool IsSnakeLoaded();

  /**
   * This optional method allows us to load an external advection
   * field. This field can be used when image data includes some
   * directional components, i.e., DTI 
   */
  void SetExternalAdvectionField( FloatImageType *imgX, 
    FloatImageType *imgY, FloatImageType *imgZ);

  /** This method reverts back to using gradient based advection fields */
  void RemoveExternalAdvectionField()
    { m_ExternalAdvectionField = NULL; }

  /** =========== Methods dealing with the segmentation pipeline ============ */

  /**
   * This method computes the two-sided distance transform from the array of
   * bubbles passed on and, if the segmentation image is not blank, the pixels
   * in that image.  This method also initializes the level set driver, ie, the
   * pipeline driving the segmentation process.  This method may take a while
   * to run because of the distance transforms.
   * 
   * @return If there were no voxels of labelColor in the union of the 
   * segmentation image with the bubbles, false will be returned and 
   * initialization will not be completed.  Otherwise, true will be returned.
   */
  bool InitializeSegmentationPipeline(
    const SnakeParameters &parameters,Bubble *bubbles, 
    unsigned int nBubbles, unsigned int labelColor);

  /** Begin level set pipeline execution.  This method is somewhat peculiar,
   * because it does not return until the pipeline has finished executing.  In
   * the mean time, it communicates with the caller by the means of two callback
   * commands, idleCallback and updateCallback, which it calls after some 
   * iterations are executed.  Remember, do not put any code that deals with
   * the segmentation pipeline after calling StartSegmentationPipeline! */
  void StartSegmentationPipeline(
    itk::Command *idleCallback,itk::Command *updateCallback);

  /** Request that the pipeline execute a number of iterations.  This method
   * must be called from the idle or update callbacks passed in to the 
   * StartSegmentationPipeline method.  After running the iterations, the
   * pipeline will resume calling idleCallback continuously.  This method
   * schedules a step and returns immideately */
  void RequestSegmentationStep(unsigned int nIterations);

  /** Request that the pipeline be restarted (brought back to the beginning).  
   * This method will schedule the restart and will return immideately */
  void RequestSegmentationRestart();

  /** Request that the pipeline be shut down.  This method will return 
   * immideately, but will schedule an exit from the pipeline mechanism.  If
   * everything goes well, control will return to the point where 
   * StartSegmentationPipeline was called */
  void RequestSegmentationPipelineTermination();

  /** Update the segmentation parameters, can be done either from the 
   * segmentation pipeline callback or on the fly.  This method is smart enough 
   * to reinitialize the level set driver if the Solver parameter changes */
  void SetSegmentationParameters(const SnakeParameters &parameters);

  /** Release the resources associated with the level set segmentation.  This 
   * method must be called once the segmentation pipeline has terminated, or 
   * else it would create a nasty crash */
  void ReleaseSegmentationPipeline();

  /** This method is an alternative to using the segmentation pipeline with 
   * callbacks.  It will simply run nIterations of the solver and return with
   * an updated SnakeWrapper.  Call this method after calling 
   * InitializeSegmentationPipeline() */
  void RunNonInteractiveSegmentation(int nIterations);

  /** Check if the segmentation pipeline has been initialized */
  bool IsSegmentationPipelineInitialized();

  /** Check if the segmentation pipeline is actively running, i.e., the 
   * stack frame contains the Update method of the segmentation filter */
  bool IsSegmentationPipelineRunning();

  /** ====================================================================== */
  
  /**
   * Merge the segmentation result with the segmentation contained in a
   * IRIS image data object.
   */
  void MergeSnakeWithIRIS(IRISImageData *target) const;

  /**
   * Set the cross-hairs position
   */
  void SetCrosshairs(const Vector3ui &crosshairs);

  /**
   * Set the image coordinate geometry
   */
  void SetImageGeometry(const ImageCoordinateGeometry &geometry);

  /**
   * Get the level set image currently being evolved
   */
  LevelSetImageType *GetLevelSetImage();

  /** This method is public for testing purposes.  It will give a pointer to 
   * the level set function used internally for segmentation */
  SNAPLevelSetFunction<SpeedImageWrapper::ImageType> *GetLevelSetFunction();
  
private:
  
  /** Initialize the driver used to control the snake.  This driver is used to
   * run the snake several iterations at a time, without resetting the filter
   * between iteration blocks.  After executing each block of iterations, the
   * filter will call a callback routine, which is provided as a parameter to
   * this method.  In a UI environment, that callback routine should check for
   * user input.  */
  void InitalizeSnakeDriver(const SnakeParameters &param);

  /** A callback used internally to communicate with the LevelSetDriver */
  void IntermediatePauseCallback(
    itk::Object *object,const itk::EventObject &event);

  /** Another callback, used in non-interactive mode */
  void TerminatingPauseCallback();

  /** Type of fommands used for callbacks to the user of this class */
  typedef itk::SmartPointer<itk::Command> CommandPointer;
  
  /** A callback for idle cycle of the segmentation pipeline */
  CommandPointer m_SegmentationIdleCallback;

  /** A callback made after an update in the segmentation pipeline */
  CommandPointer m_SegmentationUpdateCallback;

  /** Get the snake initialization image */
  LevelSetImageWrapper* GetSnakeInitialization();

  /** Clear the snake initialization image (discard data, etc) */
  void ClearSnakeInitialization();

  /** Check the snake initialization image for validity */
  bool IsSnakeInitializationLoaded();
  
  // Speed image adata
  SpeedImageWrapper *m_SpeedWrapper;

  // Wrapper around the level set image
  LevelSetImageWrapper *m_SnakeWrapper;
  
  // Snake initialization data (initial distance transform
  LevelSetImageWrapper *m_SnakeInitializationWrapper;

  // Snake driver
  SNAPLevelSetDriver *m_LevelSetDriver;

  // Label color used for the snake images
  LabelType m_SnakeColorLabel;

  // Current value of snake parameters
  SnakeParameters m_CurrentSnakeParameters;       

  // Typedefs for defining the advection image that can be loaded externally
  typedef itk::FixedArray<float, 3> VectorType;
  typedef itk::Image< VectorType, 3> VectorImageType;
  typedef itk::SmartPointer<VectorImageType> VectorImagePointer;

  // The advection image
  VectorImagePointer m_ExternalAdvectionField;
};




#endif
