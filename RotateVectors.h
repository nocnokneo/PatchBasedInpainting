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

#ifndef RotateVectors_h
#define RotateVectors_h

#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkRigid2DTransform.h"

template< class TInput, class TOutput>
class RotateVectors
{
public:
  RotateVectors() {};
  ~RotateVectors() {};
  bool operator!=( const RotateVectors & ) const
    {
    return false;
    }
  bool operator==( const RotateVectors & other ) const
    {
    return !(*this != other);
    }
  inline TOutput operator()( const TInput & A ) const
    {
      // Rotate the vector by 90 degrees.
      TInput output;

      output[0] = -A[1];
      output[1] = A[0];

      return output;
    }
};

#endif