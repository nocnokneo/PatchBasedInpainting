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

#include "PriorityConfidence.h" // Appease syntax parser

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>

template <typename TNode>
void PriorityConfidence::Update(const TNode& sourceNode, const TNode& targetNode, const unsigned int patchNumber)
{
  float value = ComputeConfidenceTerm(targetNode);
  UpdateConfidences(targetNode, value);

  if(this->IsDebugOn())
  {
    ITKHelpers::WriteSequentialImage(this->ConfidenceMapImage.GetPointer(), "ConfidenceMap", patchNumber, 3, "mha");
  }
}

template <typename TNode>
float PriorityConfidence::ComputePriority(const TNode& queryPixel) const
{
  float priority = ComputeConfidenceTerm(queryPixel);

  return priority;
}

template <typename TNode>
void PriorityConfidence::UpdateConfidences(const TNode& targetNode, const float value)
{
  itk::Index<2> targetPixel = ITKHelpers::CreateIndex(targetNode);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

  // Force the region to update to be entirely inside the image
  region.Crop(this->MaskImage->GetLargestPossibleRegion());

  // Set the hole pixels in the region to 'value'. Since this is sensitive to when we inpaint the mask (before or after this function)
  // we instead use the technique below.
//  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

//  while(!maskIterator.IsAtEnd())
//  {
//    if(maskIterator.Get() == this->MaskImage->GetHoleValue()) // avoid the GetPixel call in the above line.
//    {
//      this->ConfidenceMapImage->SetPixel(maskIterator.GetIndex(), value);
//      // std::cout << "Set " << maskIterator.GetIndex() << " to " << value << std::endl;
//    }
//    ++maskIterator;
//  }

  // Set the pixels which currently have a confidence of zero in the region to 'value'.
  itk::ImageRegionIterator<ConfidenceImageType> confidenceImageIterator(this->ConfidenceMapImage, region);

  while(!confidenceImageIterator.IsAtEnd())
  {
    if(confidenceImageIterator.Get() == 0.0f)
    {
      confidenceImageIterator.Set(value);
      // std::cout << "Set " << maskIterator.GetIndex() << " to " << value << std::endl;
    }
    ++confidenceImageIterator;
  }

}

template <typename TNode>
float PriorityConfidence::ComputeConfidenceTerm(const TNode& queryNode) const
{
  // Sum the confidence map values in the valid region

  itk::Index<2> queryPixel = ITKHelpers::CreateIndex(queryNode);

  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

  // Ensure that the patch to use to compute the confidence is entirely inside the image
  region.Crop(this->MaskImage->GetLargestPossibleRegion());

  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);
  itk::ImageRegionConstIterator<ConfidenceImageType> confidenceImageIterator(this->ConfidenceMapImage, region);

  // The confidence is computed as the sum of the confidences of patch pixels
  // in the source region / area of the patch

  float sum = 0.0f;

  while(!maskIterator.IsAtEnd())
  {
    if(maskIterator.Get() == this->MaskImage->GetValidValue())
    {
      sum += confidenceImageIterator.Get();
    }
    ++confidenceImageIterator;
    ++maskIterator;
  }

  if(sum == 0.0f)
  {
    throw std::runtime_error("Confidence is zero!");
  }

  unsigned int numberOfPixels = region.GetNumberOfPixels();
  float areaOfPatch = static_cast<float>(numberOfPixels);

  float confidence = sum/areaOfPatch;

  return confidence;
}
