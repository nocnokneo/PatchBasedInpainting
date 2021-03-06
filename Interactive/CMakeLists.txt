# If we did not build the PatchBasedInpainting library with Qt, then find Qt now,
# it is definitely required for the interactive executable.
if(NOT QT4_FOUND)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})
endif(NOT QT4_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(Interactive_includes ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

QT4_WRAP_UI(InpaintingGUI_UISrcs
            BasicViewerWidget.ui
            ManualPatchSelectionDialog.ui
            TopPatchesWidget.ui
            TopPatchesDialog.ui)
QT4_WRAP_CPP(InpaintingGUI_MOCSrcs
            BasicViewerWidget.h
            #NearestNeighbor/VisualSelectionBest.hpp
            MovablePatch.h
            TopPatchesWidget.h
            TopPatchesDialog.h
            Delegates/PixmapDelegate.h
            ManualPatchSelectionDialog.h)

add_library(InpaintingGUI Layer.cpp InteractorStyleImageWithDrag.cpp
            ImageCamera.cpp MovablePatch.cpp PatchHighlighter.cpp
            Delegates/PixmapDelegate.cpp ${InpaintingGUI_UISrcs} ${InpaintingGUI_MOCSrcs})
target_link_libraries(InpaintingGUI ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} Helpers VTKHelpers QtHelpers)

# if(BuildInteractive)
#   message("Interactive/Building interactive.")
#   add_subdirectory(Widgets)
#   add_subdirectory(ModelView)
# 
#   QT4_WRAP_UI(UISrcs PatchBasedInpainting.ui FileSelector.ui)
#   QT4_WRAP_CPP(MOCSrcs PatchBasedInpaintingGUI.h InpaintingComputationObject.h FileSelector.h PixmapDelegate.h)
# 
#   add_executable(PatchBasedInpaintingInteractive
#   ColorPalette.cpp
#   DisplayState.cpp
#   FileSelector.cpp
#   ImageCamera.cpp
#   ImageInput.cpp
#   InpaintingComputationObject.cpp
#   InpaintingIterationRecord.cpp
#   InpaintingIterationRecordViewer.cpp
#   InteractorStyleImageWithDrag.cpp
#   Layer.cpp
#   MovablePatch.cpp
#   PatchBasedInpaintingGUI.cpp
#   PatchBasedInpaintingGUISlots.cpp
#   PatchBasedInpaintingInteractive.cpp #driver (contains main())
#   PixmapDelegate.cpp
#   InpaintingDisplaySettings.cpp
#   VectorLayer.cpp
#   ${UISrcs} ${MOCSrcs})
# 
#   target_link_libraries(PatchBasedInpaintingInteractive PatchBasedInpainting libWidgets libModelView libHelpers ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
#   INSTALL( TARGETS PatchBasedInpaintingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
# endif(BuildInteractive)

# Inpainting viewer
# if(BuildViewer)
#   message("Interactive/Building the viewer.")
#   QT4_WRAP_UI(UISrcs FullInspectionWidget.ui)
#   QT4_WRAP_CPP(MOCSrcs FullInspectionWidget.h)
# 
#   add_executable(PatchBasedInpaintingViewer
#   ColorPalette.cpp
#   HelpersQt.cpp
#   ImageCamera.cpp
#   InteractorStyleImageWithDrag.cpp
#   Layer.cpp
#   FullInspectionWidget.cpp
#   PatchBasedInpaintingViewer.cpp #driver (contains main())
#   InpaintingDisplaySettings.cpp
# 
#   ../ImageProcessing/Mask.cpp
# 
#   ${UISrcs} ${MOCSrcs})
# 
#   target_link_libraries(PatchBasedInpaintingViewer libHelpers ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
#   INSTALL( TARGETS PatchBasedInpaintingViewer RUNTIME DESTINATION ${INSTALL_DIR} )
# endif(BuildViewer)

# if(BuildTests)
#   add_subdirectory(Tests)
# endif(BuildTests)
