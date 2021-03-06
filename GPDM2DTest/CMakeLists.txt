PROJECT(GPDM2DTest)


IF( BUILD_OUTSIDE_INSIGHT_APPLICATIONS )

  FIND_PACKAGE(ITK)
  IF(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
  ELSE(ITK_FOUND)
    MESSAGE(FATAL_ERROR
            "Cannot build ITKApps without ITK.  Please set ITK_DIR.")
  ENDIF(ITK_FOUND)

  FIND_PACKAGE(FLTK)
  IF(FLTK_FOUND)
    INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
  ENDIF(FLTK_FOUND)

ENDIF( BUILD_OUTSIDE_INSIGHT_APPLICATIONS )


INCLUDE_DIRECTORIES(
${ITK_SOURCE_DIR}/Auxiliary/FltkImageViewer
${ITK_SOURCE_DIR}/Example/GPDM3DTest
${ITK_BINARY_DIR}/Auxiliary/FltkImageViewer
${FLTK_INCLUDE_PATH}
${OPENGL_INCLUDE_PATH}
${CMAKE_HOME_DIRECTORY}
)

LINK_DIRECTORIES(
${ITK_BINARY_DIR}/Auxiliary/FltkImageViewer
${ITK_BINARY_DIR}/Utilities/MetaIO
${ITK_BINARY_DIR}/Utilities/png
${ITK_BINARY_DIR}/Utilities/zlib
${FLTK_LIB_PATH}
)

LINK_LIBRARIES (
  ${ITK_LIBRARIES}
  FltkImageViewer
  ${FLTK_LIBRARY}
  ${OPENGL_LIBRARY}
  ${OPENGL_glu_LIBRARY}
)


SET(GPDM2DTest_SRCS
GPDM2DTest.cpp
MySegmentation.cxx
)

SET(GPDM2DTest_FLTK_SRCS
MyTestAppUI.fl
)

ADD_EXECUTABLE(GPDM2DTest   ${GPDM2DTest_SRCS})
INSTALL_TARGETS(/bin GPDM2DTest)

IF(HAS_FLTK)
  FLTK_WRAP_UI(GPDM2DTest ${GPDM2DTest_FLTK_SRCS})
  ITK_DISABLE_FLTK_GENERATED_WARNINGS("${GPDM2DTest_FLTK_SRCS}")
ENDIF(HAS_FLTK)

