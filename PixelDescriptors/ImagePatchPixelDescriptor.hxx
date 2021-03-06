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

#ifndef ImagePatchPixelDescriptor_hxx
#define ImagePatchPixelDescriptor_hxx

#include "ImagePatchPixelDescriptor.h" // Appease syntax parser

#include "Mask/Mask.h"

#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"

template <typename TImage>
ImagePatchPixelDescriptor<TImage>::ImagePatchPixelDescriptor() : Image(NULL), FullyValid(false), InsideImage(false)
{

}

template <typename TImage>
ImagePatchPixelDescriptor<TImage>::ImagePatchPixelDescriptor(TImage* const image,
                                                             Mask* const maskImage, const itk::ImageRegion<2>& region) :
  Region(region), Image(image), MaskImage(maskImage), InsideImage(false)
{
  if((region.GetIndex()[0] > 10000) || (region.GetIndex()[1] > 10000))
  {
    std::stringstream ss;
    ss << "ImagePatchPixelDescriptor() region.GetIndex() is invalid! "
       << region.GetIndex()[0] << ", " << region.GetIndex()[1] << " is not a valid vertex!";
    throw std::runtime_error(ss.str());
  }

  if(image->GetLargestPossibleRegion().IsInside(region))
  {
    this->InsideImage = true;
  }
  else
  {
    this->FullyValid = false;
    return;
  }

  this->FullyValid = maskImage->IsValid(region);

  if(this->FullyValid)
  {
    this->SetStatus(SOURCE_NODE);
  }
  else
  {
    this->SetStatus(INVALID);
  }
}

template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::IsFullyValid() const
{
  return this->FullyValid;
}

template <typename TImage>
bool ImagePatchPixelDescriptor<TImage>::IsInsideImage() const
{
  return this->InsideImage;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetImage(const TImage* const image)
{
  this->Image = image;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetRegion(const itk::ImageRegion<2>& region)
{
  this->Region = region;
}

template <typename TImage>
TImage* ImagePatchPixelDescriptor<TImage>::GetImage() const
{
  return this->Image;
}

template <typename TImage>
itk::Index<2> ImagePatchPixelDescriptor<TImage>::GetCorner() const
{
  return this->Region.GetIndex();
}

template <typename TImage>
itk::ImageRegion<2> ImagePatchPixelDescriptor<TImage>::GetRegion() const
{
  return this->Region;
}

template <typename TImage>
std::ostream& operator<<(std::ostream& output, const ImagePatchPixelDescriptor<TImage> &patch)
{
  output << "Patch: " << patch.GetRegion() << std::endl;
  return output;
}

template <typename TImage>
void ImagePatchPixelDescriptor<TImage>::SetValidOffsets(const std::vector<itk::Offset<2> >& validOffsets)
{
  this->ValidOffsets = validOffsets;
}

#endif
