
SET ( ImageColorViewer_SRCS
  ImageColorViewer.cxx
  )

INCLUDE_DIRECTORIES (
  ${InsightApplications_SOURCE_DIR}/Auxiliary/FltkImageViewer
)


IF(WIN32)
  LINK_LIBRARIES(
    wsock32
  )
ENDIF(WIN32)

ADD_GUI_EXECUTABLE( ImageColorViewer "${ImageColorViewer_SRCS}" )
TARGET_LINK_LIBRARIES (ImageColorViewer
  ITKBasicFilters
  ITKFltkImageViewer
  ITKIO
)

SET(ImageColorViewer_GUI_SRCS
  ImageColorViewerGUI.fld
  )

FLTK_WRAP_UI( ImageColorViewer ${ImageColorViewer_GUI_SRCS} )