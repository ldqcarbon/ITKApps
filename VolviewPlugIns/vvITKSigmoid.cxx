/* perform a pixel-wise intensity transformation using a Sigmoid function */

#include "vvITKFilterModule.h"

#include "itkSigmoidImageFilter.h"

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{

  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  const unsigned int Dimension = 3;

  try 
  {
  switch( info->InputVolumeScalarType )
    {
    case VTK_UNSIGNED_CHAR:
      {
      typedef  unsigned char                        PixelType;
      typedef  itk::Image< PixelType, Dimension >   ImageType; 
      typedef  itk::SigmoidImageFilter< ImageType,  ImageType >   FilterType;
      VolView::PlugIn::FilterModule< FilterType > module;
      module.SetPlugInfo( info );
      module.SetUpdateMessage("Transforming intensities with a Sigmoid function...");
      // Set the parameters on it
      module.GetFilter()->SetAlpha(          atof( info->GetGUIProperty(info, 0, VVP_GUI_VALUE )) );
      module.GetFilter()->SetBeta(           atof( info->GetGUIProperty(info, 1, VVP_GUI_VALUE )) );
      module.GetFilter()->SetOutputMinimum(  atoi( info->GetGUIProperty(info, 2, VVP_GUI_VALUE )) );
      module.GetFilter()->SetOutputMaximum(  atoi( info->GetGUIProperty(info, 3, VVP_GUI_VALUE )) );
      // Execute the filter
      module.ProcessData( pds  );
      break; 
      }
    case VTK_UNSIGNED_SHORT:
      {
      typedef  unsigned short                       PixelType;
      typedef  itk::Image< PixelType, Dimension >   ImageType; 
      typedef  itk::SigmoidImageFilter< ImageType,  ImageType >   FilterType;
      VolView::PlugIn::FilterModule< FilterType > module;
      module.SetPlugInfo( info );
      module.SetUpdateMessage("Transforming intensities with a Sigmoid function...");
      // Set the parameters on it
      module.GetFilter()->SetAlpha(          atof( info->GetGUIProperty(info, 0, VVP_GUI_VALUE )) );
      module.GetFilter()->SetBeta(           atof( info->GetGUIProperty(info, 1, VVP_GUI_VALUE )) );
      module.GetFilter()->SetOutputMinimum(  atoi( info->GetGUIProperty(info, 2, VVP_GUI_VALUE )) );
      module.GetFilter()->SetOutputMaximum(  atoi( info->GetGUIProperty(info, 3, VVP_GUI_VALUE )) );
      // Execute the filter
      module.ProcessData( pds );
      break; 
      }
    }
  }
  catch( itk::ExceptionObject & except )
  {
    info->SetProperty( info, VVP_ERROR, except.what() ); 
    return -1;
  }
  return 0;
}


static int UpdateGUI(void *inf)
{
  char tmp[1024];
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "Alpha");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_SCALE);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT, "5");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP, "Factor that defines the width of the Sigmoid in the range scale. Setting a small alpha results in a step transion on the Sigmoid function. A large alpha value produces a very smooth and low slanted Sigmoid.");
  info->SetGUIProperty(info, 0, VVP_GUI_HINTS , "-100 100 0.1");

  info->SetGUIProperty(info, 1, VVP_GUI_LABEL, "Beta");
  info->SetGUIProperty(info, 1, VVP_GUI_TYPE, VVP_GUI_SCALE);
  const float meanValue = (info->InputVolumeScalarRange[1] - 
                           info->InputVolumeScalarRange[0]) / 2.0;
  sprintf(tmp,"%f",meanValue);
  info->SetGUIProperty(info, 1, VVP_GUI_DEFAULT, tmp );
  info->SetGUIProperty(info, 1, VVP_GUI_HELP, "Origin of the Sigmoid function in the range scale. It cooresponds to the intensity of the imput image that will be mapped almost linearly to the output image. Intensities far from this value will be transformed non-linearly.");
  sprintf(tmp,"%f %f %f",info->InputVolumeScalarTypeRange[0], 1.0,
                         info->InputVolumeScalarTypeRange[1] );
  info->SetGUIProperty(info, 1, VVP_GUI_HINTS , tmp );

  info->GUIItems[2].Label = "Output Minimum";
  info->GUIItems[2].GUIType = VV_GUI_SCALE;
  info->GUIItems[2].Default = "0";
  info->GUIItems[2].Help = "Desired value for the minimum intensity of the output image.";
  info->GUIItems[2].Hints = "0 255 1";

  info->GUIItems[3].Label = "Output Maximum";
  info->GUIItems[3].GUIType = VV_GUI_SCALE;
  info->GUIItems[3].Default = "255";
  info->GUIItems[3].Help = "Desired value for the maximum intensity of the output image.";
  info->GUIItems[3].Hints = "0 255 1";

  info->RequiredZOverlap = 0;
  
  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  info->OutputVolumeNumberOfComponents = 
    info->InputVolumeNumberOfComponents;
  memcpy(info->OutputVolumeDimensions,info->InputVolumeDimensions,
         3*sizeof(int));
  memcpy(info->OutputVolumeSpacing,info->InputVolumeSpacing,
         3*sizeof(float));
  memcpy(info->OutputVolumeOrigin,info->InputVolumeOrigin,
         3*sizeof(float));

  return 1;
}


extern "C" {
  
void VV_PLUGIN_EXPORT vvITKSigmoidInit(vtkVVPluginInfo *info)
{
  // setup information that never changes
  info->ProcessData = ProcessData;
  info->UpdateGUI   = UpdateGUI;
  info->Name = "Sigmoid (ITK)";
  info->TerseDocumentation = "Simoid Intensity Transform";
  info->FullDocumentation = 
    "This filters applies a pixel-wise intensity transform by using a Sigmoid function";
  info->SupportsInPlaceProcessing = 0;
  info->SupportsProcessingPieces = 1;
  info->RequiredZOverlap = 0;

  // Number of bytes required in intermediate memory per voxel
  info->PerVoxelMemoryRequired = 1; // actually depends on the input pixel size
  
  /* setup the GUI components */
  info->NumberOfGUIItems = 4;
  info->GUIItems = (vtkVVGUIItem *)malloc(info->NumberOfGUIItems*sizeof(vtkVVGUIItem));
}

}
