#include "LandmarkRegistrator.h"

LandmarkRegistrator::LandmarkRegistrator()
  {
  m_FixedLandmarkSet = LandmarkSetType::New();
  m_MovingLandmarkSet = LandmarkSetType::New();
  m_Optimizer = OptimizerType::New();
  m_Generator = NormalGeneratorType::New();
  m_Transform = TransformType::New();
  m_Metric = MetricType::New();

  m_Transform->SetIdentity();
  m_InitialTransformParameters = m_Transform->GetParameters();

  m_OptimizerNumberOfIterations = 5000;

  m_OptimizerScales.resize(9);
  m_OptimizerScales[0] = 500; // rotation
  m_OptimizerScales[1] = 500;
  m_OptimizerScales[2] = 500;
  m_OptimizerScales[3] = 200; // center of rotation
  m_OptimizerScales[4] = 200;
  m_OptimizerScales[5] = 200;
  m_OptimizerScales[6] = 10;   // offset
  m_OptimizerScales[7] = 10;
  m_OptimizerScales[8] = 10;
  }

LandmarkRegistrator
::~LandmarkRegistrator()
  {
  }

void
LandmarkRegistrator
::StartRegistration()
  { 
  m_Generator->Initialize(1289);

  if( m_FixedLandmarkSet->Size() != m_MovingLandmarkSet->Size() )
    {
    itk::ExceptionObject e("LandmarkRegistrator.txx",77);
    e.SetLocation("LandmarkRegistrator::Register()");
    e.SetDescription("Landmark sets do not have the same number of elements");
    throw(e);
    }

  ParametersType fixedCenter;
  fixedCenter.resize(3);
  fixedCenter.Fill(0);
  ParametersType movingCenter;
  movingCenter.resize(3);
  movingCenter.Fill(0);
  for( unsigned int i=0; i<m_MovingLandmarkSet->Size(); i++)
    {
    for( unsigned int j=0; j<3; j++)
      {
      fixedCenter[j] += m_FixedLandmarkSet->ElementAt(i)[j];
      movingCenter[j] += m_MovingLandmarkSet->ElementAt(i)[j];
      }
    }
  for( unsigned int j=0; j<3; j++)
    {
    fixedCenter[j] /= m_FixedLandmarkSet->Size();
    movingCenter[j] /= m_MovingLandmarkSet->Size();
    }
  m_InitialTransformParameters[3] = movingCenter[0];
  m_InitialTransformParameters[4] = movingCenter[1];
  m_InitialTransformParameters[5] = movingCenter[2];
  m_InitialTransformParameters[6] = fixedCenter[0]-movingCenter[0];
  m_InitialTransformParameters[7] = fixedCenter[1]-movingCenter[1];
  m_InitialTransformParameters[8] = fixedCenter[2]-movingCenter[2];

  std::cout << "LandmarkRegistrator: InitialParameters = " << std::endl
            << m_InitialTransformParameters << std::endl;
  try
    {
    m_Metric->SetFixedPointSet(m_FixedLandmarkSet);
    m_Metric->SetMovingPointSet(m_MovingLandmarkSet);

    m_Optimizer->SetInitialPosition(m_InitialTransformParameters);
    m_Optimizer->SetScales(m_OptimizerScales);
    m_Optimizer->SetCostFunction(m_Metric);
    m_Optimizer->SetNormalVariateGenerator(m_Generator);
    m_Optimizer->SetMaximumIteration(m_OptimizerNumberOfIterations);
    m_Optimizer->Initialize(4.0, 1.1, 0.9); // Initial search radius
    m_Optimizer->SetEpsilon(0.00000001);
    m_Optimizer->StartOptimization();
    }
  catch( itk::ExceptionObject &e )
    {
    std::cout<<"-------------------------------------------------"<<std::endl;
    std::cout<<"Exception caught in LandmarkRegistrator:"<<std::endl;
    std::cout<<e<<std::endl;
    std::cout<<"-------------------------------------------------"<<std::endl;
    }
  catch( ... )
    {
    std::cout<<"-------------------------------------------------"<<std::endl;
    std::cout<<"Exception caught in LandmarkRegistrator:"<<std::endl;
    std::cout<<"unknown exception caught !!!"<<std::endl;
    std::cout<<"-------------------------------------------------"<<std::endl;
    }

  m_Transform = TransformType::New();
  m_Transform->SetParameters(m_Optimizer->GetCurrentPosition());
  }

void 
LandmarkRegistrator
::PrintSelf( std::ostream &os, itk::Indent indent ) const
  {
  os<<"-------------------------------------"<<std::endl;
  Superclass::PrintSelf(os,indent);
  os<<"FixedLandmarkSet: "<<m_FixedLandmarkSet<<std::endl;
  os<<"MovingLandmarkSet: "<<m_MovingLandmarkSet<<std::endl;
  os<<"Metric Function: "<<m_Metric<<std::endl;
  os<<"OptimizerScales: "<<m_OptimizerScales<<std::endl;
  os<<"Transform: "<<m_Transform<<std::endl;
  os<<"Optimizer: "<<m_Optimizer<<std::endl;
  }

void 
LandmarkRegistrator
::CopyLandmarkSet( LandmarkSetType::Pointer source,
                   LandmarkSetType::Pointer dest ) const
  {
  unsigned int i;
  unsigned int size= source->Size();
  
  dest->Initialize();

  for( i=0; i<size; i++ )
    {
    dest->InsertElement(i,source->GetElement(i));
    }
  }