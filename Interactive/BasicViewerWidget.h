/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef BasicViewerWidget_H
#define BasicViewerWidget_H

#include "ui_BasicViewerWidget.h"

// VTK
#include <vtkSmartPointer.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QThread>

// Custom
#include "ImageCamera.h"
#include "Mask/Mask.h"
#include "Node.h"
#include "Interactive/Layer.h"
#include "Interactive/PatchHighlighter.h"

class InteractorStyleImageWithDrag;

class BasicViewerWidgetParent : public QMainWindow, public Ui::BasicViewerWidget
{
Q_OBJECT

public slots:

  virtual void slot_UpdateImage() = 0;
  virtual void slot_UpdateSource(const itk::ImageRegion<2>& region, const itk::ImageRegion<2>& targetregion) = 0;
  virtual void slot_UpdateTarget(const itk::ImageRegion<2>& region) = 0;
  virtual void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion) = 0;

};

template <typename TImage>
class BasicViewerWidget : public BasicViewerWidgetParent
{
private:
  /** The image that will be displayed, and the from which the patches will be extracted before being displayed. */
  TImage* Image;

  /** The mask that will be used to mask the patches that are displayed. */
  Mask* MaskImage;

  /** This variable is used to track whether or not the image size changed between this refresh and the last refresh.
   * Typically it is simply used to determine if ResetCamera should be called before rendering. We typically do not
   * want to call ResetCamera if only the image content has been changed, but we do want to call it if the image
   * size has changed (typically this only when the image is changed, or setup for the first time). */
  int ImageDimension[3];

  /** A wrapper that creates and holds the image, the mapper, and the actor. */
  Layer ImageLayer;

public:
  // Constructor
  BasicViewerWidget(TImage* const image, Mask* const mask);

  void slot_UpdateImage();

  // We need the target region as well while updating the source region because we may want to mask the source patch with the target patch's mask.
  void slot_UpdateSource(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

  void slot_UpdateTarget(const itk::ImageRegion<2>& region);
  void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);
  
private:

  void SetupScenes();

  // The interactor to allow us to zoom and pan the image while still moving images with Pickable=true
  vtkSmartPointer<InteractorStyleImageWithDrag> InteractorStyle;

  // The only renderer
  vtkSmartPointer<vtkRenderer> Renderer;

  QGraphicsScene* SourcePatchScene;
  QGraphicsScene* TargetPatchScene;
  QGraphicsScene* ResultPatchScene;
  QGraphicsScene* MaskedSourcePatchScene;
  QGraphicsScene* MaskedTargetPatchScene;

  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  // Connect all signals and slots.
  void SetupConnections();

  ImageCamera* Camera;

  PatchHighlighter* SourceHighlighter;
  PatchHighlighter* TargetHighlighter;
};

#include "BasicViewerWidget.hpp"

#endif // BasicViewerWidget_H
