/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    ImageIOWizardLogic.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __ImageIOWizardLogic_txx_
#define __ImageIOWizardLogic_txx_

#include "FL/Fl_File_Chooser.H"
#include "FL/Fl_Text_Buffer.H"
//#include <limits>
#include <stdio.h>
#include <cmath>
#include <map>
#include <string>

#include "itkImage.h"
#include "itkImageIOBase.h"
#include "itkAnalyzeImageIO.h"
#include "itkGiplImageIO.h"
#include "itkMetaImageIO.h"
#include "itkRawImageIO.h"
#include "itkDicomImageIO.h"
#include "itkGE4ImageIO.h"
#include "itkGE5ImageIO.h"
#include "itkSiemensVisionImageIO.h"
#include "itkVTKImageIO.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataObject.h"

using std::map;
using namespace itk;
using namespace itk::SpatialOrientation;

template <class TRawPixel> 
itk::ImageIOBase::Pointer 
CreateRawImageIO(TRawPixel itkNotUsed(dummy),unsigned int header)
{
  typedef RawImageIO<TRawPixel,3> IOType;
  
  typename IOType::Pointer rawIO = IOType::New();
  
  rawIO->SetHeaderSize(header);

  ImageIOBase::Pointer baseIO = rawIO.GetPointer();
  
  return baseIO;
}

template <class TPixel>
ImageIOWizardLogic<TPixel>
::ImageIOWizardLogic() 
{                 
  // Initialize the menu item map
  m_MapOrientationIndexAndFlipToMenuItem[0][0] = 0;   // R in RAI
  m_MapOrientationIndexAndFlipToMenuItem[0][1] = 1;   // L in RAI
  m_MapOrientationIndexAndFlipToMenuItem[1][0] = 2;   // A in RAI
  m_MapOrientationIndexAndFlipToMenuItem[1][1] = 3;   // P in RAI
  m_MapOrientationIndexAndFlipToMenuItem[2][0] = 4;   // I in RAI
  m_MapOrientationIndexAndFlipToMenuItem[2][1] = 5;   // S in RAI

  // Initialize the axes to doll pages map
  m_MapOrientationIndexToDollPageIndex[0][1][2] = 0; // RAI
  m_MapOrientationIndexToDollPageIndex[0][2][1] = 1; // RIA
  m_MapOrientationIndexToDollPageIndex[1][0][2] = 2; // ARI
  m_MapOrientationIndexToDollPageIndex[1][2][0] = 3; // AIR
  m_MapOrientationIndexToDollPageIndex[2][0][1] = 4; // IRA
  m_MapOrientationIndexToDollPageIndex[2][1][0] = 5; // IAR

  // Initialize the axes to origin vertex map
  m_OrientationIndexToDollVertex[0][1][2] = 7; // 111 in RAI
  m_OrientationIndexToDollVertex[0][2][1] = 0; // 000 in RIA
  m_OrientationIndexToDollVertex[1][0][2] = 3; // 011 in ARI
  m_OrientationIndexToDollVertex[1][2][0] = 1; // 001 in AIR
  m_OrientationIndexToDollVertex[2][0][1] = 4; // 100 in IRA
  m_OrientationIndexToDollVertex[2][1][0] = 6; // 110 in IAR

  // Initialize the file format extensions 
  m_FileFormatPattern[FORMAT_MHA] = "mha,mhd";
  m_FileFormatPattern[FORMAT_GIPL] = "gipl,gipl.gz";
  m_FileFormatPattern[FORMAT_RAW] = "raw*";
  m_FileFormatPattern[FORMAT_ANALYZE] = "hdr,img,img.gz";  
  m_FileFormatPattern[FORMAT_DICOM] = "dcm";
  m_FileFormatPattern[FORMAT_GE4] = "ge4";
  m_FileFormatPattern[FORMAT_GE5] = "ge5";
  m_FileFormatPattern[FORMAT_SIEMENS] = "ima";
  m_FileFormatPattern[FORMAT_VTK] = "vtk";

  // Initialize the file format descriptions
  m_FileFormatDescription[FORMAT_MHA] = "MetaImage";
  m_FileFormatDescription[FORMAT_GIPL] = "GIPL";
  m_FileFormatDescription[FORMAT_RAW] = "Raw Binary";
  m_FileFormatDescription[FORMAT_ANALYZE] = "Analyze"; 
  m_FileFormatDescription[FORMAT_DICOM] = "DICOM";
  m_FileFormatDescription[FORMAT_GE4] = "GE Version 4";
  m_FileFormatDescription[FORMAT_GE5] = "GE Version 5";
  m_FileFormatDescription[FORMAT_SIEMENS] = "Siemens Vision";
  m_FileFormatDescription[FORMAT_VTK] = "VTK";
  
  // Initialize the text buffers
  m_SummaryTextBuffer = new Fl_Text_Buffer();

  // Initialize the callback pointer
  m_Callback = NULL;
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::MakeWindow()
{
  // Call the parent method
  ImageIOWizard::MakeWindow();

  // Initialize the file save dialog box based on the allowed file types
  for(unsigned int i = 0;i < FORMAT_COUNT;i++)
    {
    // Create an appropriate description
    StringType text = m_FileFormatDescription[i] + " File";
    
    // Add a menu option to the save menu, disabling it if it's unsupported
    m_InFilePageFormat->add(text.c_str(),0,NULL,NULL,
                            this->CanLoadFileFormat((FileFormat) i) ? 
                              0 : FL_MENU_INACTIVE);
    
    // Add a menu option to the save menu, disabling it if it's unsupported
    m_InSaveFilePageFormat->add(text.c_str(),0,NULL,NULL,
                                this->CanSaveFileFormat((FileFormat) i) ? 
                                  0 : FL_MENU_INACTIVE);
    }

  m_InSaveFilePageFormat->value(0);
  
  // Add the buffer to the summary text widget
  m_OutSummaryMetaData->buffer(m_SummaryTextBuffer);
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::GoBack(Fl_Group *current) 
{
  // The link to the last page
  Fl_Group *last = NULL;

  // In the input is NULL, get the current page from the wizard widget
  current = (current == NULL) ? (Fl_Group *)m_WizInput->value() : current;

  if (current == m_PageHeader)
    {
    last = m_PageFile;
    }
  else if (current == m_PageOrientation)
    {
    last = m_PageHeader;
    }
  else if (current == m_PageSummary)
    {
    last = m_PageOrientation;
    }
  else
    {
    assert(0 == "Next page not valid at this position in the wizard");
    }

  if (!last->active())
    {
    GoBack(last);
    } 
  else
    {
    m_WizInput->value(last);
    }
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::GoForward(Fl_Group *current) 
{
  // The link to the next page
  Fl_Group *next = NULL;

  // In the input is NULL, get the current page from the wizard widget
  current = (current == NULL) ? (Fl_Group *)m_WizInput->value() : current;

  // Follow the sequence of the pages
  if(current == m_PageFile) 
    {
    next = m_PageHeader;
    }
  else if (current == m_PageHeader)
    {
    next = m_PageOrientation;
    }    
  else if (current == m_PageOrientation)
    {
    next = m_PageSummary;
    }    
  else
    {
    assert(0 == "Next page not valid");
    }
    
  // Check if the page is active 
  if (!next->active())
    {
    GoForward(next);
    return;
    } 
    
  // Flip to the new page
  m_WizInput->value(next);

  // Call the enter-page method
  if (next == m_PageHeader)
    {
    OnHeaderPageEnter();
    }
  else if (next == m_PageOrientation)
    {
    OnOrientationPageEnter();
    }
  else if (next == m_PageSummary)
    {
    OnSummaryPageEnter();
    }
}

template <class TPixel>
typename ImageIOWizardLogic<TPixel>::StringType
ImageIOWizardLogic<TPixel>
::GetFilePattern(bool forLoading) 
{
  // String containing the whole patterns
  StringType pattern = "";
  bool patternNeedsTab = false;

  // String containing the "All Image Files" pattern
  StringType allImageFiles = "All Image Files (*.{"; 
  bool allImageFilesNeedsComma = false;

  // Go through all supported formats
  for(unsigned int i=0;i < FORMAT_COUNT;i++)
    {
    // Check if the file format is supported
    if((forLoading && this->CanLoadFileFormat((FileFormat) i)) ||
       (!forLoading && this->CanSaveFileFormat((FileFormat) i)))
      {
      // Add comma to allImageFiles
      if(allImageFilesNeedsComma)
        allImageFiles += ",";
      else
        allImageFilesNeedsComma = true;

      // Add extension to all image files
      allImageFiles += m_FileFormatPattern[i];

      // Add a tab to the pattern
      if(patternNeedsTab)
        pattern += "\t";
      else
        patternNeedsTab = true;

      // Construct the pattern
      pattern += m_FileFormatDescription[i];
      pattern += " Files (*.{";
      pattern += m_FileFormatPattern[i];
      pattern += "})";
      }
    }

  // Finish the all image pattern
  allImageFiles += "})\t";

  // Compete the pattern
  pattern = allImageFiles + pattern;
  
  return pattern;
}


template <class TPixel>
typename ImageIOWizardLogic<TPixel>::FileFormat
ImageIOWizardLogic<TPixel>
::DetermineFileFormatFromFileName(bool forLoading, const char *testFile) 
{
  // Iterate over the known file types
  for(unsigned int i = 0;i < FORMAT_COUNT;i++)
    {
    // Check if the file format is supported
    if((forLoading && this->CanLoadFileFormat((FileFormat) i)) ||
       (!forLoading && this->CanSaveFileFormat((FileFormat) i)))
      {
      // Create a matching pattern
      StringType pattern = "*.{" + m_FileFormatPattern[i] + "}";

      // Check if the filename matches the pattern
      if(fl_filename_match(testFile,pattern.c_str()))
        return (FileFormat) i;
      }
    }

  // Failed: return illegal pattern
  return FORMAT_COUNT;
}

template <class TPixel>
bool ImageIOWizardLogic<TPixel>
::CanLoadFileFormat(FileFormat irisNotUsed(format)) const 
{ 
  return true; 
}

template <class TPixel>
bool ImageIOWizardLogic<TPixel>
::CanSaveFileFormat(FileFormat format) const 
{ 
  return (
    format != FORMAT_DICOM &&
    format != FORMAT_GE4 &&
    format != FORMAT_GE5 &&
    format != FORMAT_SIEMENS);
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::OnFilePageBrowse() 
{
  // Get the path and pattern for reading in the file 
  StringType pattern = this->GetFilePattern(true);
  
  const char *path = m_InFilePageBrowser->value();
  path = strlen(path) ? path : NULL;

  // Configure a file dialog
  char *fName = fl_file_chooser("Load Image", pattern.c_str(), path);
  
  // Bring up th choice dialog
  if (fName)
    {
    // Set the new filename
    m_InFilePageBrowser->value(fName);

    // Reset the format drop-down box to a null value
    m_InFilePageFormat->value(0);

    // Run the filename change event
    OnFilePageFileInputChange();
    }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnFilePageFileFormatChange()
{
  // Activate the next button if there is a format selected
  if(m_InFilePageFormat->value() > 0)
    m_BtnFilePageNext->activate();
  else 
    m_BtnFilePageNext->deactivate();

  // If the user selects 'raw', update the format
  if (m_InFilePageFormat->value()-1 == FORMAT_RAW)
    m_PageHeader->activate();
  else
    m_PageHeader->deactivate();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnFilePageFileHistoryChange()
{
  // Copy the history to the file box
  m_InFilePageBrowser->value(
    m_InFilePageHistory->mvalue()->label());

  // Update everything
  OnFilePageFileInputChange();
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::OnFilePageFileInputChange() 
{
  // Check the length of the input
  const char *text = m_InFilePageBrowser->value();
  if (text != NULL && strlen(text) > 0)
    {
    // Try to select a file format accoring to the file name
    FileFormat fmt = DetermineFileFormatFromFileName(true,text);
    
    // If the filename does not match any format, we do not change the 
    // format choice box in case that the user has already set it manually
    if(fmt < FORMAT_COUNT)
      m_InFilePageFormat->value((int)fmt+1);
    
    // Run the format change event
    OnFilePageFileFormatChange();
    } 
  else
    {
    m_BtnFilePageNext->deactivate();
    }            
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::OnFilePageNext() 
{
  // Check if a file has been specified
  const char *fname = m_InFilePageBrowser->value();
  assert(fname && strlen(fname) > 0);

  // Check that a format has been specified
  assert(m_InFilePageFormat->value() > 0);

  // Check if the file format is raw
  if(m_InFilePageFormat->value() - 1 == (int) FORMAT_RAW)
    {
    GoForward();
    }
  else
    {
    // Create an Image IO for loading
    CreateImageIO((FileFormat)(m_InFilePageFormat->value()-1),true);

    // Try Loading image
    if(LoadImage(m_ImageIO))
      {
      GoForward();
      }      
    }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnCancel() 
{
  // Clear the data stored in the image
  // m_Image->Reset();
  m_Image = NULL;

  // Set the status to negative: nothing was loaded
  m_ImageLoaded = false;

  // Hide this window
  m_WinInput->hide();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnHeaderPageNext() 
{
  // Make sure we're not here by mistake
  assert(m_InFilePageFormat->value() == 1 + (int)FORMAT_RAW);

  // Create an ImageIO for loading the file
  CreateImageIO(FORMAT_RAW,true);

  // Better have something!
  assert(m_ImageIO);

  // Set the dimensions of the image
  m_ImageIO->SetDimensions(0,(unsigned int)m_InHeaderPageDimX->value());
  m_ImageIO->SetDimensions(1,(unsigned int)m_InHeaderPageDimY->value());
  m_ImageIO->SetDimensions(2,(unsigned int)m_InHeaderPageDimZ->value());

  // Set the spacing of the image
  m_ImageIO->SetSpacing(0,m_InHeaderPageSpacingX->value());
  m_ImageIO->SetSpacing(1,m_InHeaderPageSpacingY->value());
  m_ImageIO->SetSpacing(2,m_InHeaderPageSpacingZ->value());
  
  // Set the endianness
  if(m_InHeaderPageByteAlign->value() == 0)
    m_ImageIO->SetByteOrderToBigEndian();
  else
    m_ImageIO->SetByteOrderToLittleEndian();

  // Do the loading
  if(LoadImage(m_ImageIO))
    GoForward();
}

template <class TPixel>
bool 
ImageIOWizardLogic<TPixel>
::LoadImage(ImageIOType *customIO)
{
  bool rc;

  // Show a wait cursor
  fl_cursor(FL_CURSOR_WAIT,FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
  
  // Try to load a file
  try 
    {
    typedef ImageFileReader<ImageType> ReaderType;
    typedef typename ReaderType::Pointer ReaderPointer;

    // Create a reader
    ReaderPointer reader = ReaderType::New();

    // Set the file name
    reader->SetFileName(m_InFilePageBrowser->value());

    // Check if custom IO is required
    if(customIO)
      reader->SetImageIO(customIO);
        
    // Perform the read
    reader->Update();
    
    // Get the output
    m_Image = reader->GetOutput();

    // Disconnect the image from the reader
    m_Image->DisconnectPipeline();
     
    // Store the image IO
    m_ImageIO = reader->GetImageIO();

    // Check if the image is really valid
    if(rc = CheckImageValidity())
      {
      // Try to retrieve a registry accociated with the currently selected image
      m_Registry.Clear();
      if(m_Callback)
        m_Callback->FindRegistryAssociatedWithImage(
          m_InFilePageBrowser->value(), m_Registry);

      // Try to determine the RAI code
      GuessImageOrientation();
      }
    }
  catch(ExceptionObject &exc)
  {
    // Clear the image and image IO
    m_Image = NULL;
    m_ImageIO = NULL;

    // Show the error
    fl_alert("Error reading image: %s.",exc.GetDescription());
    rc = false;
  }

  // Fix the cursor
  fl_cursor(FL_CURSOR_DEFAULT,FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);

  // Check if the image is valid (subclasses can perform extra tasks here)
  return rc;
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnHeaderPageInputChange() 
{
  // Header input has changed
  if (m_InHeaderPageDimX->value() >= 1 &&
      m_InHeaderPageDimY->value() >= 1 &&
      m_InHeaderPageDimZ->value() >= 1 &&
      m_InHeaderPageSpacingX->value() > 0 &&
      m_InHeaderPageSpacingY->value() > 0 &&
      m_InHeaderPageSpacingZ->value() > 0)
    {
    m_BtnHeaderPageNext->activate();
    }
  else
    {
    m_BtnHeaderPageNext->deactivate();
    }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnHeaderPageBack() 
{
  GoBack();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnOrientationPageNext() 
{
  // Go on to the summary page
  GoForward();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnOrientationPageBack() 
{
  GoBack();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnFilePageEnter()
{

}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnHeaderPageEnter()
{

}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::GuessImageOrientation()
{
  // Flag as to whether we guessed an orientation or not
  bool flagGuessed = false;

  // First, check if the application has an idea of what the orientation should be
  // from earlier attempts to load the image. To do this, we make a callback to 
  // the parent application
  std::string sStoredOrientation = m_Registry["Orientation"][""];
  if(ImageCoordinateGeometry::IsRAICodeValid(sStoredOrientation.c_str()))
    {
    SetRAI(sStoredOrientation.c_str());
    flagGuessed = true;
    }

  // Otherwise, try to use the image's header to retrieve the RAI code
  else
    {
    // Get the meta data for the image
    MetaDataDictionary &mdd = m_Image->GetMetaDataDictionary();

    // Find the entry dealing with orientation
    typedef MetaDataObject<ValidCoordinateOrientationFlags> ObjectType;
    ObjectType *entry = 
      reinterpret_cast<ObjectType *> ( mdd[ITK_CoordinateOrientation].GetPointer() );

    // If the entry has a value, map it to RAI
    if(entry)
      {
      // This is a really dumb way to process the flag, but it's the only way that
      // is guaranteed to stay up to date with ITK's flags. Thanks, VIM, for macros! 
      ValidCoordinateOrientationFlags flag = entry->GetMetaDataObjectValue();
      switch(flag) 
        {
        case ITK_COORDINATE_ORIENTATION_RIP : SetRAI("RIP"); break;
        case ITK_COORDINATE_ORIENTATION_LIP : SetRAI("LIP"); break;
        case ITK_COORDINATE_ORIENTATION_RSP : SetRAI("RSP"); break;
        case ITK_COORDINATE_ORIENTATION_LSP : SetRAI("LSP"); break;
        case ITK_COORDINATE_ORIENTATION_RIA : SetRAI("RIA"); break;
        case ITK_COORDINATE_ORIENTATION_LIA : SetRAI("LIA"); break;
        case ITK_COORDINATE_ORIENTATION_RSA : SetRAI("RSA"); break;
        case ITK_COORDINATE_ORIENTATION_LSA : SetRAI("LSA"); break;
        case ITK_COORDINATE_ORIENTATION_IRP : SetRAI("IRP"); break;
        case ITK_COORDINATE_ORIENTATION_ILP : SetRAI("ILP"); break;
        case ITK_COORDINATE_ORIENTATION_SRP : SetRAI("SRP"); break;
        case ITK_COORDINATE_ORIENTATION_SLP : SetRAI("SLP"); break;
        case ITK_COORDINATE_ORIENTATION_IRA : SetRAI("IRA"); break;
        case ITK_COORDINATE_ORIENTATION_ILA : SetRAI("ILA"); break;
        case ITK_COORDINATE_ORIENTATION_SRA : SetRAI("SRA"); break;
        case ITK_COORDINATE_ORIENTATION_SLA : SetRAI("SLA"); break;
        case ITK_COORDINATE_ORIENTATION_RPI : SetRAI("RPI"); break;
        case ITK_COORDINATE_ORIENTATION_LPI : SetRAI("LPI"); break;
        case ITK_COORDINATE_ORIENTATION_RAI : SetRAI("RAI"); break;
        case ITK_COORDINATE_ORIENTATION_LAI : SetRAI("LAI"); break;
        case ITK_COORDINATE_ORIENTATION_RPS : SetRAI("RPS"); break;
        case ITK_COORDINATE_ORIENTATION_LPS : SetRAI("LPS"); break;
        case ITK_COORDINATE_ORIENTATION_RAS : SetRAI("RAS"); break;
        case ITK_COORDINATE_ORIENTATION_LAS : SetRAI("LAS"); break;
        case ITK_COORDINATE_ORIENTATION_PRI : SetRAI("PRI"); break;
        case ITK_COORDINATE_ORIENTATION_PLI : SetRAI("PLI"); break;
        case ITK_COORDINATE_ORIENTATION_ARI : SetRAI("ARI"); break;
        case ITK_COORDINATE_ORIENTATION_ALI : SetRAI("ALI"); break;
        case ITK_COORDINATE_ORIENTATION_PRS : SetRAI("PRS"); break;
        case ITK_COORDINATE_ORIENTATION_PLS : SetRAI("PLS"); break;
        case ITK_COORDINATE_ORIENTATION_ARS : SetRAI("ARS"); break;
        case ITK_COORDINATE_ORIENTATION_ALS : SetRAI("ALS"); break;
        case ITK_COORDINATE_ORIENTATION_IPR : SetRAI("IPR"); break;
        case ITK_COORDINATE_ORIENTATION_SPR : SetRAI("SPR"); break;
        case ITK_COORDINATE_ORIENTATION_IAR : SetRAI("IAR"); break;
        case ITK_COORDINATE_ORIENTATION_SAR : SetRAI("SAR"); break;
        case ITK_COORDINATE_ORIENTATION_IPL : SetRAI("IPL"); break;
        case ITK_COORDINATE_ORIENTATION_SPL : SetRAI("SPL"); break;
        case ITK_COORDINATE_ORIENTATION_IAL : SetRAI("IAL"); break;
        case ITK_COORDINATE_ORIENTATION_SAL : SetRAI("SAL"); break;
        case ITK_COORDINATE_ORIENTATION_PIR : SetRAI("PIR"); break;
        case ITK_COORDINATE_ORIENTATION_PSR : SetRAI("PSR"); break;
        case ITK_COORDINATE_ORIENTATION_AIR : SetRAI("AIR"); break;
        case ITK_COORDINATE_ORIENTATION_ASR : SetRAI("ASR"); break;
        case ITK_COORDINATE_ORIENTATION_PIL : SetRAI("PIL"); break;
        case ITK_COORDINATE_ORIENTATION_PSL : SetRAI("PSL"); break;
        case ITK_COORDINATE_ORIENTATION_AIL : SetRAI("AIL"); break;
        case ITK_COORDINATE_ORIENTATION_ASL : SetRAI("ASL"); break;
        default: SetRAI("RAI"); break;
        }
      flagGuessed = true;
      }
    }

  if(flagGuessed)
    {
    // Set the preset to default
    m_InOrientationPagePreset->value(4);
    OnOrientationPageSelectPreset();
    }
  else
    {      
    // Set the preset to default
    m_InOrientationPagePreset->value(0);
    OnOrientationPageSelectPreset();
    }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnOrientationPageEnter()
{
  // Check if the currently selected RAI is valid
  if(ImageCoordinateGeometry::IsRAICodeValid(m_InRAICode->value()))
    {
    // Make sure that the old RAI gets integrated with the new image
    SetRAI(m_InRAICode->value());
    }
  else
    {
    // Enter the default mode
    m_InOrientationPagePreset->value(0);

    // Make sure the preset is applied
    OnOrientationPageSelectPreset();
    }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSummaryPageEnter()
{
  const char *boTypes[] = 
    {"Big Endian", "Little Endian","Order Not Applicable"};

  // Get the IO base object
  // itk::ImageIOBase *ioBase = m_Image->GetImageIO();

  // The object better not be NULL!
  if(m_ImageIO && m_Image)
    {
    // A stream to simplify converting to string
    IRISOStringStream sout;    

    // Print file name
    m_OutSummaryFileName->value(m_ImageIO->GetFileName());

    // Print file dimensions
    sout << m_Image->GetBufferedRegion().GetSize();
    m_OutSummaryDimensions->value(sout.str().c_str());

    // Print file size in bytes
    m_OutSummarySize->value((int)(m_ImageIO->GetImageSizeInBytes() / (1024.0)));
    
    // Print pixel spacing 
    sout.str(""); sout << m_Image->GetSpacing();
    m_OutSummarySpacing->value(sout.str().c_str());

    // Print the image origin
    sout.str(""); sout << m_Image->GetOrigin();
    m_OutSummaryOrigin->value(sout.str().c_str());
    
    // TODO: This is a workaround on an itk bug with RawImageIO
    if(m_ImageIO->GetComponentType() != ImageIOBase::UNKNOWNCOMPONENTTYPE)
      {
      // There actually is a type in the IO object
      m_OutSummaryPixelType->value(
        m_ImageIO->GetComponentTypeAsString(m_ImageIO->GetComponentType()).c_str());
      }
    else
      {
      m_OutSummaryPixelType->value(m_InHeaderPageVoxelType->text());
      }
    
    // Print the byte order
    m_OutSummaryByteOrder->value(
       boTypes[(unsigned int)(m_ImageIO->GetByteOrder() - ImageIOType::BigEndian)]);

    // Dump the contents of the meta data dictionary
    m_SummaryTextBuffer->text("");
    MetaDataDictionary &mdd = m_ImageIO->GetMetaDataDictionary();
    MetaDataDictionary::ConstIterator itMeta = mdd.Begin();
    while(itMeta != mdd.End())
      {
      // Get the metadata as a generic object
      std::string key = itMeta->first;
      itk::MetaDataObjectBase *meta = itMeta->second;

      // Check if the meta data string is a 
      if( typeid(std::string) == meta->GetMetaDataObjectTypeInfo() )
        {
        // Cast the value to a string and print it
        typedef MetaDataObject<std::string> ObjectType;
        std::string value = ((ObjectType *)(meta))->GetMetaDataObjectValue();

        // For some weird reason, some of the strings returned by this method 
        // contain '\0' characters. We will replace them by spaces
        sout.str("");
        for(unsigned int i=0;i<value.length();i++)
          if(value[i] >= ' ') sout << value[i];
        value = sout.str();

        // Make sure the value has more than blanks
        if(value.find_first_not_of(" ") != value.npos)
          {
          m_SummaryTextBuffer->append(key.c_str());
          m_SummaryTextBuffer->append(" = ");
          m_SummaryTextBuffer->append(value.c_str());
          m_SummaryTextBuffer->append("\n");
          }
        }
      ++itMeta;
      }
    }
  else 
    {
    m_OutSummaryFileName->value("Error loading image.");
    m_OutSummaryDimensions->value("n/a");
    m_OutSummarySize->value(0);
    m_OutSummarySpacing->value("n/a");
    m_OutSummaryOrigin->value("n/a");
    m_OutSummaryPixelType->value("n/a");
    m_OutSummaryByteOrder->value("n/a");
    m_SummaryTextBuffer->text("");
    }
}


template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::SetRAIToInvalid(const char *rai) 
{
  // Invalidate the page
  m_BtnOrientationPageNext->deactivate();
  m_WizOrientationPageDoll->value(m_GrpOrientationPageDollInvalid);
  
  // Pass the invalid RAI to the RAI editor (needed when this method is called
  // in response to the editor
  m_InRAICode->value(rai);

  // Hide all the corner widgets
  for(unsigned int j=0;j<8;j++)
    {
    m_OutOrientationPageCorner[j]->hide();
    m_OutOrientationPageCorner[j]->value("");
    }
}


template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::SetRAI(const char *rai) 
{
  // Make sure we have a valid RAI - caller should check first
  assert(rai && ImageCoordinateGeometry::IsRAICodeValid(rai));

  // Convert RAI to a numeric mapping
  Vector3i map = 
    ImageCoordinateGeometry::ConvertRAIToCoordinateMapping(rai);

  // Get the coordinate indices and directions
  unsigned int cidx[3];
  unsigned int cflip[3];

  for(unsigned int i=0;i<3;i++) 
    {
    cidx[i] = abs(map[i]) - 1;
    cflip[i] = map[i] < 0 ? 1 : 0;
    }
  
  // Apply the RAI to the orientation page drop down boxes
  m_InOrientationPageX->value(
    m_MapOrientationIndexAndFlipToMenuItem[cidx[0]][cflip[0]]);
  m_InOrientationPageY->value(
    m_MapOrientationIndexAndFlipToMenuItem[cidx[1]][cflip[1]]);
  m_InOrientationPageZ->value(
    m_MapOrientationIndexAndFlipToMenuItem[cidx[2]][cflip[2]]);

  // Use the orientation transform to choose the correct doll page
  m_WizOrientationPageDoll->value(
    m_GrpOrientationPageDoll[
      m_MapOrientationIndexToDollPageIndex[cidx[0]][cidx[1]][cidx[2]]]);

  // Find the origin code
  unsigned int origin = 
    m_OrientationIndexToDollVertex[cidx[0]][cidx[1]][cidx[2]];

  // Flip the origin if necessary
  origin ^= (4 * cflip[0] + 2 * cflip[1] + 1 * cflip[2]);

  // Compute the anti-origin
  unsigned int anti = 7 ^ origin;

  // Set the state of the corner display boxes
  for(unsigned int j=0;j<8;j++)
    {
    if(j == origin)
      {
      // Origin is visible
      m_OutOrientationPageCorner[j]->show();

      // Origin gets the zero index
      m_OutOrientationPageCorner[j]->value("0,0,0");
      }
    else if(j == anti)
      {
      // Anti-origin is visible
      m_OutOrientationPageCorner[j]->show();

      // Anti-origin gets the image dimensions
      typename ImageType::SizeType size = 
        m_Image->GetLargestPossibleRegion().GetSize();

      // Print the size out
      char str[256];
      sprintf(str,"%ld,%ld,%ld",size[0],size[1],size[2]);
      
      // Origin gets the zero index
      m_OutOrientationPageCorner[j]->value(str);
      }
    else
      {
      // Everything else is hidder
      m_OutOrientationPageCorner[j]->value("");
      m_OutOrientationPageCorner[j]->hide();
      }
    }
  
  // Plug the RAI code into the code editor
  m_InRAICode->value(rai);
  
  // Set the state to valid, allowing user to continue to next page
  m_BtnOrientationPageNext->activate();
};

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnOrientationPageSelectPreset() 
{

  // Whether or not custom is allowed
  int useCustom = 0;

  switch (m_InOrientationPagePreset->value()) {
  case 0 : 
    SetRAI("RAI");break;
  case 1 :
    SetRAI("ASR");break;
  case 2 : 
    SetRAI("RSP");break;
  case 3:
    SetRAI("RAI");break;
  case 4:
    useCustom = 1;
    break;
  default:
    assert(0 == "Unknown Preset");
  }

  // Enable or disable the pane
  if (useCustom)
    m_GrpOrientationPageCustom->activate();
  else
    m_GrpOrientationPageCustom->deactivate();
}

template <class TPixel>
void ImageIOWizardLogic<TPixel>
::OnOrientationPageSelect() 
{
  // Return the currently set RAI
  static char *raiList = "RLAPIS";

  // Compute the RAI code based on the state of the drop-downs
  char rai[4];
  rai[0] = raiList[m_InOrientationPageX->value()];
  rai[1] = raiList[m_InOrientationPageY->value()];
  rai[2] = raiList[m_InOrientationPageZ->value()];
  rai[3] = '\0';
  
  // Check if valid
  if (ImageCoordinateGeometry::IsRAICodeValid(rai))
    {
    // Run the standard SetRAI procedure (update all related controls)
    SetRAI(rai);
    }    
  else
    {
    SetRAIToInvalid("");
    }    
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSummaryPageFinish() 
{
  // Perform a final validity / sanity check
  if (CheckFinalValidity()) 
  {
    // Set the status to positive, the image has been loaded!
    m_ImageLoaded = true;

    // Hide this window
    m_WinInput->hide();
  }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnOrientationPageRAIChange()
{
  // The user has manually edited the RAI code
  const char *rai = m_InRAICode->value();

  // If the rai code is not 3 characters long, return (don't react to edits)
  if(strlen(rai) != 3)
    return;

  // Set the preset mode to custom
  m_InOrientationPagePreset->value(4);
  m_GrpOrientationPageCustom->activate();

  // Check that the code is valid
  if(ImageCoordinateGeometry::IsRAICodeValid(rai))
    {
    SetRAI(rai);
    }
  else
    {
    SetRAIToInvalid(rai);
    }
}



template <class TPixel>
ImageIOWizardLogic<TPixel>::~ImageIOWizardLogic() 
{
  delete m_SummaryTextBuffer;
}

template <class TPixel>
bool 
ImageIOWizardLogic<TPixel>
::DisplayInputWizard(const char *file) 
{
  // The loaded flag is false
  m_ImageLoaded = false;

  // Point the wizard to the first page
  m_WizInput->value(m_PageFile);

  // Clear the file name box
  // TODO: Implement a history list
  if(file)
    m_InFilePageBrowser->value(file);
  
  OnFilePageFileInputChange();

  // Show the input window
  m_WinInput->show();

  // Loop until the window has been closed
  while (m_WinInput->visible())
    Fl::wait();

  // Whether or not the load has been succesfull
  return m_ImageLoaded;
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSummaryPageBack() 
{
  GoBack();
}

template <class TPixel>
bool 
ImageIOWizardLogic<TPixel>
::DisplaySaveWizard(ImageType *image,const char *file)
{
  // Clear the saved flag
  m_ImageSaved = false;

  // Store the image
  m_Image = image;

  // Clear the file name box
  // TODO: Implement a history list
  if(file)
    m_InFilePageBrowser->value(file);
  OnSaveFilePageFileInputChange();

  // Show the input window
  m_WinOutput->show();

  // Loop until the window has been closed
  while (m_WinOutput->visible())
    Fl::wait();

  // Clear the image pointer
  m_Image = NULL;

  // Whether or not the load has been succesfull
  return m_ImageSaved;
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSaveFilePageFileInputChange()
{
  // Check the length of the input
  const char *text = m_InSaveFilePageBrowser->value();
  if (text != NULL && strlen(text) > 0)
    {
    // Try to select a file format accoring to the file name
    FileFormat fmt = DetermineFileFormatFromFileName(false,text);
    
    // If the filename does not match any format, we do not change the 
    // format choice box in case that the user has already set it manually
    if(fmt < FORMAT_COUNT)
      m_InSaveFilePageFormat->value((int)fmt+1);
    
    // Run the format change event
    OnSaveFilePageFileFormatChange();
    } 
  else
    {
    m_BtnSaveFilePageNext->deactivate();
    }            
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSaveFilePageFileHistoryChange()
{
  // Copy the history to the file box
  m_InSaveFilePageBrowser->value(
    m_InSaveFilePageHistory->mvalue()->label());

  // Update everything
  OnSaveFilePageFileInputChange();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSaveFilePageFileFormatChange()
{
  // Activate the next button if there is a format selected
  if(m_InSaveFilePageFormat->value() > 0)
    m_BtnSaveFilePageNext->activate();
  else 
    m_BtnSaveFilePageNext->deactivate();
}


template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSaveFilePageBrowse()
{  
  // Get the path and pattern for reading in the file 
  const char *path = m_InSaveFilePageBrowser->value();
  path = strlen(path) ? path : NULL;

  // Get a pattern
  StringType pattern = this->GetFilePattern(false);

  // Create a file chooser
  char *fName = fl_file_chooser("Save Image As", pattern.c_str(), path);
  if (fName)
    {
    // Set the new filename
    m_InSaveFilePageBrowser->value(fName);

    // Reset the format drop-down box to a null value
    m_InSaveFilePageFormat->value(0);

    // Run the filename change event
    OnSaveFilePageFileInputChange();
    }
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::CreateImageIO(FileFormat fmt,bool forLoading)
{
  switch(fmt)
    {
    case FORMAT_MHA:
      m_ImageIO = MetaImageIO::New();
      break;
    case FORMAT_ANALYZE:
      m_ImageIO = AnalyzeImageIO::New();
      break;
    case FORMAT_GIPL:
      m_ImageIO = GiplImageIO::New();
      break;
    case FORMAT_DICOM:
      m_ImageIO = DicomImageIO::New();
      break;
    case FORMAT_GE4:
      m_ImageIO = GE4ImageIO::New();
      break;
    case FORMAT_GE5:
      m_ImageIO = GE5ImageIO::New();
      break;
    case FORMAT_SIEMENS:
      m_ImageIO = SiemensVisionImageIO::New();
      break;
    case FORMAT_VTK:
      m_ImageIO = VTKImageIO::New();
      break;
    case FORMAT_RAW:
      if(forLoading)
        {
        // Use header page values to initialize the RAW io
        unsigned int header = (unsigned int)m_InHeaderPageHeaderSize->value(); 
        switch((int)m_InHeaderPageVoxelType->value())
          {
          case 0: m_ImageIO = CreateRawImageIO((unsigned char) 0, header); break;
          case 1: m_ImageIO = CreateRawImageIO((char) 0, header); break;
          case 2: m_ImageIO = CreateRawImageIO((unsigned short) 0, header); break;
          case 3: m_ImageIO = CreateRawImageIO((short) 0, header); break;
          case 4: m_ImageIO = CreateRawImageIO((unsigned int) 0,header); break;
          case 5: m_ImageIO = CreateRawImageIO((int) 0, header); break;
          case 6: m_ImageIO = CreateRawImageIO(0.0f, header); break;
          default: assert(0);
          }
        }
      else
        {
        // Use current pixel type and zero size header
        m_ImageIO = CreateRawImageIO((TPixel)0,0);
        }
      break;
    default:
      assert(0);
    }
}


template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSaveFilePageSave()
{
  // There better be a format selected
  assert(m_InSaveFilePageFormat->value() > 0);
  
  // Create an appropriate image IO based on the selected format
  CreateImageIO((FileFormat)(m_InSaveFilePageFormat->value()-1),false);

  // Try to save the image using the current format
  typedef ImageFileWriter<ImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(m_InSaveFilePageBrowser->value());
  writer->SetImageIO(m_ImageIO);
  writer->SetInput(m_Image);

  fl_cursor(FL_CURSOR_WAIT,FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
  
  try 
    {
    writer->Update();
    m_ImageSaved = true;
    m_WinOutput->hide();
    }
  catch(ExceptionObject &exc)
    {
    fl_alert("Error saving file: %s",exc.GetDescription());
    }
  
  fl_cursor(FL_CURSOR_DEFAULT,FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR);
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::OnSaveCancel()
{
  // Just close the window
  m_WinOutput->hide();
}

template <class TPixel>
void 
ImageIOWizardLogic<TPixel>
::SetHistory(const HistoryType &history)
{
  // Store the history
  m_History = history;

  // Clear the history drop box
  m_InFilePageHistory->clear();
  m_InSaveFilePageHistory->clear();

  // Add the history
  if(history.size() > 0)
    {  
    // Add each item to the history menu (history is traversed
    // backwards)
    for(HistoryType::reverse_iterator it=m_History.rbegin();
        it!=m_History.rend();it++)
      {
      // FLTK's add() treats slashes as submenu separators, hence this code
      m_InFilePageHistory->replace(
        m_InFilePageHistory->add("dummy"),it->c_str());      
      m_InSaveFilePageHistory->replace(
        m_InSaveFilePageHistory->add("dummy"),it->c_str());
      }

    // Activate the history menu    
    m_InFilePageHistory->activate();
    m_InSaveFilePageHistory->activate();
    }
  else
    {
    // Deactivate history
    m_InFilePageHistory->deactivate();
    m_InSaveFilePageHistory->deactivate();
    }
}


#endif // __ImageIOWizardLogic_txx_
