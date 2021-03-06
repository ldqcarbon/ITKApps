/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    MRFKmeansImageMSClassifierApp.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _MRFKmeansImageMSClassifierApp_txx
#define _MRFKmeansImageMSClassifierApp_txx

#include "MRFKmeansImageMSClassifierApp.h"

namespace itk
{

template <typename TVectorInputImage, typename TMaskImage>
MRFKmeansImageMSClassifierApp<TVectorInputImage,TMaskImage>
::MRFKmeansImageMSClassifierApp()
{
  m_VectorInputImage   = NULL;
  m_MaskInputImage     = NULL;
  m_ClassifiedImage    = NULL;
  m_NumberOfClasses    = 1;
  m_NumberOfChannels   = 1;

  //-------------------------------------------------------------------
  // Initialize the containers for means/covariance/number of samples 
  //-------------------------------------------------------------------

  m_ClassMeans.resize( m_NumberOfClasses, m_NumberOfChannels );
  m_ClassMeans.fill( 0 );

  m_ClassCovariances.resize( m_NumberOfClasses );

  for(unsigned int i = 0; i < m_NumberOfClasses; i++ )
    {
    m_ClassCovariances[i].resize( m_NumberOfChannels, m_NumberOfChannels );
    m_ClassCovariances[i].set_identity( );
    }

  m_ClassNumberOfSamples.resize( m_NumberOfClasses, m_NumberOfChannels );
  m_ClassNumberOfSamples.fill( 0 );

}


template <typename TVectorInputImage, typename TMaskImage>
void
MRFKmeansImageMSClassifierApp<TVectorInputImage,TMaskImage>
::Execute()
{

  //----------------------------------------------------------------------
  //Set membership function (Using the statistics objects)
  //----------------------------------------------------------------------

  typedef Statistics::DistanceToCentroidMembershipFunction< VectorInputPixelType > 
    MembershipFunctionType ;

  typedef MembershipFunctionType::Pointer MembershipFunctionPointer ;

  typedef std::vector< MembershipFunctionPointer > 
    MembershipFunctionPointerVector;

  //----------------------------------------------------------------------
  //Set the image model estimator
  //----------------------------------------------------------------------
  typedef itk::ImageKmeansModelEstimator< VectorInputImageType, 
    MembershipFunctionType> ImageKmeansModelEstimatorType;

  ImageKmeansModelEstimatorType::Pointer 
    applyKmeansModelEstimator = ImageKmeansModelEstimatorType::New();

  //------------------------------------------------------------------
  //Read the codebook into an vnl_matrix
  //------------------------------------------------------------------

  vnl_matrix<double> inCDBK(m_NumberOfClasses, m_NumberOfChannels);
  //There are 5 entries to the code book
  int r,c;
  r=0; c=0; inCDBK.put(r,c,0);
  r=0; c=1; inCDBK.put(r,c,0);
  r=1; c=0; inCDBK.put(r,c,20);
  r=1; c=1; inCDBK.put(r,c,20);
  r=2; c=0; inCDBK.put(r,c,600);
  r=2; c=1; inCDBK.put(r,c,700);
  r=3; c=0; inCDBK.put(r,c,700);
  r=3; c=1; inCDBK.put(r,c,1851);
  r=4; c=0; inCDBK.put(r,c,900);
  r=4; c=1; inCDBK.put(r,c,1300);

  //----------------------------------------------------------------------
  //Set the parameters of the clusterer
  //----------------------------------------------------------------------

  std::cout << "Starting to build the K-means model ....." << std::endl;

  applyKmeansModelEstimator->SetInputImage( m_VectorInputImage );
  applyKmeansModelEstimator->SetNumberOfModels(m_NumberOfClasses);
  applyKmeansModelEstimator->SetThreshold(0.01);
  applyKmeansModelEstimator->SetCodebook(inCDBK);

  applyKmeansModelEstimator->Update();

  MembershipFunctionPointerVector membershipFunctions = 
    applyKmeansModelEstimator->GetMembershipFunctions(); 



  std::vector<double> kmeansResultForClass(membershipFunctions.size());
  std::vector<double> kmeansRes( membershipFunctions.size() );

  
  
  std::cout << "Result of K-Means clustering" << std::endl;

  for(unsigned int classIndex=0; classIndex < membershipFunctions.size(); 
    classIndex++ )
    {
    kmeansResultForClass[classIndex] = 
      (double) (membershipFunctions[classIndex]->GetCentroid())[0];
    kmeansRes[classIndex] = 
      (double) (membershipFunctions[classIndex]->GetCentroid())[1];
    }

  for(unsigned int classIndex=0; classIndex < membershipFunctions.size(); 
    classIndex++ )
    {
    std::cout << kmeansResultForClass[ classIndex ] << " ; " 
      << kmeansRes[ classIndex ] << std::endl;
    }

  vnl_vector<double> temp =  membershipFunctions[0]->GetCentroid();
  for(unsigned int classIndex=0; classIndex < membershipFunctions.size(); 
    classIndex++ )
    {
    temp[0] = (double) kmeansResultForClass[classIndex];
    membershipFunctions[classIndex]->SetCentroid(temp);
    }  

  //----------------------------------------------------------------------
  //Set the decision rule 
  //----------------------------------------------------------------------  
  typedef DecisionRuleBase::Pointer DecisionRuleBasePointer;

  typedef MinimumDecisionRule DecisionRuleType;
  DecisionRuleType::Pointer  
    classifierDecisionRule = DecisionRuleType::New();

  //------------------------------------------------------
  //Instantiate the MSClassifier model (as the input image is in right format)
  //------------------------------------------------------  

  //Assign a class label image type
  typedef ImageClassifierBase< VectorInputImageType,ClassifiedImageType > 
    SupervisedClassifierType;

  SupervisedClassifierType::Pointer 
    classifierPointer = SupervisedClassifierType::New();


  typedef ShowProgressObject ProgressType;

  ProgressType progressWatch(classifierPointer);
  SimpleMemberCommand<ProgressType>::Pointer command;
  command = SimpleMemberCommand<ProgressType>::New();

  command->SetCallbackFunction(&progressWatch,
                               &ProgressType::ShowProgress);

  classifierPointer->AddObserver(itk::ProgressEvent(), command);

  //------------------------------------------------------  
  // Set the Classifier parameters
  //------------------------------------------------------  
  classifierPointer->SetNumberOfClasses( m_NumberOfClasses );
  classifierPointer->SetInputImage( m_VectorInputImage );

  // Set the decison rule 
  classifierPointer->
    SetDecisionRule( (DecisionRuleBasePointer) classifierDecisionRule );

  MembershipFunctionPointer membershipFunction;
  //------------------------------------------------------
  //Set the classifier membership functions
  //------------------------------------------------------
  for( unsigned int i=0; i<m_NumberOfClasses; i++ )
    {
    classifierPointer->AddMembershipFunction( membershipFunctions[i] );
    }
  
  //Do the classification
  //Run the kmeans classifier algorithm
  classifierPointer->Update();

  //Get the classified image
  typedef typename ClassifiedImageType::Pointer ClassifiedImagePointer;

  ClassifiedImagePointer    
    outClassImage = classifierPointer->GetClassifiedImage();

  //------------------------------------------------------
  //Set the MRF labeller and populate the parameters
  //------------------------------------------------------
  //Set the MRF labeller
  typedef itk::MRFImageFilter<VectorInputImageType,ClassifiedImageType> 
    MRFFilterType;

  MRFFilterType::Pointer applyMRFFilter = MRFFilterType::New();

  // Set the MRF labeller parameters
  applyMRFFilter->SetNumberOfClasses(m_NumberOfClasses);
  applyMRFFilter->SetMaximumNumberOfIterations(m_MaximumNumberOfIterations);
  applyMRFFilter->SetErrorTolerance(m_ErrorTolerance);
  applyMRFFilter->SetSmoothingFactor( m_SmoothingFactor );

  //For setting up a square/cubic or hypercubic neighborhood
  applyMRFFilter->SetNeighborhoodRadius( m_NeighborhoodRadius );
 
  applyMRFFilter->SetInput(m_VectorInputImage);
  applyMRFFilter->SetClassifier( classifierPointer ); 
  
  //Kick off the MRF labeller function
  applyMRFFilter->Update();

  this->SetClassifiedImage( 
    applyMRFFilter->GetOutput() );

}

} // namespace itk

#endif
