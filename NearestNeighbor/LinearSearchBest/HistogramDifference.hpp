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

#ifndef LinearSearchBestHistogramDifference_HPP
#define LinearSearchBestHistogramDifference_HPP

#include "HistogramParent.hpp"

#include <Utilities/Histogram/HistogramHelpers.hpp>
#include <Utilities/Histogram/HistogramDifferences.hpp>

/**
   * This function template is similar to std::min_element but can be used when the comparison
   * involves computing a derived quantity (a.k.a. distance). This algorithm will search for the
   * the element in the range [first,last) which has the "smallest" distance (of course, both the
   * distance metric and comparison can be overriden to perform something other than the canonical
   * Euclidean distance and less-than comparison, which would yield the element with minimum distance).
   * \tparam DistanceValueType The value-type for the distance measures.
   * \tparam DistanceFunctionType The functor type to compute the distance measure.
   * \tparam CompareFunctionType The functor type that can compare two distance measures (strict weak-ordering).
   */
template <typename PropertyMapType, typename TImage, typename TIterator, typename TImageToWrite = TImage>
class LinearSearchBestHistogramDifference : public LinearSearchBestHistogramParent<PropertyMapType, TImage, TIterator, TImageToWrite>
{

public:
  /** Constructor. This class requires the property map, an image, and a mask. */
  LinearSearchBestHistogramDifference(PropertyMapType propertyMap, TImage* const image, Mask* const mask) :
    LinearSearchBestHistogramParent<PropertyMapType, TImage, TIterator, TImageToWrite>(propertyMap, image, mask)
  {}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  typename TIterator::value_type operator()(const TIterator first, const TIterator last, typename TIterator::value_type query)
  {
    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      std::cerr << "LinearSearchBestHistogram: Nothing to do..." << std::endl;
      return *last;
    }

    // If the input element range is empty, there is nothing to do.
    for(unsigned int channel = 0; channel < this->Image->GetNumberOfComponentsPerPixel(); ++channel)
    {
      if(!(this->RangeMax[channel] > this->RangeMin[channel]))
      {
        std::stringstream ss;
        ss << "LinearSearchBestHistogramDifference: Channel " << channel
           << " has RangeMax (" << this->RangeMax[channel] << ") must be > RangeMin ("
           << this->RangeMin[channel] << ")";
        throw std::runtime_error(ss.str());
      }
    }

    typedef int BinValueType;
    typedef MaskedHistogramGenerator<BinValueType> MaskedHistogramGeneratorType;
    typedef HistogramGenerator<BinValueType>::HistogramType HistogramType;

    itk::ImageRegion<2> queryRegion = get(this->PropertyMap, query).GetRegion();

    bool allowOutside = true;
    HistogramType targetHistogram =
      MaskedHistogramGeneratorType::ComputeMaskedImage1DHistogram(
          this->Image, queryRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
          this->RangeMin, this->RangeMax, allowOutside, this->MaskImage->GetValidValue());

    if(this->WriteDebugPatches)
    {
      std::cout << "LinearSearchBestHistogram: Iteration " << this->Iteration << std::endl;
      ITKHelpers::WriteRegionAsRGBImage(this->ImageToWrite, queryRegion, Helpers::GetSequentialFileName("QueryRegion",this->Iteration,"png",3));
//      ITKHelpers::WriteScalarImageRegion(this->MaskImage, queryRegion, Helpers::GetSequentialFileName("QueryMask",this->Iteration,"png",3));
      ITKHelpers::WriteRegion(this->MaskImage, queryRegion, Helpers::GetSequentialFileName("QueryMask",this->Iteration,"png",3));
      ITKHelpers::WriteImage(this->MaskImage, Helpers::GetSequentialFileName("Mask",this->Iteration,"png",3));
      ITKHelpers::WriteRegionAsRGBImage(this->ImageToWrite, get(this->PropertyMap, *first).GetRegion(),
                                        Helpers::GetSequentialFileName("SSDRegion",this->Iteration,"png",3));
      typename TImage::PixelType holeColor;
      holeColor[0] = 0;
      holeColor[1] = 255;
      holeColor[2] = 0;
      MaskOperations::WriteMaskedRegionPNG(this->ImageToWrite, this->MaskImage, queryRegion,
                                           Helpers::GetSequentialFileName("MaskedQueryRegion",this->Iteration,"png",3), holeColor);

      this->WriteTopPatches(first, last);

      // Compute the histogram of the best SSD region using the queryRegion mask
      HistogramType bestSSDHistogram =
       MaskedHistogramGeneratorType::ComputeMaskedImage1DHistogram(
                      this->Image, get(this->PropertyMap, *first).GetRegion(), this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                      this->RangeMin, this->RangeMax, allowOutside, this->MaskImage->GetValidValue());

      float ssdMatchHistogramScore = HistogramDifferences::HistogramDifference(targetHistogram, bestSSDHistogram);
//      float ssdMatchHistogramScore = HistogramDifferences::WeightedHistogramDifference(targetHistogram, bestSSDHistogram);
//      float ssdMatchHistogramScore = HistogramDifferences::HistogramCoherence(targetHistogram, bestSSDHistogram);


      std::cout << "Best SSDHistogramDifference: " << ssdMatchHistogramScore << std::endl;
      HistogramHelpers::WriteHistogram(bestSSDHistogram, Helpers::GetSequentialFileName("BestSSDHistogram",this->Iteration,"txt",3));
      HistogramHelpers::WriteHistogram(targetHistogram, Helpers::GetSequentialFileName("TargetHistogram",this->Iteration,"txt",3));
    }

    // Initialize
    float bestDistance = std::numeric_limits<float>::infinity();
    TIterator bestPatch = last;

    unsigned int bestId = 0; // Keep track of which of the top SSD patches is the best by histogram score (just for information sake)
    HistogramType bestHistogram;

    // Iterate through all of the input elements
    for(TIterator currentPatch = first; currentPatch != last; ++currentPatch)
    {
      itk::ImageRegion<2> currentRegion = get(this->PropertyMap, *currentPatch).GetRegion();

      // Compute the histogram of the source region using the queryRegion mask
      HistogramType testHistogram =
          MaskedHistogramGeneratorType::ComputeMaskedImage1DHistogram(
                      this->Image, currentRegion, this->MaskImage, queryRegion, this->NumberOfBinsPerDimension,
                      this->RangeMin, this->RangeMax, allowOutside, this->MaskImage->GetValidValue());

       float histogramDifference = HistogramDifferences::HistogramDifference(targetHistogram, testHistogram);
//      float histogramDifference = HistogramDifferences::WeightedHistogramDifference(targetHistogram, testHistogram);
//      float histogramDifference = HistogramDifferences::HistogramCoherence(targetHistogram, testHistogram);

      if(histogramDifference < bestDistance)
      {
        bestDistance = histogramDifference;
        bestPatch = currentPatch;

        // These are not needed - just for debugging
        bestId = currentPatch - first;
        bestHistogram = testHistogram;
      }
    }

    if(this->WriteDebugPatches)
    {
      itk::ImageRegion<2> bestRegion = get(this->PropertyMap, *bestPatch).GetRegion();
      ITKHelpers::WriteRegionAsRGBImage(this->ImageToWrite, bestRegion, Helpers::GetSequentialFileName("HistogramRegion",this->Iteration,"png",3));
      HistogramHelpers::WriteHistogram(bestHistogram, Helpers::GetSequentialFileName("BestHistogram",this->Iteration,"txt",3));
      std::cout << "Best histogram id: " << bestId << std::endl;
      std::cout << "Best histogramDifference: " << bestDistance << std::endl;
    }

    this->Iteration++;

    return *bestPatch;
  }

}; // end class LinearSearchBestHistogramDifference

#endif
