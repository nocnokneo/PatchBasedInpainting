#ifndef SourceHoleTargetValidCompare_HPP
#define SourceHoleTargetValidCompare_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "Mask/Mask.h"
#include "ITKHelpers/ITKHelpers.h"
#include "BoostHelpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage, typename TFunctor>
struct SourceHoleTargetValidCompare : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  TFunctor Functor;

  float DifferenceThreshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  SourceHoleTargetValidCompare(TImage* const image, Mask* const mask, const unsigned int halfWidth,
                               TFunctor functor, const float differenceThreshold = 100,
    const std::string& visitorName = "SourceHoleTargetValidCompare") :
  AcceptanceVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth),
  NumberOfFinishedVertices(0), Functor(functor), DifferenceThreshold(differenceThreshold)
  {
  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    //std::cout << "DilatedVarianceDifferenceAcceptanceVisitor::AcceptMatch" << std::endl;

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    std::vector<itk::Offset<2> > validOffsets = MaskImage->GetValidOffsetsInRegion(targetRegion);
    std::vector<itk::Offset<2> > holeOffsets = MaskImage->GetHoleOffsetsInRegion(targetRegion);

    std::vector<itk::Index<2> > validPixelsIndicesTargetRegion =
           ITKHelpers::OffsetsToIndices(validOffsets, targetRegion.GetIndex());
    std::vector<typename TImage::PixelType> validPixelsTargetRegion =
           ITKHelpers::GetPixelValues(Image, validPixelsIndicesTargetRegion);

    typename TypeTraits<typename TImage::PixelType>::LargerType targetValue = Functor(validPixelsTargetRegion);

    std::vector<itk::Index<2> > holePixelsIndicesSourceRegion =
           ITKHelpers::OffsetsToIndices(holeOffsets, sourceRegion.GetIndex());
    std::vector<typename TImage::PixelType> holePixelsSourceRegion =
           ITKHelpers::GetPixelValues(Image, holePixelsIndicesSourceRegion);

    typename TypeTraits<typename TImage::PixelType>::LargerType sourceValue =
           Functor(holePixelsSourceRegion);

    // Compute the difference
    typename TypeTraits<typename TImage::PixelType>::LargerType difference = targetValue - sourceValue;
    //computedEnergy = (targetValue - sourceValue).GetNorm();
    computedEnergy = ITKHelpers::SumOfComponentMagnitudes(difference);
    //std::cout << this->VisitorName << ": Energy: " << computedEnergy << std::endl;

    if(computedEnergy < DifferenceThreshold)
      {
      std::cout << this->VisitorName << ": Match accepted (" << computedEnergy << " is less than "
                << DifferenceThreshold << ")" << std::endl << std::endl;
      return true;
      }
    else
      {
      std::cout << this->VisitorName << ": Match rejected (" << computedEnergy << " is greater than "
                << DifferenceThreshold << ")" << std::endl << std::endl;
      return false;
      }
  };

};

#endif
