#include "PriorityViewerWidget.h" // Appease syntax parser

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "Helpers/VTKHelpers.h"
#include "Interactive/HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"

PriorityViewerWidget::PriorityViewerWidget(Priority* const priorityFunction, const itk::Size<2>& imageSize) :
PriorityFunction(priorityFunction), ImageSize(imageSize), PreviouslyDisplayed(false)
{
  this->setupUi(this);

  PriorityImage = PriorityImageType::New();
  itk::ImageRegion<2> fullRegion(ITKHelpers::CornerRegion(ImageSize));
  PriorityImage->SetRegions(fullRegion);
  PriorityImage->Allocate();

//   int dims[3] = {imageSize[0], imageSize[1], 1};
//   this->ImageLayer.ImageData->SetDimensions(dims);
//   this->ImageLayer.ImageData->SetNumberOfScalarComponents(1);
//   this->ImageLayer.ImageData->SetScalarTypeToUnsignedChar();
//   this->AllocateScalars();

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(ImageLayer.ImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->Camera = new ImageCamera(this->Renderer);
}

void PriorityViewerWidget::slot_UpdateImage()
{
  std::cout << "PriorityViewerWidget::slot_UpdateImage." << std::endl;

  // Compute the priority at every pixel
  itk::ImageRegionIterator<PriorityImageType> imageIterator(PriorityImage, PriorityImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(PriorityFunction->ComputePriority(imageIterator.GetIndex()));

    ++imageIterator;
    }

//   // Rescale the priorities to be displayed as a grayscale image
//   typedef itk::RescaleIntensityImageFilter<PriorityImageType, PriorityImageType> RescaleFilterType;
//   RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
//   rescaleFilter->SetInput(PriorityImage);
//   rescaleFilter->SetOutputMinimum(0);
//   rescaleFilter->SetOutputMaximum(255);
//   rescaleFilter->Update();

  ITKVTKHelpers::ITKScalarImageToScaledVTKImage(PriorityImage, this->ImageLayer.ImageData);

  if(!PreviouslyDisplayed)
    {
    this->Renderer->ResetCamera();
    PreviouslyDisplayed = true;
    }

  this->qvtkWidget->GetRenderWindow()->Render();
}
