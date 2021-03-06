#include <cmath>
#include <iostream>
#include <vector>

#include "NearestNeighbor/LinearSearchBest.hpp"

typedef std::vector<float> DescriptorType;

struct DifferenceFunctor
{
  DescriptorType objectToCompare;

  float operator()(const DescriptorType& a, const DescriptorType& b)
  {
    float difference = 0.0f;
    for(unsigned int i = 0; i < a.size(); ++i)
      {
      difference += fabs(a[i] - b[i]);
      }
    return difference;
  }

  float operator()(const DescriptorType& other)
  {
    float difference = 0.0f;
    for(unsigned int i = 0; i < other.size(); ++i)
      {
      difference += fabs(objectToCompare[i] - other[i]);
      }
    return difference;
  }
};

int main(int argc, char *argv[])
{
  typedef std::vector<DescriptorType> DescriptorCollectionType;
  DescriptorCollectionType descriptors;

  const unsigned int descriptorDimension = 4;
  for(unsigned int i = 0; i < 10; ++i)
    {
    DescriptorType descriptor(descriptorDimension);
    for(unsigned int component = 0; component < descriptorDimension; ++component)
      {
      descriptor[component] = i;
      }
    descriptors.push_back(descriptor);
    }

  DescriptorType queryDescriptor(descriptorDimension, 5.3);
  DifferenceFunctor differenceObject;
  differenceObject.objectToCompare = queryDescriptor;
  LinearSearchBest<DescriptorCollectionType::iterator, DifferenceFunctor> linearSearchBest;
  DescriptorCollectionType::iterator result = linearSearchBest(descriptors.begin(), descriptors.end(), queryDescriptor);

  std::cout << "Closest integer point to: " << queryDescriptor[0] << " is " << (*result)[0] << std::endl; // Output the first component of the best match

  return 0;
}
