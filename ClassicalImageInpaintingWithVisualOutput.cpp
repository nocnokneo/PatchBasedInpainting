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

// Custom
#include "Helpers/OutputHelpers.h"

#include "Visitors/InformationVisitors/FillOrderLoggerVisitor.hpp"

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Acceptance visitors
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Information visitors
#include "Visitors/InformationVisitors/DisplayVisitor.hpp"
#include "Visitors/InformationVisitors/DebugVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/CompositeInpaintingVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/LinearSearchBestProperty.hpp"
//#include "NearestNeighbor/DefaultSearchBest.hpp"
#include "NearestNeighbor/FirstValidDescriptor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/MaskImagePatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityOnionPeel.h"
#include "Priority/PriorityCriminisi.h"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Debug
#include "Helpers/OutputHelpers.h"

// Qt
#include <QApplication>
#include <QtConcurrentRun>

// Custom
#include "Interactive/BasicViewerWidget.h"

// Run with: Data/trashcan.mha Data/trashcan_mask.mha 15 filled.mha
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
    {
    std::cerr << "Required arguments: image.mha imageMask.mha patch_half_width output.mha" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
      {
      std::cerr << argv[i] << " ";
      }
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  unsigned int patchHalfWidth = 0;
  ssPatchRadius >> patchHalfWidth;

  std::string outputFilename = argv[4];

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

  typedef FloatVectorImageType ImageType;

  typedef  itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType::Pointer image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths =
               { { imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0],
                   imageReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the priority map
  typedef boost::vector_property_map<float, IndexMapType> PriorityMapType;
  PriorityMapType priorityMap(num_vertices(graph), indexMap);

  // Create the boundary status map. A node is on the current boundary if this property is true. 
  // This property helps the boundaryNodeQueue because we can mark here if a node has become no longer
  // part of the boundary, so when the queue is popped we can check this property to see if it should
  // actually be processed.
  typedef boost::vector_property_map<bool, IndexMapType> BoundaryStatusMapType;
  BoundaryStatusMapType boundaryStatusMap(num_vertices(graph), indexMap);

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter. The inpainter needs to know the status of each pixel
  // to determine if they should be inpainted.
  typedef MaskImagePatchInpainter InpainterType;
  InpainterType patchInpainter(patchHalfWidth, mask);

  // Create the priority function
//   typedef PriorityRandom PriorityType;
//   PriorityType priorityFunction;

//   typedef PriorityOnionPeel PriorityType;
//   PriorityType priorityFunction(mask, patchHalfWidth);

  typedef PriorityCriminisi<ImageType> PriorityType;
  PriorityType priorityFunction(image, mask, patchHalfWidth);

  // Create the boundary node queue. The priority of each node is used to order the queue.
  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  // Create the priority compare functor - we want to use the highest priority pixels first
  typedef std::greater<float> PriorityCompareType;
  PriorityCompareType lessThanFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap,
                                     PriorityMapType, PriorityCompareType> BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(priorityMap, index_in_heap, lessThanFunctor);

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType>
            ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(image, mask, imagePatchDescriptorMap, patchHalfWidth);

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  AcceptanceVisitorType acceptanceVisitor;

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, ImageType, BoundaryNodeQueueType,
                            ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType,
                            PriorityMapType, BoundaryStatusMapType>
                            InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(image, mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor, priorityMap,
                                          &priorityFunction, patchHalfWidth,
                                          boundaryStatusMap, outputFilename);
  
  typedef DisplayVisitor<VertexListGraphType, ImageType> DisplayVisitorType;
  DisplayVisitorType displayVisitor(image, mask, patchHalfWidth);

  typedef DebugVisitor<VertexListGraphType, ImageType, BoundaryStatusMapType, BoundaryNodeQueueType>
          DebugVisitorType;
  DebugVisitorType debugVisitor(image, mask, patchHalfWidth, boundaryStatusMap, boundaryNodeQueue);

  typedef FillOrderLoggerVisitor<VertexListGraphType> FillOrderLoggerVisitorType;
  FillOrderLoggerVisitorType fillOrderLoggerVisitor("fill_order.mha", mask, patchHalfWidth);

  typedef CompositeInpaintingVisitor<VertexListGraphType> CompositeVisitorType;
  CompositeVisitorType compositeVisitor;
  compositeVisitor.AddVisitor(&inpaintingVisitor);
  compositeVisitor.AddVisitor(&displayVisitor);
  compositeVisitor.AddVisitor(&debugVisitor);
  compositeVisitor.AddVisitor(&fillOrderLoggerVisitor);

  InitializePriority(mask, boundaryNodeQueue, priorityMap, &priorityFunction, boundaryStatusMap);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<ImagePatchDescriptorVisitorType, VertexDescriptorType>(mask.GetPointer(),
                                                                                 &imagePatchDescriptorVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.size()
            << " nodes in the boundaryNodeQueue" << std::endl;

  // Create the nearest neighbor finder
//   typedef LinearSearchBestProperty<ImagePatchDescriptorMapType,
//            ImagePatchDifference<ImagePatchPixelDescriptorType> > BestSearchType;
//   BestSearchType searchBest(imagePatchDescriptorMap);

//   typedef DefaultSearchBest BestSearchType;
//   BestSearchType searchBest;

  typedef FirstValidDescriptor<ImagePatchDescriptorMapType> BestSearchType;
  BestSearchType searchBest(imagePatchDescriptorMap);

  // Setup the GUI
  QApplication app( argc, argv );

  BasicViewerWidget<ImageType> basicViewerWidget(image, mask);

  basicViewerWidget.show();

  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshImage()), &basicViewerWidget, SLOT(slot_UpdateImage()),
                   Qt::BlockingQueuedConnection);
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshSource(const itk::ImageRegion<2>&,
                                                                const itk::ImageRegion<2>&)),
                   &basicViewerWidget, SLOT(slot_UpdateSource(const itk::ImageRegion<2>&,
                                                              const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshTarget(const itk::ImageRegion<2>&)),
                   &basicViewerWidget, SLOT(slot_UpdateTarget(const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(&displayVisitor, SIGNAL(signal_RefreshResult(const itk::ImageRegion<2>&,
                                                                const itk::ImageRegion<2>&)),
                   &basicViewerWidget, SLOT(slot_UpdateResult(const itk::ImageRegion<2>&,
                                                              const itk::ImageRegion<2>&)),
                   Qt::BlockingQueuedConnection);

  QtConcurrent::run(boost::bind(InpaintingAlgorithm<VertexListGraphType, CompositeVisitorType,
                                BoundaryStatusMapType, BoundaryNodeQueueType, BestSearchType, InpainterType>,
                              graph, compositeVisitor, &boundaryStatusMap, &boundaryNodeQueue, searchBest,
                              patchInpainter));

//   InpaintingAlgorithm<VertexListGraphType, CompositeVisitorType, BoundaryStatusMapType,
//  BoundaryNodeQueueType, BestSearchType, InpainterType>(
//                               graph, compositeVisitor, boundaryStatusMap, boundaryNodeQueue,
//  searchBest, patchInpainter);
  return app.exec();
}
