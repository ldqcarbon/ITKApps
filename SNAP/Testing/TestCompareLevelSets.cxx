/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    TestCompareLevelSets.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "TestCompareLevelSets.h"
#include "SNAPRegistryIO.h"

#include "IRISApplication.h"
#include "IRISImageData.h"
#include "SNAPImageData.h"
#include "ImageWrapper.h"
#include "GreyImageWrapper.h"
#include "SNAPLevelSetFunction.h"

#include "itkDenseFiniteDifferenceImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkSparseFieldLevelSetImageFilter.h"
#include "itkParallelSparseFieldLevelSetImageFilter.h"
#include "itkNarrowBandLevelSetImageFilter.h"
#include "itkCommand.h"
#include "itkUnaryFunctorImageFilter.h"


#include <ctime>
#include <iomanip>

using namespace std;

void 
TestCompareLevelSets
::PrintUsage() 
{
  // Run the parent's part of the test
  Superclass::PrintUsage();

  // RAI may be passed to this test
  cout << "  config FILE : Set configuration registry file (required)" << endl;
  cout << "  generate : Instead of running the test, generate a sample config" << endl;
}

template <class TFilter>
class LevelSetExtensionFilter : public TFilter
{
public:
  
  /** Standard class typedefs. */
  typedef LevelSetExtensionFilter<TFilter> Self;
  typedef TFilter Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Run-time type information. */
  itkTypeMacro(LevelSetExtensionFilter,TFilter);

  /** Capture information from the superclass. */
  typedef typename Superclass::InputImageType   InputImageType;
  typedef typename Superclass::OutputImageType  OutputImageType;
  // typedef typename Superclass::UpdateBufferType UpdateBufferType;

  /** Dimensionality of input and output data is assumed to be the same.
   * It is inherited from the superclass. */
  itkStaticConstMacro(ImageDimension, unsigned int,Superclass::ImageDimension);

  itkNewMacro(LevelSetExtensionFilter);

  /** The pixel type of the output image will be used in computations.
   * Inherited from the superclass. */
  typedef typename Superclass::PixelType PixelType;
  typedef typename Superclass::TimeStepType TimeStepType;

  /** Set/Get the number of iterations that the filter will run. */
  itkSetMacro(NumberOfIterations, unsigned int);
  itkGetMacro(NumberOfIterations, unsigned int);

protected:
  LevelSetExtensionFilter() 
  {
    m_NumberOfIterations = 10;
  }
  
  ~LevelSetExtensionFilter() {}
  
  void PrintSelf(std::ostream& os, itk::Indent indent) const
  {
    Superclass::PrintSelf(os,indent);
  }

  /** Supplies the halting criteria for this class of filters.  The
   * algorithm will stop after a user-specified number of iterations. */
  virtual bool Halt() 
  {
    if (this->GetElapsedIterations() >= m_NumberOfIterations) return true;
    else return false;
  }

private:
  LevelSetExtensionFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
  unsigned int     m_NumberOfIterations;
};

#ifdef _USE_FastLevelSetFunction_
class FastSNAPLevelSetFunction : 
  public SNAPLevelSetFunction<itk::Image<float, 3> >
{
public:
  
  /** Standard class typedefs. */
  typedef FastSNAPLevelSetFunction Self;
  typedef itk::Image<float,3> ImageType;
  typedef itk::SegmentationLevelSetFunction<ImageType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
                                                                                
  /** Method for creation through the object factory. */
  itkNewMacro(FastSNAPLevelSetFunction);
                                                                                
  /** Run-time type information (and related methods) */
  itkTypeMacro( SNAPLevelSetFunction, itk::LevelSetFunction );
                                                                                
  /** Extract some parameters from the superclass. */
  typedef Superclass::ImageType ImageType;
  typedef ImageType::Pointer ImagePointer;
  typedef Superclass::NeighborhoodType NeighborhoodType;
  typedef Superclass::ScalarValueType ScalarValueType;
  typedef Superclass::RadiusType RadiusType;
  typedef Superclass::FloatOffsetType FloatOffsetType;
  
  typedef Superclass::VectorType VectorType;
  typedef itk::Image<VectorType,ImageDimension> VectorImageType;
  typedef VectorImageType::Pointer VectorImagePointer;

  typedef Superclass::TimeStepType TimeStepType;
  typedef Superclass::GlobalDataStruct GlobalDataStruct;

  /** Our own initialize method: computes all offsets */
  virtual void Initialize(const RadiusType &r);

  /** Our own compute update method: fast and furiuos */
  virtual float ComputeUpdate(const NeighborhoodType &it, void *globalData,
                              const FloatOffsetType& offset);
  
  /** Image offsets for fast neighborhood operations */
  int m_OffsetFX;
  int m_OffsetFY;
  int m_OffsetFZ;
  int m_OffsetBX;
  int m_OffsetBY;
  int m_OffsetBZ;
  int m_OffsetFXFY;
  int m_OffsetFYFZ;
  int m_OffsetFZFX;
  int m_OffsetBXFY;
  int m_OffsetBYFZ;
  int m_OffsetBZFX;
  int m_OffsetFXBY;
  int m_OffsetFYBZ;
  int m_OffsetFZBX;
  int m_OffsetBXBY;
  int m_OffsetBYBZ;
  int m_OffsetBZBX;
};

void 
FastSNAPLevelSetFunction
::Initialize(const RadiusType &r)
{
  // Let the parent do his work
  Superclass::Initialize(r);

  // Compute all the offsets
  m_OffsetFX = m_xStride[0];
  m_OffsetFY = m_xStride[1];
  m_OffsetFZ = m_xStride[2];

  m_OffsetBX = -m_OffsetFX;
  m_OffsetBY = -m_OffsetFY;
  m_OffsetBZ = -m_OffsetFZ;

  m_OffsetFXFY = m_OffsetFX + m_OffsetFY;
  m_OffsetFYFZ = m_OffsetFY + m_OffsetFZ;
  m_OffsetFZFX = m_OffsetFZ + m_OffsetFX;

  m_OffsetBXFY = m_OffsetBX + m_OffsetFY;
  m_OffsetBYFZ = m_OffsetBY + m_OffsetFZ;
  m_OffsetBZFX = m_OffsetBZ + m_OffsetFX;

  m_OffsetFXBY = m_OffsetFX + m_OffsetBY;
  m_OffsetFYBZ = m_OffsetFY + m_OffsetBZ;
  m_OffsetFZBX = m_OffsetFZ + m_OffsetBX;

  m_OffsetBXBY = m_OffsetBX + m_OffsetBY;
  m_OffsetBYBZ = m_OffsetBY + m_OffsetBZ;
  m_OffsetBZBX = m_OffsetBZ + m_OffsetBX;
}

float 
FastSNAPLevelSetFunction
::ComputeUpdate(const NeighborhoodType &it, 
                void *globalData,const FloatOffsetType& offset)
{
  // Get the central pixel location
  const float *pixel = it.GetCenterPointer();

  // Global data structure
  GlobalDataStruct *gd = (GlobalDataStruct *)globalData;

  // Get the neighboring pixel values
  float v  = *(pixel);  
  float fx = *(pixel + m_OffsetFX);
  float bx = *(pixel + m_OffsetBX);
  float fy = *(pixel + m_OffsetFY);
  float by = *(pixel + m_OffsetBY);
  float fz = *(pixel + m_OffsetFZ);
  float bz = *(pixel + m_OffsetBZ);  

  // Compute the second derivatives
  float vv = v + v;
  float uxx = fx + bx - vv;
  float uyy = fy + by - vv;
  float uzz = fz + bz - vv;

  // Forward and backward differences
  float uxf = fx - v;
  float uyf = fy - v;
  float uzf = fz - v;
  float uxb = v - bx;
  float uyb = v - by;
  float uzb = v - bz;

  // Compute the central first derivatives
  float uxc = 0.5f * (fx - bx);
  float uyc = 0.5f * (fy - by);
  float uzc = 0.5f * (fz - bz);

  // Compute the squared central first derivatives 
  float uxc2 = uxc*uxc;
  float uyc2 = uyc*uyc;
  float uzc2 = uzc*uzc;

  // Compute the Hessian matrix and various other derivatives.  Some of these
  // derivatives may be used by overloaded virtual functions.
  gd->m_GradMagSqr = 1.0e-6 + uxc2 + uyc2 + uzc2;

  // Compute the curvature term
  float curvature_term = 0.0f;
  if(m_CurvatureWeight != 0.0f)
    {
    // More terms to compute
    float fxfy = *(pixel + m_OffsetFXFY);
    float fyfz = *(pixel + m_OffsetFYFZ);
    float fzfx = *(pixel + m_OffsetFZFX);
    
    float bxfy = *(pixel + m_OffsetBXFY);
    float byfz = *(pixel + m_OffsetBYFZ);
    float bzfx = *(pixel + m_OffsetBZFX);
    
    float fxby = *(pixel + m_OffsetFXBY);
    float fybz = *(pixel + m_OffsetFYBZ);
    float fzbx = *(pixel + m_OffsetFZBX);
                          
    float bxby = *(pixel + m_OffsetBXBY);
    float bybz = *(pixel + m_OffsetBYBZ);
    float bzbx = *(pixel + m_OffsetBZBX);

    // Compute the mixed derivatives
    float uxy = 0.25f * (fxfy + bxby - bxfy - fxby);
    float uyz = 0.25f * (fyfz + bybz - byfz - fybz);
    float uzx = 0.25f * (fzfx + bzbx - bzfx - fzbx);

    curvature_term = m_CurvatureWeight * 
      ((uxc2+uyc2)*uzz + (uyc2+uzc2)*uxx + (uzc2+uxc2)*uyy
       - 2*( uxy*uxc*uyc + uyz*uyc*uzc + uzx*uzc*uxc ))
       /  gd->m_GradMagSqr;

    curvature_term *= this->CurvatureSpeed(it, offset);
    }

  // Compute the advection term
  float advection_term = 0.0f;  
  if (m_AdvectionWeight != 0.0f)
    {    
    VectorType advection_field = this->AdvectionField(it, offset, gd);
    float ax = advection_field[0] * m_AdvectionWeight;
    float ay = advection_field[1] * m_AdvectionWeight;
    float az = advection_field[2] * m_AdvectionWeight;
    
    advection_term = 
      ((ax > 0) ? uxb * ax : uxf * ax) +
      ((ay > 0) ? uyb * ay : uyf * ay) + 
      ((az > 0) ? uzb * az : uzf * az);

    // Compute the maximal advection change
    float maxaxay = (ax > ay) ? ax : ay;
    float maxazam = (az > gd->m_MaxAdvectionChange) ? az : gd->m_MaxAdvectionChange;
    gd->m_MaxAdvectionChange = (maxaxay > maxazam) ? maxaxay : maxazam;
    }

  float propagation_term = 0.0f;
  if (m_PropagationWeight != 0.0f)
    {
    // Get the propagation speed
    propagation_term = m_PropagationWeight * this->PropagationSpeed(it, offset, gd);

    float propagation_gradient = (propagation_term > 0) ? 
      vnl_math_sqr( vnl_math_max(uxb, 0.0f) ) + vnl_math_sqr( vnl_math_min(uxf, 0.0f) ) + 
      vnl_math_sqr( vnl_math_max(uyb, 0.0f) ) + vnl_math_sqr( vnl_math_min(uyf, 0.0f) ) + 
      vnl_math_sqr( vnl_math_max(uzb, 0.0f) ) + vnl_math_sqr( vnl_math_min(uzf, 0.0f) ) 
      :      
      vnl_math_sqr( vnl_math_min(uxb, 0.0f) ) + vnl_math_sqr( vnl_math_max(uxf, 0.0f) ) + 
      vnl_math_sqr( vnl_math_min(uyb, 0.0f) ) + vnl_math_sqr( vnl_math_max(uyf, 0.0f) ) + 
      vnl_math_sqr( vnl_math_min(uzb, 0.0f) ) + vnl_math_sqr( vnl_math_max(uzf, 0.0f) );
    
    // Collect energy change from propagation term.  This will be used in
    // calculating the maximum time step that can be taken for this iteration.
    gd->m_MaxPropagationChange = 
      vnl_math_max(gd->m_MaxPropagationChange,vnl_math_abs(propagation_term));
    
    // Scale the propagation term by gradient magnitude
    propagation_term *= vcl_sqrt( propagation_gradient );
    }

  float laplacian_term = 0.0f;
  if(m_LaplacianSmoothingWeight != 0.0f)
    {
    laplacian_term = (uxx + uyy + uzz)
     * m_LaplacianSmoothingWeight * LaplacianSmoothingSpeed(it,offset, gd);
    }

  // Return the combination of all the terms.
  return curvature_term - propagation_term - advection_term - laplacian_term;
}

#endif //  _USE_FastLevelSetFunction_

/** Used to report on each iteration */
class IterationReporter {
public:
  typedef itk::Image<float,3> ImageType;
  typedef itk::ImageRegionConstIterator<ImageType> IteratorType;
  typedef itk::ImageToImageFilter<ImageType,ImageType> FilterType;

  IterationReporter(FilterType *source,ofstream &fout)
  : m_Iteration(0), m_Out(fout), m_Source(source) 
  {
    m_Command = CommandType::New();
    m_Command->SetCallbackFunction(this,&IterationReporter::Callback);
    source->AddObserver(itk::IterationEvent(),m_Command);    
    m_LastClock = clock();
  }

  void Callback(itk::Object *object, const itk::EventObject &event)
  {
    // Record the time
    clock_t currentClock = clock();
    double delta = (currentClock - m_LastClock);
    m_Iteration++;

    // Write data
    m_Out << m_Iteration << "\t" << delta << /* "\t" << CountInterfaceVoxels() << */ endl;
  }

  unsigned long CountInterfaceVoxels()
  {
    ImageType *out = m_Source->GetOutput();
    IteratorType it(out,out->GetBufferedRegion());
    unsigned long count = 0;
    while(!it.IsAtEnd())
      {
      if(it.Value() > -1.0f && it.Value() < 1.0f)
        count++;
      ++it;
      }
    return count;
  }

private:
  int m_Iteration;
  clock_t m_LastClock;
  ofstream &m_Out;

  typedef itk::MemberCommand<IterationReporter> CommandType;
  CommandType::Pointer m_Command;
  FilterType::Pointer m_Source;
};

float 
TestCompareLevelSets
::ComputeOverlapDistance(FloatImageType *i1,FloatImageType *i2)
{
  typedef itk::ImageRegionConstIterator<FloatImageType> IteratorType;

  IteratorType it1(i1,i1->GetBufferedRegion());
  IteratorType it2(i2,i2->GetBufferedRegion());

  unsigned long nEither = 0;
  unsigned long nBoth = 0;

  while(!it1.IsAtEnd()) 
    {
    float v1 = it1.Value();
    float v2 = it2.Value();
    
    if(v1 <= 0 || v2 <= 0) nEither++;
    if(v1 <= 0 && v2 <= 0) nBoth++;

    ++it1;
    ++it2;
    }

  return nBoth * 1.0f / nEither;
}

void 
TestCompareLevelSets
::RunExperiment() 
{
  // We will use a registry to save orload configuration parameters
  Registry registry;
  try
  {    
    registry.ReadFromFile(m_Command.GetOptionParameter("config"));
  }
  catch(Registry::IOException &exc)
  {
    cerr << "Error: " << exc << endl;
    cerr << "Hint: Try generating a config file using 'generate' option!" << endl;
    throw new TestUsageException;
  }

  // Get the experiment ID
  string expID = registry["ExperimentId"]["000"];

  // Some image pointers
  GreyImageWrapper::ImagePointer grey;
  LabelImageWrapper::ImagePointer bubble;
  SpeedImageWrapper::ImagePointer speed;

  // Get the grey image from the registry
  LoadImageFromFile(registry["Image.Grey"][""],grey);

  // Get the snake initialization (segmentation from the registry)
  LoadImageFromFile(registry["Image.Bubble"][""],bubble);

  // Get the preprocessing image from the registry
  LoadImageFromFile(registry["Image.Speed"][""],speed);

  // Initialize an IRIS application 
  IRISApplication *app = new IRISApplication();
  
  // Pass the images to the application
  app->UpdateIRISGreyImage(grey,"RAI");
  app->UpdateIRISSegmentationImage(bubble);

  // Specify the label used for segmentation
  app->GetGlobalState()->SetDrawingColorLabel(
    (LabelType)registry["Image.BubbleLabel"][255]);

  // Start SNAP logic with the entire region
  app->InitializeSNAPImageData(
    app->GetCurrentImageData()->GetImageRegion());

  // Get a pointer to the SNAP image data
  SNAPImageData *snap = app->GetSNAPImageData();

  // Get the snake mode
  bool edgeMode = registry["Image.SpeedIsEdge"][false];

  // A registry IO object to read/write SNAP properties
  SNAPRegistryIO regio;

  // Pass in the speed image
  // Preprocess the image and initialize the level set image
  SnakeParameters parameters;
  if(edgeMode)
    {
    app->UpdateSNAPSpeedImage(speed,EDGE_SNAKE);
    parameters = regio.ReadSnakeParameters(
      registry.Folder("SnakeParameters.Edge"),
      SnakeParameters::GetDefaultEdgeParameters());
    }    
  else
    {
    app->UpdateSNAPSpeedImage(speed,IN_OUT_SNAKE);
    parameters = regio.ReadSnakeParameters(
      registry.Folder("SnakeParameters.InOut"),
      SnakeParameters::GetDefaultInOutParameters());
    }

  // Done with the initialization.  Open an output dump file
  string targetPath = registry["OutputPath"]["."] + string("/");
  ofstream fout((targetPath + "report." + expID + ".txt").c_str());

  // Now, we have a speed image and a level set image.  We are ready to
  // test different segmenters
  typedef SpeedImageWrapper::ImageType FloatImageType;
  typedef itk::DenseFiniteDifferenceImageFilter<
    FloatImageType,FloatImageType> DenseFilterType;  
  typedef itk::SparseFieldLevelSetImageFilter<
    FloatImageType,FloatImageType> SparseFilterType;
  typedef itk::ParallelSparseFieldLevelSetImageFilter<
    FloatImageType,FloatImageType> ParallelSparseFilterType;
  typedef itk::NarrowBandLevelSetImageFilter<
    FloatImageType,FloatImageType> NarrowFilterType;

  typedef LevelSetExtensionFilter<DenseFilterType> DenseExtensionFilter;
  typedef LevelSetExtensionFilter<SparseFilterType> SparseExtensionFilter;
  typedef LevelSetExtensionFilter<ParallelSparseFilterType> ParallelExtensionFilter;
  typedef LevelSetExtensionFilter<NarrowFilterType> NarrowExtensionFilter;

  // Pull out the finite difference function
  snap->InitializeSegmentationPipeline(parameters,NULL,0,
                           app->GetGlobalState()->GetDrawingColorLabel());
  SNAPLevelSetFunction<FloatImageType> *phi = snap->GetLevelSetFunction();

  // Decide on a number of iterations
  unsigned int nIterations = registry["Iterations"][10];

  // Set up the dense filter
  DenseExtensionFilter::Pointer fltDense = DenseExtensionFilter::New();
  fltDense->SetInput(snap->GetSnake()->GetImage());
  fltDense->SetDifferenceFunction(phi);
  fltDense->SetNumberOfIterations(nIterations);

  // Set up the sparse fileter
  SparseExtensionFilter::Pointer fltSparse = SparseExtensionFilter::New();
  fltSparse->SetInput(snap->GetSnake()->GetImage());
  fltSparse->SetDifferenceFunction(phi);
  fltSparse->SetNumberOfIterations(nIterations);
  fltSparse->SetNumberOfLayers(3);
  fltSparse->SetIsoSurfaceValue(0.0f);

  ParallelExtensionFilter::Pointer fltParallel = ParallelExtensionFilter::New();
  fltParallel->SetInput(snap->GetSnake()->GetImage());
  fltParallel->SetDifferenceFunction(phi);
  fltParallel->SetNumberOfIterations(nIterations);
  fltParallel->SetNumberOfLayers(3);
  fltParallel->SetIsoSurfaceValue(0.0f);

  // Set up the narrow band filter
  NarrowExtensionFilter::Pointer fltNarrow = NarrowExtensionFilter::New();
  fltNarrow->SetSegmentationFunction(phi);
  fltNarrow->SetInput(snap->GetSnake()->GetImage());
  fltNarrow->SetFeatureImage(snap->GetSpeed()->GetImage());  
  fltNarrow->SetNumberOfIterations(nIterations);

  // Create iteration reporters
  IterationReporter irDense(fltDense,fout);
  IterationReporter irSparse(fltSparse,fout);
  IterationReporter irNarrow(fltNarrow,fout);
  IterationReporter irParallel(fltParallel,fout);

  // Create a filter for filling in the interior of a distance transform
  typedef itk::UnaryFunctorImageFilter<
    FloatImageType,LabelImageWrapper::ImageType,InteriorFunctor> InteriorFilter;
  InteriorFilter::Pointer fltInterior = InteriorFilter::New();

  // Run and save the three filters
  fout << "Running the Narrow filter!" << endl;
  fltNarrow->Update();

  fout << "Running the Sparse filter!" << endl;
  fltSparse->Update();

  fout << "Running the Parallel filter!" << endl;
  fltParallel->Update();
  
  // fout << "Running the Dense filter!" << endl;
  // fltDense->Update();

  // Compute the difference between filter outputs
  fout << "Narrow-Sparse\t" << 
    ComputeOverlapDistance(fltNarrow->GetOutput(),fltSparse->GetOutput()) << endl;
  fout << "Narrow-Dense\t" << 
    ComputeOverlapDistance(fltNarrow->GetOutput(),fltDense->GetOutput()) << endl;
  fout << "Narrow-Parallel\t" << 
    ComputeOverlapDistance(fltNarrow->GetOutput(),fltParallel->GetOutput()) << endl;
  fout << "Sparse-Dense\t" << 
    ComputeOverlapDistance(fltSparse->GetOutput(),fltDense->GetOutput()) << endl;
  fout << "Sparse-Parallel\t" << 
    ComputeOverlapDistance(fltSparse->GetOutput(),fltParallel->GetOutput()) << endl;
  fout << "Dense-Parallel\t" << 
    ComputeOverlapDistance(fltDense->GetOutput(),fltParallel->GetOutput()) << endl;
  
  // Write each result to a file
  fltInterior->SetInput(fltNarrow->GetOutput());
  string file = targetPath + "CompareLevelSets." + expID + ".narrow.gipl";
  SaveImageToFile(file.c_str(),fltInterior->GetOutput());
  
  fltInterior->SetInput(fltDense->GetOutput());
  file = targetPath + "CompareLevelSets." + expID + ".dense.gipl";
  SaveImageToFile(file.c_str(),fltInterior->GetOutput());
  
  fltInterior->SetInput(fltSparse->GetOutput());
  file = targetPath + "CompareLevelSets." + expID + ".sparse.gipl";
  SaveImageToFile(file.c_str(),fltInterior->GetOutput());
  
  fltInterior->SetInput(fltParallel->GetOutput());
  file = targetPath + "CompareLevelSets." + expID + ".parallel.gipl";
  SaveImageToFile(file.c_str(),fltInterior->GetOutput());

  // Close the output
  fout.close();
}

void 
TestCompareLevelSets
::Run() 
{
  RunExperiment();
  return;

  // Run the parent's part of the test (loads image)
  Superclass::Run();

  // We need to wrap the image for some operations
  GreyImageWrapper wrapper;
  wrapper.SetImage(m_Image);
  
  // We need some default parameters for the test
  SnakeParameters parmSnakeEdge = 
    SnakeParameters::GetDefaultEdgeParameters();

  SnakeParameters parmSnakeInOut = 
    SnakeParameters::GetDefaultInOutParameters();

  EdgePreprocessingSettings parmPreprocessEdge = 
    EdgePreprocessingSettings::MakeDefaultSettings();

  ThresholdSettings parmPreprocessInOut = 
    ThresholdSettings::MakeDefaultSettings(&wrapper);

  // Some other defaults: which snake to use
  bool parmUseEdgeSnake = true;
  
  // Bubble default, based on the image
  Vector3f parmBubbleCenter = to_float(wrapper.GetSize()) / 2.0f;
  float parmBubbleRadius = 2.0;

  // Get the configuration file
  if(!m_Command.IsOptionPresent("config"))
    throw new TestUsageException;

  // We will use a registry to save orload configuration parameters
  Registry registry;

  // A registry IO object to read/write SNAP properties
  SNAPRegistryIO regio;

  // Check if the user wants to generate a config file
  if(m_Command.IsOptionPresent("generate"))
    {
    // Store the options in the registry
    regio.WriteSnakeParameters(
      parmSnakeEdge,registry.Folder("SnakeParameters.Edge"));

    regio.WriteSnakeParameters(
      parmSnakeInOut,registry.Folder("SnakeParameters.InOut"));

    regio.WriteEdgePreprocessingSettings(
      parmPreprocessEdge,registry.Folder("Preprocessing.Edge"));

    regio.WriteThresholdSettings(
      parmPreprocessInOut,registry.Folder("Preprocessing.InOut"));

    // Store the default position (center of image, kinda-dumb)
    registry["Seed.Position"] << parmBubbleCenter;
    registry["Seed.Radius"] << parmBubbleRadius;

    // Store the snake mode to use
    registry["SnakeMode"] << parmUseEdgeSnake;

    // Write the registry
    registry.WriteToFile(m_Command.GetOptionParameter("config"));
    return;
    }

  // Try loading the registry
  try
  {    
    registry.ReadFromFile(m_Command.GetOptionParameter("config"));
  }
  catch(Registry::IOException &exc)
  {
    cerr << "Error: " << exc << endl;
    cerr << "Hint: Try generating a config file using 'generate' option!" << endl;
    throw new TestUsageException;
  }

  // Read snake parameters from the registry
  parmSnakeEdge = regio.ReadSnakeParameters(
    registry.Folder("SnakeParameters.Edge"),parmSnakeEdge);
  
  parmSnakeInOut = regio.ReadSnakeParameters(
    registry.Folder("SnakeParameters.InOut"),parmSnakeInOut);

  // Read preprocessing settings from the registry
  parmPreprocessEdge = regio.ReadEdgePreprocessingSettings(
    registry.Folder("Preprocessing.Edge"),parmPreprocessEdge);

  parmPreprocessInOut = regio.ReadThresholdSettings(
    registry.Folder("Preprocessing.InOut"),parmPreprocessInOut);

  // Read the seed position 
  parmBubbleCenter = registry["Seed.Position"][parmBubbleCenter];
  parmBubbleRadius = registry["Seed.Radius"][parmBubbleRadius];

  // Read the snake mode to use
  parmUseEdgeSnake = registry["SnakeMode"][parmUseEdgeSnake];

  /** Now we have some configuration parameters and an image to work with.  
   Let's get to the gritty of it! */

  // Initialize an IRIS application 
  IRISApplication *app = new IRISApplication();
  
  // Pass the image to the application
  app->UpdateIRISGreyImage(m_Image,"RAI");

  // Start SNAP logic with the entire region
  app->InitializeSNAPImageData(app->GetCurrentImageData()->GetImageRegion());

  // Get a pointer to the SNAP image data
  SNAPImageData *snap = app->GetSNAPImageData();

  // Create an initialization bubble
  Bubble bubble;
  bubble.center = to_int(parmBubbleCenter);
  bubble.radius = (int) parmBubbleRadius;

  // Preprocess the image and initialize the level set image
  if(parmUseEdgeSnake)
    {
    snap->DoEdgePreprocessing(parmPreprocessEdge);
    snap->InitializeSegmentationPipeline(parmSnakeEdge,&bubble,1,255);
    }    
  else
    {
    snap->DoInOutPreprocessing(parmPreprocessInOut);
    snap->InitializeSegmentationPipeline(parmSnakeEdge,&bubble,1,255);
    }
    
  // Now, we have a speed image and a level set image.  We are ready to
  // test different segmenters
  typedef SpeedImageWrapper::ImageType FloatImageType;
  typedef itk::DenseFiniteDifferenceImageFilter<
    FloatImageType,FloatImageType> DenseFilterType;  
  typedef itk::SparseFieldLevelSetImageFilter<
    FloatImageType,FloatImageType> SparseFilterType;
  typedef itk::NarrowBandLevelSetImageFilter<
    FloatImageType,FloatImageType> NarrowFilterType;

  typedef LevelSetExtensionFilter<DenseFilterType> DenseExtensionFilter;
  typedef LevelSetExtensionFilter<SparseFilterType> SparseExtensionFilter;
  typedef LevelSetExtensionFilter<NarrowFilterType> NarrowExtensionFilter;

  // Pull out the finite difference function
  SNAPLevelSetFunction<FloatImageType> *phi = snap->GetLevelSetFunction();

  // Decide on a number of iterations
  unsigned int nIterations = 10;

  // Create a callback object
  typedef itk::MemberCommand<TestCompareLevelSets> CommandType;
  CommandType::Pointer callback = CommandType::New();
  callback->SetCallbackFunction(this,&TestCompareLevelSets::IterationCallback);

  // Set up the dense filter
  DenseExtensionFilter::Pointer fltDense = DenseExtensionFilter::New();
  fltDense->SetInput(snap->GetSnake()->GetImage());
  fltDense->SetDifferenceFunction(phi);
  fltDense->SetNumberOfIterations(nIterations);
  fltDense->AddObserver(itk::IterationEvent(),callback);

  // Set up the sparse fileter
  SparseExtensionFilter::Pointer fltSparse = SparseExtensionFilter::New();
  fltSparse->SetInput(snap->GetSnake()->GetImage());
  fltSparse->SetDifferenceFunction(phi);
  fltSparse->SetNumberOfIterations(nIterations);
  fltSparse->SetNumberOfLayers(3);
  fltSparse->SetIsoSurfaceValue(0.0f);
  fltSparse->AddObserver(itk::IterationEvent(),callback);

  // Set up the narrow band filter
  NarrowExtensionFilter::Pointer fltNarrow = NarrowExtensionFilter::New();
  fltNarrow->SetSegmentationFunction(phi);
  fltNarrow->SetInput(snap->GetSnake()->GetImage());
  fltNarrow->SetFeatureImage(snap->GetSpeed()->GetImage());  
  fltNarrow->SetNumberOfIterations(nIterations);
  fltNarrow->AddObserver(itk::IterationEvent(),callback);

  // Create a filter for filling in the interior of a distance transform
  typedef itk::UnaryFunctorImageFilter<
    FloatImageType,LabelImageWrapper::ImageType,InteriorFunctor> InteriorFilter;
  InteriorFilter::Pointer fltInterior = InteriorFilter::New();

  // Run and save the narrow filter
  cout << "Running the Narrow filter!" << endl;
  //fltInterior->SetInput(fltNarrow->GetOutput());
  //fltInterior->Update();
  //SaveImageToFile("CompareLevelSets.narrow.gipl",fltInterior->GetOutput());
  
  // Run and save the dense filter
  cout << "Running the Dense filter!" << endl;
  //fltInterior->SetInput(fltDense->GetOutput());
  //fltInterior->Update();
  //SaveImageToFile("CompareLevelSets.dense.gipl",fltInterior->GetOutput());

  // Run and save the sparse filter
  cout << "Running the Sparse filter!" << endl;
  fltInterior->SetInput(fltSparse->GetOutput());
  fltInterior->Update();
  SaveImageToFile("CompareLevelSets.sparse.gipl",fltInterior->GetOutput());
}

void 
TestCompareLevelSets
::IterationCallback(itk::Object *object, const itk::EventObject &irisNotUsed(event))
{
  static clock_t lastTime = clock();
  clock_t currentTime = clock();
  double delta = (double)(currentTime - lastTime) / CLOCKS_PER_SEC;
  double progress = 
    reinterpret_cast<itk::ProcessObject *>(object)->GetProgress();

  cout << setw(20) << progress << setw(20) << delta * 1000 << endl;
}


