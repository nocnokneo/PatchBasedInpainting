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

#ifndef PriorityCurvature_H
#define PriorityCurvature_H

#include "Priority.h"

// ITK
#include "itkImage.h"

// VTK
class vtkStructuredGrid;

/**
\class PriorityCurvature
\brief This class returns a the curvature value as the priority.
*/
template <typename TNode>
class PriorityCurvature
{
public:

  typedef itk::Image<float, 2> CurvatureImageType;

  PriorityCurvature(vtkStructuredGrid* const structuredGrid, const unsigned int patchRadius);

  /** Return the curvature - the higher the curvature, the more we want to fill this node.*/
  float ComputePriority(const TNode& queryPixel) const;

  /** Copy the source curvatures to the target curvatures.*/
  void Update(const TNode& sourceNode, const TNode& targetNode);

private:
  unsigned int PatchRadius;
  CurvatureImageType::Pointer CurvatureImage;
};

#include "PriorityCurvature.hpp"

#endif
