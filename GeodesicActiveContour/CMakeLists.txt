PROJECT( GeodesicActiveContour )

INCLUDE_DIRECTORIES(
${ITK_SOURCE_DIR}/Auxiliary/FltkImageViewer
${ITK_SOURCE_DIR}/Auxiliary/VtkFltk
${ITK_SOURCE_DIR}/Auxiliary/vtk
${ITK_BINARY_DIR}/Auxiliary/FltkImageViewer
${ITK_BINARY_DIR}/Auxiliary/VtkFltk
${GeodesicActiveContour_SOURCE_DIR}
${GeodesicActiveContour_BINARY_DIR}
${FLTK_INCLUDE_PATH}
${OPENGL_INCLUDE_PATH}
)


SET(GeodesicActiveContour_SRCS
GeodesicActiveContourBase.cxx
GeodesicActiveContour.cxx
)

SET(GeodesicActiveContour_GUI_SRCS
GeodesicActiveContourGUI.fl
)

LINK_LIBRARIES (
ITKCommon
ITKNumerics
ITKBasicFilters 
ITKIO
vtkRendering
vtkGraphics	  
vtkHybrid	  
vtkImaging
vtkIO
vtkFiltering
vtkCommon
ITKFltkImageViewer
ITKVtkFltk
${FLTK_LIBRARY}
${OPENGL_LIBRARY}
${GLU_LIBRARY}
)

IF(WIN32)
  LINK_LIBRARIES(
  wsock32
  )
ENDIF(WIN32)

IF(UNIX)
  LINK_LIBRARIES (
  ${CMAKE_X_LIBS}
  )
ENDIF(UNIX)

ADD_EXECUTABLE(GeodesicActiveContour ${GeodesicActiveContour_SRCS})
FLTK_WRAP_UI( GeodesicActiveContour ${GeodesicActiveContour_GUI_SRCS} )
