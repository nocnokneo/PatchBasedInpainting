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

#include "ImagePatch.h" // Make syntax parser happy

#include "Mask.h"

#include "itkImageRegionConstIteratorWithIndex.h"

template <typename TImage>
void ImagePatch<TImage>::VisitAllPixels(const TImage* const image, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::ImageRegionConstIterator<TImage> imageIterator(image, this->Region);

  while(!imageIterator.IsAtEnd())
    {
    visitor.Visit(imageIterator.Get());
    ++imageIterator;
    }
}

template <typename TImage>
TImage* ImagePatch<TImage>::GetImage() const
{
  return this->Image;
}

template <typename TImage>
void ImagePatch<TImage>::VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::ImageRegionConstIteratorWithIndex<TImage> imageIterator(image, this->Region);

  while(!imageIterator.IsAtEnd())
    {
    if(mask->IsValid(imageIterator.GetIndex()))
      {
      visitor.Visit(imageIterator.Get());
      }
    ++imageIterator;
    }
}

template <typename TImage>
void ImagePatch<TImage>::VisitOffsets(const TImage* const image, const std::vector<itk::Offset<2> >& offsets, PixelVisitor<typename TImage::PixelType> &visitor)
{
  itk::Index<2> corner = this->Region.GetIndex();

  for(unsigned int offsetId = 0; offsetId < offsets.size(); ++offsetId)
    {
    visitor.Visit(image->GetPixel(corner + offsets[offsetId]));
    }
}


#include "itkImageRegionConstIterator.h"

template <typename TImage>
ImagePatch<TImage>::ImagePatch(const TImage* const image, const itk::ImageRegion<2>& region)
{
  this->Region = region;
}

template <typename TImage>
itk::Index<2> ImagePatch<TImage>::GetCorner() const
{
  return this->Region.GetIndex();
}

template <typename TImage>
itk::ImageRegion<2> ImagePatch<TImage>::GetRegion() const
{
  return this->Region;
}

template <typename TImage>
std::ostream& operator<<(std::ostream& output, const ImagePatch<TImage> &patch)
{
  output << "Patch: " << patch.GetRegion() << std::endl;
  return output;
}

template <typename TImage>
bool ImagePatch<TImage>::operator==(const ImagePatch& other) const
{
  if(this->Region == other.Region)
    {
    return true;
    }
  return false;
}

template <typename TImage>
bool ImagePatch<TImage>::operator!=(const ImagePatch& other) const
{
  return !operator==(other);
}

template <typename TImage>
bool ImagePatch<TImage>::operator<(const ImagePatch &other) const
{
  if(this->Region.GetIndex()[0] < other.Region.GetIndex()[0])
    {
    return true;
    }
  else if (other.Region.GetIndex()[0] < this->Region.GetIndex()[0])
    {
    return false;
    }

  if (this->Region.GetIndex()[1] < other.Region.GetIndex()[1])
    {
    return true;
    }
  else if (other.Region.GetIndex()[1] < this->Region.GetIndex()[1])
    {
    return false;
    }
  assert(0); // This should never be reached
  return true;
}
