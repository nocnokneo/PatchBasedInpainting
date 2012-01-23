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

#ifndef ImagePatchPixelDescriptor_H
#define ImagePatchPixelDescriptor_H

#include "PixelVisitor.h"
#include "Types.h"

class Mask;

/**
\class ImagePatchPixelDescriptor
\brief This class indicates a rectangular region in an image.
*/
template <typename TImage>
class ImagePatchPixelDescriptor
{
public:

  /** Construct a patch from a region.*/
  ImagePatchPixelDescriptor(const TImage* const image, const itk::ImageRegion<2>& region);

  /** Compute the difference to another ImagePatch.*/
  float Compare(const ImagePatchPixelDescriptor* const item) const;
  
  /** Check if two patches are the same.*/
  bool operator==(const ImagePatchPixelDescriptor& other) const;

  /** Check if two patches are different.*/
  bool operator!=(const ImagePatchPixelDescriptor& other) const;

  /** Get the region described by the patch.*/
  itk::ImageRegion<2> GetRegion() const;

  /** Get the corner of the patch.*/
  itk::Index<2> GetCorner() const;

  /** Sort the patches by index (so they can be stored in a container such as std::set).*/
  bool operator<(const ImagePatchPixelDescriptor& other) const;

  /** Output information about the patch. Even though this is inside a class template definition, we still need to declare it as a function template. */
  template <typename T>
  friend std::ostream& operator<<(std::ostream& output,  const ImagePatchPixelDescriptor<T>& patch);

  /** Visit all pixels in a patch.*/
  void VisitAllPixels(const TImage* const image, PixelVisitor<typename TImage::PixelType> &visitor);

  /** Visit all pixels in a patch where the mask value is valid.*/
  void VisitAllValidPixels(const TImage* const image, const Mask* const mask, PixelVisitor<typename TImage::PixelType> &visitor);

  /** Visit the pixels in a patch specified by a list of offsets from the corner of the patch. */
  void VisitOffsets(const TImage* const image, const std::vector<itk::Offset<2> >& offsets, PixelVisitor<typename TImage::PixelType> &visitor);

  TImage* GetImage() const;

private:
  /** The region in the image defining the location of the patch. */
  itk::ImageRegion<2> Region;

  /** The image that the patch points to. */
  TImage* Image;

};

#include "ImagePatchPixelDescriptor.hxx"

#endif