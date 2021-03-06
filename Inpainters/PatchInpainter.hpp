/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef PatchInpainter_HPP
#define PatchInpainter_HPP

#include "PatchInpainterParent.h"

#include <Utilities/Debug/Debug.h>

template <typename TImage>
class PatchInpainter : public PatchInpainterParent, public Debug
{
  /** The image to inpaint. */
  TImage* Image;

  /** The mask to use to determine which pixels are holes (which pixels to inpaint). */
  const Mask* MaskImage;

  /** The size of the patches used in the inpainting. */
  std::size_t PatchHalfWidth;

  // Debug only
  /** Count how many times this classes function has been performed. */
  unsigned int Iteration;

  /** The name of the image as the programmer would refer to it. I.e. "TheMask" or similar. */
  std::string ImageName;

public:

  /** Copy the pixel value at sourceIndex to targetIndex. */
  void PaintVertex(const itk::Index<2>& targetIndex, const itk::Index<2>& sourceIndex)
  {
    this->Image->SetPixel(targetIndex, this->Image->GetPixel(sourceIndex));
  }

  /** Specify a name for the image. */
  void SetImageName(const std::string& imageName)
  {
    this->ImageName = imageName;
  }

  /** Inpaint a patch only in the masked pixels. */
  PatchInpainter(std::size_t patchHalfWidth, TImage* const image, const Mask* const mask) :
    Image(image), MaskImage(mask), PatchHalfWidth(patchHalfWidth), Iteration(0), ImageName("Unnamed")
  {
//    std::cout << "PatchInpainter: size: " << this->Image->GetLargestPossibleRegion().GetSize() << std::endl;
  }

  void PaintPatch(const itk::Index<2>& targetCenter, const itk::Index<2>& sourceCenter)
  {
    assert(this->Image);

//    std::cout << "PatchInpainter: Mask size: " << this->MaskImage->GetLargestPossibleRegion().GetSize() << std::endl;

    itk::ImageRegion<2> targetRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(targetCenter, this->PatchHalfWidth);
    itk::ImageRegion<2> sourceRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(sourceCenter, this->PatchHalfWidth);

    // Ensure that the source patch will match the target patch after it is cropped
    sourceRegion = ITKHelpers::CropRegionAtPosition(sourceRegion, this->Image->GetLargestPossibleRegion(), targetRegion);

    // Ensure that the target patch is inside the image.
    targetRegion.Crop(this->Image->GetLargestPossibleRegion());

    if(this->GetDebugImages())
    {
      // Write the target patch that was inpainted.
      try
      {
        ITKHelpers::WriteRegion(this->Image, targetRegion,
                                Helpers::GetSequentialFileName("TargetPatchBefore", this->Iteration, "png", 3));
      }
      catch (...)
      {
        ITKHelpers::WriteRegionAsRGBImage(this->Image, targetRegion,
                                          Helpers::GetSequentialFileName("TargetPatchBefore", this->Iteration, "png", 3));
      }
    }

    // Iterate over all pixels in the target patch (we must iterate over the target patch, because it may be smaller than the source patch)
    itk::ImageRegionConstIteratorWithIndex<TImage> targetIterator(this->Image, targetRegion);

    while(!targetIterator.IsAtEnd())
    {
      itk::Offset<2> offset = targetIterator.GetIndex() - targetRegion.GetIndex();
      itk::Index<2> sourcePixel = sourceRegion.GetIndex() + offset;
      // Only paint the pixel if it is currently a hole
      if( this->MaskImage->IsHole(targetIterator.GetIndex()) )
      {
        PaintVertex(targetIterator.GetIndex(), sourcePixel);
      }
      ++targetIterator;
    }

    if(this->GetDebugOutputs())
    {
      std::cout << "PatchInpainter::PaintPatch(): Painted patch " << targetCenter[0] << " " << targetCenter[1]
                << " with " << sourceCenter[0] << " " << sourceCenter[1] << std::endl;
      std::cout << "PatchInpainter::PaintPatch() After filling, there are " << this->MaskImage->CountHolePixels() << " hole pixels remaining." << std::endl;
    }

    if(this->GetDebugImages())
    {
      // Write the inpainted image after this iteration. If the writer fails to write the image directly as a png (it would be an unsupported pixel type), then convert it to a supported type before writing.
      try
      {
        ITKHelpers::WriteSequentialImage(this->Image, this->ImageName, this->Iteration, 3, "png");
      }
      catch (...)
      {
        ITKHelpers::WriteSequentialRGBImage(this->Image, this->ImageName, this->Iteration, 3, "png");
      }

      // Write the target patch that was inpainted.
      try
      {
        ITKHelpers::WriteRegion(this->Image, targetRegion,
                                Helpers::GetSequentialFileName("TargetPatchAfter", this->Iteration, "png", 3));
      }
      catch (...)
      {
        ITKHelpers::WriteRegionAsRGBImage(this->Image, targetRegion,
                                          Helpers::GetSequentialFileName("TargetPatchAfter", this->Iteration, "png", 3));
      }
    }

    this->Iteration++;
  } // end operator()

}; // end class PatchInpainter

#endif
