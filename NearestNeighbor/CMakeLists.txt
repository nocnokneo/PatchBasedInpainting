# if(BuildTests)
#   add_subdirectory(Tests)
# endif(BuildTests)

add_custom_target(NearestNeighbor SOURCES DefaultSearchBest.hpp
FirstValidDescriptor.hpp
LinearSearchAlgorithm.hpp
LinearSearchBest.hpp
LinearSearchCriteriaProperty.hpp
LinearSearchKNN.hpp
LinearSearchKNNProperty.hpp
LocalOptimizationSearchBestProperty.hpp
metric_space_concept.hpp
metric_space_search.hpp
PassThrough.hpp
PrecomputedNeighbors.hpp
SearchFunctor.hpp
StorageAndSearchFunctor.hpp
ThreeStepNearestNeighbor.hpp
topological_search.hpp
TwoStepNearestNeighbor.hpp
VisualSelectionBest.hpp
weak_metric_space_concept.hpp)

add_subdirectory(LinearSearchBest)
