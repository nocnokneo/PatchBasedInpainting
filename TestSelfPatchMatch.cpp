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

#include "Types.h"
#include "SelfPatchMatch.h"

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Only gave " << argc << " arguments!" << std::endl;
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  itk::Index<2> queryPixel;
  queryPixel[0] = 10;
  queryPixel[1] = 10;

  unsigned int patchRadius = 5;

  /*
  itk::Index<2> bestMatch = SelfPatchMatch(imageReader->GetOutput(), maskReader->GetOutput(), queryPixel, patchRadius);

  std::cout << "Best match: " << bestMatch << std::endl;

  if(bestMatch != queryPixel)
    {
    std::cerr << "Error: the best match should be the query pixel!" << std::endl;
    std::cerr << "Query: " << queryPixel << " Best match: " << bestMatch << std::endl;
    return EXIT_FAILURE;
    }
  */
  return EXIT_SUCCESS;
}
