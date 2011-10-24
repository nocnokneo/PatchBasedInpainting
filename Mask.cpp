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

#include "Mask.h"
#include <itkFlatStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>

bool Mask::IsHole(const itk::Index<2>& index) const
{
  if(this->GetPixel(index) == this->HoleValue)
    {
    return true;
    }
  return false;
}
    
bool Mask::IsValid(const itk::ImageRegion<2>& region) const
{
  // If any of the pixels in the region are invalid, the region is invalid.
  
  itk::ImageRegionConstIterator<Mask> maskIterator(this, region);

  while(!maskIterator.IsAtEnd())
    {
    if(!this->IsValid(maskIterator.GetIndex()))
      {
      //std::cout << "Mask::IsValid - Pixel " << maskIterator.GetIndex() << " has value " << static_cast<unsigned int>(maskIterator.Get()) 
      //          << " which makes the region invalid because Mask::ValidValue = " << static_cast<unsigned int>(this->ValidValue) << std::endl;
      return false;
      }

    ++maskIterator;
    }
  return true;
}

bool Mask::IsValid(const itk::Index<2>& index) const
{
  if(this->GetPixel(index) == this->ValidValue)
    {
    return true;
    }
  return false;
}

void Mask::Invert()
{
  // Exchange HoleValue and ValidValue, but leave everything else alone.
  itk::ImageRegionIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());
  unsigned int invertedCounter = 0;
  while(!maskIterator.IsAtEnd())
    {
    if(this->IsValid(maskIterator.GetIndex()))
      {
      maskIterator.Set(this->HoleValue);
      invertedCounter++;
      }
    else if(this->IsHole(maskIterator.GetIndex()))
      {
      maskIterator.Set(this->ValidValue);
      invertedCounter++;
      }
    ++maskIterator;
    }
  std::cout << "Inverted " << invertedCounter << " in the mask." << std::endl;
}

void Mask::Cleanup()
{
  // We want to interpret pixels that are "pretty much hole value" as holes, and pixels that
  // are "pretty much valid value" as valid. The "do not use" pixels must be very far away from both of these values.
  itk::ImageRegionIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());
  
  float tolerance = 4;
  while(!maskIterator.IsAtEnd())
    {
    if(fabs(maskIterator.Get() - this->ValidValue) < tolerance)
      {
      //std::cout << "Setting valid pixel to " << static_cast<unsigned int>(this->ValidValue) << std::endl;
      maskIterator.Set(this->ValidValue);
      }
    else if(fabs(maskIterator.Get() - this->HoleValue) < tolerance)
      {
      //std::cout << "Setting hole pixel to " << static_cast<unsigned int>(this->HoleValue) << std::endl;
      maskIterator.Set(this->HoleValue);
      }
    ++maskIterator;
    }
  
}

void Mask::SetHoleValue(const unsigned char value)
{
  this->HoleValue = value; 
}

void Mask::SetValidValue(const unsigned char value)
{
  this->ValidValue = value; 
}

unsigned char Mask::GetHoleValue() const
{
  return this->HoleValue;
}

unsigned char Mask::GetValidValue() const
{
  return this->ValidValue;
}

void Mask::OutputMembers() const
{
  std::cout << "HoleValue: " << static_cast<unsigned int>(this->HoleValue) << std::endl;
  std::cout << "ValidValue: " << static_cast<unsigned int>(this->ValidValue) << std::endl;
}

void Mask::DeepCopyFrom(const Mask::Pointer inputMask)
{
  this->SetRegions(inputMask->GetLargestPossibleRegion());
  this->Allocate();

  itk::ImageRegionConstIterator<Mask> inputIterator(inputMask, inputMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<Mask> thisIterator(this, this->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    thisIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++thisIterator;
    }
  this->SetHoleValue(inputMask->GetHoleValue());
  this->SetValidValue(inputMask->GetValidValue());
}


Mask::Mask()
{
  this->HoleValue = 255;
  this->ValidValue = 0;
}

void Mask::ExpandHole()
{
  // Expand the mask - this is necessary to prevent the isophotes from being undefined in the target region
  typedef itk::FlatStructuringElement<2> StructuringElementType;
  StructuringElementType::RadiusType radius;
  radius.Fill(2); // Just a little bit of expansion
  //radius.Fill(this->PatchRadius[0]); // This was working, but huge expansion
  //radius.Fill(2.0* this->PatchRadius[0]);

  StructuringElementType structuringElement = StructuringElementType::Box(radius);
  typedef itk::BinaryDilateImageFilter<Mask, Mask, StructuringElementType> BinaryDilateImageFilterType;
  BinaryDilateImageFilterType::Pointer expandMaskFilter = BinaryDilateImageFilterType::New();
  expandMaskFilter->SetInput(this);
  expandMaskFilter->SetKernel(structuringElement);
  expandMaskFilter->Update();
    
  //Helpers::DeepCopy<Mask>(expandMaskFilter->GetOutput(), this->CurrentMask);
  this->DeepCopyFrom(expandMaskFilter->GetOutput());

}

void Mask::MakeVTKImage(vtkImageData* image, const QColor& validColor, const QColor& holeColor, const bool holeTransparent, const bool validTransparent)
{
  int dims[3];
  dims[0] = this->GetLargestPossibleRegion().GetSize()[0];
  dims[1] = this->GetLargestPossibleRegion().GetSize()[1];
  dims[2] = 1;
 
  image->SetScalarTypeToUnsignedChar();
  image->SetNumberOfScalarComponents(4);
  image->SetDimensions(dims);
  image->AllocateScalars();
  
  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      // Get a pointer to the pixel so we can modify it.
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      itk::Index<2> index;
      index[0] = i;
      index[1] = j;
      if(this->IsValid(index))
	{
	pixel[0] = validColor.red();
	pixel[1] = validColor.green();
	pixel[2] = validColor.blue();
      
	if(validTransparent)
	  {
	  pixel[3] = 0; // invisible  
	  }
	else
	  {
	  pixel[3] = 255; // visible
	  }
	}
      else // Pixel is a hole
	{
	pixel[0] = holeColor.red();
	pixel[1] = holeColor.green();
	pixel[2] = holeColor.blue();
      
	if(holeTransparent)
	  {
	  pixel[3] = 0; // invisible  
	  }
	else
	  {
	  pixel[3] = 255; // visible
	  }
	}

      }
    }
}