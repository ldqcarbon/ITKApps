#ifndef __DeformableModelApplicationBase__h__
#define __DeformableModelApplicationBase__h__

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#endif

#include "itkImage.h"
#include "itkMesh.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkCastImageFilter.h"

#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

#include "itkShiftScaleImageFilter.h"

#include "itkVTKImageExport.h"
#include "itkSigmoidImageFilter.h"

#include "itkImageToVTKImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkDefaultDynamicMeshTraits.h"
#include "itkSimplexMesh.h"
#include "itkRegularSphereMeshSource.h"
#include "itkTriangleMeshToSimplexMeshFilter.h"
#include "itkVertexCell.h"
#include "itkCellInterfaceVisitor.h"
#include "itkCommand.h"

#include "itkDeformableSimplexMesh3DBalloonForceFilter.h"
#include "itkDeformableSimplexMesh3DFilter.h"
#include "itkGradientRecursiveGaussianImageFilter.h"
#include "itkSobelEdgeDetectionImageFilter.h"


class DeformableModelApplicationBase 
{

public:
  
  typedef float   PixelType;

  typedef unsigned char  VisualizationPixelType;
  
  typedef itk::Image< PixelType, 3 >                  VolumeType;
  
  typedef itk::Image< float, 3 >                      CastType;
  typedef itk::Image<unsigned char, 3>               MeshPixelType;

  typedef itk::Vector<float, 3>                      VectorPixelType;
  typedef itk::Image<VectorPixelType, 3>             VectorImageType;
  
  typedef itk::ImageFileReader< VolumeType >          VolumeReaderType;

  typedef itk::CastImageFilter < VolumeType, CastType > CastImageType;

  typedef itk::GradientAnisotropicDiffusionImageFilter < CastType, CastType >  GradientAnisotropicImageType;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter < CastType, CastType >  GradientMagnitudeType;

  
  typedef itk::ImageToVTKImageFilter < MeshPixelType > ImageToVTKImageType;
  
  typedef itk::SigmoidImageFilter< CastType, CastType > SigmoidImageType;
 
  typedef itk::Image< VisualizationPixelType, 3 >     VisualizationVolumeType;
  
  typedef itk::RescaleIntensityImageFilter< 
                          VolumeType,
                          VisualizationVolumeType >   RescaleIntensityFilterType;

  typedef itk::ImageToVTKImageFilter< 
                          VisualizationVolumeType >   ITK2VTKAdaptorFilterType;


 
   // Declare the type of the input and output mesh
  typedef itk::DefaultDynamicMeshTraits<double, 3, 3,double,double> TriangleMeshTraits;
  typedef itk::DefaultDynamicMeshTraits<double, 3, 3, double,double> SimplexMeshTraits;
  typedef itk::Mesh<double,3, TriangleMeshTraits> TriangleMeshType;

  typedef itk::SimplexMesh<double,3, SimplexMeshTraits> SimplexMeshType;

  typedef itk::VTKImageExport < MeshPixelType > VTKImageExportType;

  // declare triangle mesh source (inital mesh must be replaced with creating
  // mesh from binary image using marching cubes
  typedef itk::RegularSphereMeshSource<TriangleMeshType>  SphereMeshSourceType;
  typedef SphereMeshSourceType::PointType PointType;
  typedef SphereMeshSourceType::VectorType VectorType;


  // declare the triangle to simplex mesh filter
  typedef itk::TriangleMeshToSimplexMeshFilter<TriangleMeshType, SimplexMeshType> SimplexFilterType;

  typedef SimplexMeshType::CellsContainer::ConstIterator  CellIterator;
  typedef itk::CellInterface<
                      SimplexMeshType::PixelType, 
                      SimplexMeshType::CellTraits >  CellInterfaceType;
  typedef itk::VertexCell<CellInterfaceType>      vertexCell;
  typedef itk::LineCell<CellInterfaceType> lineCell;
  
 
  typedef SimplexMeshType::LineType  lineType;
  typedef SimplexMeshType::CellType  cellType;

  // Deformable Models

  typedef itk::DeformableSimplexMesh3DFilter<SimplexMeshType,SimplexMeshType> DeformFilterType;
 
  typedef itk::SobelEdgeDetectionImageFilter<CastType,CastType> EdgeFilterType;

  typedef DeformFilterType::GradientImageType       GradientImageType;

  typedef itk::GradientRecursiveGaussianImageFilter<CastType,GradientImageType> GradientFilterType;
  
  typedef itk::SimpleMemberCommand< DeformableModelApplicationBase >  IterationObserverType;

public:
  
  DeformableModelApplicationBase();
  
  virtual ~DeformableModelApplicationBase();
  
  virtual void SetSeedPoint( double x, double y, double z );
  
  virtual void  GetSeedPoint(double data[3]); 
  
  virtual void  IterationCallback(); 
  
protected:
  
  double                                  m_SeedPoint[3];
  
  VolumeReaderType::Pointer               m_VolumeReader;

  CastImageType::Pointer                  m_CastImage;
  
  GradientAnisotropicImageType::Pointer   m_GradientAnisotropicImage;

  GradientMagnitudeType::Pointer          m_GradientMagnitude;

  SigmoidImageType::Pointer               m_SigmoidImage;
  
  RescaleIntensityFilterType::Pointer     m_RescaleIntensity;

  ITK2VTKAdaptorFilterType::Pointer       m_ITK2VTKAdaptor;
  
  VolumeType::ConstPointer                m_LoadedVolume;

  SphereMeshSourceType::Pointer           m_SphereMeshSource;

  VTKImageExportType::Pointer             m_VTKImageExport;

  SimplexFilterType::Pointer              m_SimplexFilter;

  SimplexMeshType::Pointer                m_SimplexMesh;

  SimplexMeshType::Pointer                m_SimplexMeshToShow;
 
  DeformFilterType::Pointer               m_DeformFilter;

  GradientFilterType::Pointer             m_GradientFilter;
  
  IterationObserverType::Pointer          m_IterationObserver;

  ImageToVTKImageType::Pointer            m_ImageToVTKImage;

  TriangleMeshType::Pointer               m_TriangleMesh;
  SimplexFilterType::Pointer              m_SimplexMeshFilter;
  SimplexMeshType::Pointer                m_SimplexMeshLoaded;

};


#endif