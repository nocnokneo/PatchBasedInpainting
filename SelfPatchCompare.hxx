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

#include "CandidatePairs.h"

template<typename TDifferenceFunction>
float SelfPatchCompare::PatchAverageSourceDifference(const Patch* sourcePatch)
{
  EnterFunction("SelfPatchCompare::PatchAverageSourceDifference<TDifferenceFunction>()");
  if(this->ValidTargetPatchOffsets.size() <= 0)
    {
    std::cout << "SelfPatchCompare::PatchAverageSourceDifference had " << this->ValidTargetPatchOffsets.size() << " ValidTargetPatchOffsets on which to operate!" << std::endl;
    exit(-1);
    }
  float totalDifference = 0.0f;

  // We want to do direct pointer arithmetic for speed purposes.
  const FloatVectorImageType::InternalPixelType* bufferPointer = this->Image->GetBufferPointer();

  // Get the locations of the corners of both patches.
  int sourceCornerOffset = this->Image->ComputeOffset(sourcePatch->GetRegion().GetIndex());
  int targetCornerOffset = this->Image->ComputeOffset(this->Pairs->GetTargetPatch().GetRegion().GetIndex());

  // Compute the difference between the corners (from the target to the source).
  int targetToSourceOffsetPixels = sourceCornerOffset - targetCornerOffset;
  int targetToSourceOffset = targetToSourceOffsetPixels * this->NumberOfComponentsPerPixel;

  // Create empty pixel containers that we will fill.
  FloatVectorImageType::PixelType sourcePixel(this->NumberOfComponentsPerPixel);
  FloatVectorImageType::PixelType targetPixel(this->NumberOfComponentsPerPixel);

  // Instantiate the distance function that has been specified as a template parameter.
  TDifferenceFunction differenceFunction(this->NumberOfComponentsPerPixel);
  float difference = 0;

  // Loop through the pixels that have been determined to be valid.
  for(unsigned int pixelId = 0; pixelId < this->ValidTargetPatchOffsets.size(); ++pixelId)
    {
    // Construct a pixel out of the data at the appropriate location.
    for(unsigned int component = 0; component < this->NumberOfComponentsPerPixel; ++component)
      {
      int sourceOffset = this->ValidTargetPatchOffsets[pixelId] + targetToSourceOffset + component;
      int targetOffset = this->ValidTargetPatchOffsets[pixelId] + component;

      sourcePixel[component] = bufferPointer[sourceOffset];

      targetPixel[component] = bufferPointer[targetOffset];
      } // end component loop

    difference = differenceFunction.Difference(sourcePixel, targetPixel);

    totalDifference += difference;
    } // end pixel loop

  float averageDifference = totalDifference/static_cast<float>(this->ValidTargetPatchOffsets.size());
  return averageDifference;
}
