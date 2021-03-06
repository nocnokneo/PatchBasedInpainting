#ifndef LinearSearchBestProperty_HPP
#define LinearSearchBestProperty_HPP

// STL
#include <iostream>
#include <limits>

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
template <typename PropertyMapType,
          typename DistanceFunctionType>
struct LinearSearchBestProperty
{
  PropertyMapType PropertyMap;
  DistanceFunctionType DistanceFunction;

  LinearSearchBestProperty(PropertyMapType propertyMap, DistanceFunctionType distanceFunction = DistanceFunctionType()) :
  PropertyMap(propertyMap), DistanceFunction(distanceFunction){}

  /**
    * \param first Start of the range in which to search.
    * \param last One element past the last element in the range in which to search.
    * \param query The element to compare to.
    * \return The iterator to the best element in the range (best is defined as the one which would compare favorably to all
    *         the elements in the range with respect to the distance metric).
    */
  template <typename TIterator>
  typename TIterator::value_type operator()(TIterator first, TIterator last, typename TIterator::value_type query)
  {
    // If the input element range is empty, there is nothing to do.
    if(first == last)
    {
      return *last;
    }

    // Initialize
    float d_best = std::numeric_limits<float>::infinity();
    TIterator result = last;

    // Iterate through all of the input elements
//    #pragma omp parallel for
//    for(TIterator current = first; current != last; ++current)
    for(TIterator current = first; current < last; ++current)
    {
      //DistanceValueType d = DistanceFunction(*first, query);
      float d = this->DistanceFunction(get(this->PropertyMap, *current), get(this->PropertyMap, query));

//      #pragma omp critical // There are weird crashes without this guard
      if(d < d_best)
      {
        d_best = d;
        result = current;
      }
    }

    std::cout << "Best id: " << result - first << " score: " << d_best << std::endl;
    return *result;
  }
};

#endif
