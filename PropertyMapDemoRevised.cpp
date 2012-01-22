#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>


// Create tags for the properties you might have in your graphs:
namespace boost {

enum vertex_feature_vector_t { vertex_feature_vector };
enum vertex_image_patch_t { vertex_image_patch };

BOOST_INSTALL_PROPERTY(vertex,feature_vector);
BOOST_INSTALL_PROPERTY(vertex,image_patch);

};

class Image {};

// This is useless, all this does is associate a type-tag (FeatureVectorPixelDescriptor) with a type (std::vector<float>),
//  with BGL, the tag is the tags given above, and the type is whatever you give to the boost::property tuple.
/*
struct FeatureVectorPixelDescriptor
{
  std::vector<float> FeatureVector;
};

struct ImagePatchPixelDescriptor
{
  Image image;
};*/

// this is useless. The BGL's graphs are already tailored for composing properties.
/*struct CompositeDescriptor
{
  ImagePatchPixelDescriptor imagePatchPixelDescriptor;
  FeatureVectorPixelDescriptor featureVectorPixelDescriptor;
};*/

struct ComputeFeatureVector
{
  template <typename VertexListGraph>  //make your template parameter names more closely linked to what minimal concept they require (i.e. VertexListGraphConcept).
  void Compute(VertexListGraph& g, // Pass by reference!! Graphs are big things, don't pass by value!
	       typename boost::graph_traits<VertexListGraph>::vertex_descriptor vertexToSet) // Always use the graph_traits, don't access the nested type directly!! This is very important!
               const // Don't forget the const-ness.
  {
    std::vector<float> featureVector;
    // this is bad, don't assume that you can do this, in fact, I don't think you can!
    //g[vertexToSet].FeatureVector = featureVector;
    // To access the property, you need to use the property-map mechanisms:
    //  First, import the boost set/get functions into the ADL resolution:
    using boost::put; using boost::get;
    //  Second, define the type of the property-map that can fetch the feature-vector:
    typedef typename boost::property_map<VertexListGraph, boost::vertex_feature_vector_t>::type FeatureVectorPropertyMap;
    //  Third, obtain that property-map from the graph:
    FeatureVectorPropertyMap featureVectorProperty = get(boost::vertex_feature_vector, g);
    //   N.B., the above could also be done on construction of the ComputeFeatureVector object
    //   N.B.2, the above has no run-time cost at all, because the compiler can resolve the last get() operation statically, just based on the actual type of the graph.
    //  Finally, set the featureVector for the given vertex:
    put(featureVectorProperty, vertexToSet, featureVector);
  };
};

// Same here as above...
struct ComputeImagePatch
{
  template <typename VertexListGraph>
  void Compute(VertexListGraph& g, 
	       typename boost::graph_traits<VertexListGraph>::vertex_descriptor vertexToSet)
               const
  {
    Image image;
    using boost::put; using boost::get;
    put( get(boost::vertex_image_patch, g), vertexToSet, image);
  };
};


// Of course, if you need one functor to compute both properties, just do this:
struct ComputeFeatureVectorAndImagePatch
{
  template <typename VertexListGraph>
  void Compute(VertexListGraph& g, 
	       typename boost::graph_traits<VertexListGraph>::vertex_descriptor vertexToSet)
               const
  {
    std::vector<float> featureVector;
    Image image;
    using boost::put; using boost::get;
    put( get(boost::vertex_feature_vector, g), vertexToSet, featureVector);
    put( get(boost::vertex_image_patch, g), vertexToSet, image);
  };
};


// ****************** Optional *********************
// Maybe you've been wondering how to make composite property-maps out of single property-maps, 
//  here's one way to get that magic done (note that you can generalize this with boost::tuple):
template <typename TGraph>
struct CompositePropertyMap {
  typedef typename boost::property_map< TGraph, boost::vertex_feature_vector_t >::type FeatureVectorPropMap;
  typedef typename boost::property_map< TGraph, boost::vertex_image_patch_t >::type ImagePatchPropMap;
  
  // Define a few standard (required) nested types:
  typedef typename boost::property_traits< FeatureVectorPropMap >::key_type key_type;
  typedef std::pair< typename boost::property_traits< FeatureVectorPropMap >::value_type,
                     typename boost::property_traits< ImagePatchPropMap >::value_type > value_type;
  typedef boost::read_write_property_map_tag category;
  
  FeatureVectorPropMap featureVectorProp;
  ImagePatchPropMap imagePatchProp;
  
  CompositePropertyMap(TGraph& g) { 
    using boost::get;
    featureVectorProp = get(boost::vertex_feature_vector, g);
    imagePatchProp = get(boost::vertex_image_patch, g);
  };
};

// Get the composite property map associated to a graph:
template <typename TGraph>
CompositePropertyMap< TGraph > get( std::pair< boost::vertex_feature_vector_t, 
				               boost::vertex_image_patch_t >, 
				    TGraph& g) {
  return CompositePropertyMap< TGraph >(g);
};

// Get the composite-property associated to a vertex:
template <typename TGraph>
typename boost::property_traits< CompositePropertyMap< TGraph > >::value_type
  get( CompositePropertyMap< TGraph > p, 
       typename boost::property_traits< CompositePropertyMap< TGraph > >::key_type u) {
  typedef typename boost::property_traits< CompositePropertyMap< TGraph > >::value_type ResultType;
  using boost::get;
  return ResultType( get(p.featureVectorProp, u), get(p.imagePatchProp, u) );
};

// Set the composite-property associated to a vertex:
template <typename TGraph>
void put( CompositePropertyMap< TGraph > p, 
          typename boost::property_traits< CompositePropertyMap< TGraph > >::key_type u,
	  const typename boost::property_traits< CompositePropertyMap< TGraph > >::value_type& value) {
  using boost::put;
  put(p.featureVectorProp, u, value.first);
  put(p.imagePatchProp, u, value.second);
};

// ****************** End of Optional *********************



typedef boost::adjacency_list<boost::vecS, //OutEdgeList
                              boost::vecS,// VertexList
                              boost::undirectedS, //Directed,
                              boost::property< boost::vertex_feature_vector_t, std::vector<float> > //VertexProperties
                              > FeatureVectorGraph;

typedef boost::adjacency_list<boost::vecS, //OutEdgeList
                              boost::vecS,// VertexList
                              boost::undirectedS, //Directed,
                              boost::property< boost::vertex_image_patch_t, Image > //VertexProperties
                              > ImagePatchGraph;

typedef boost::adjacency_list<boost::vecS, //OutEdgeList
                              boost::vecS,// VertexList
                              boost::undirectedS, //Directed,
                              boost::property< boost::vertex_feature_vector_t, std::vector<float>,
			      boost::property< boost::vertex_image_patch_t, Image > > //VertexProperties
                              > CompositeDescriptorGraph;

class InpaintingParent
{
public:
  virtual void CreateGraph() = 0;
  virtual void ComputeAllDescriptors() = 0;
};

template <typename TGraph, typename TComputeDescriptor>
class PatchBasedInpainting : public InpaintingParent
{
public:
  void CreateGraph()
  {
    // Create two vertices (pixels) and an edge
    // USE TRAITS!
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexType;
    VertexType v0 = boost::add_vertex(g);

    VertexType v1 = boost::add_vertex(g);
    
    boost::add_edge(v0, v1, g);
  }

  void ComputeAllDescriptors()
  {
    // Get the vertices
    // No, no, no, the "boost::vertex_index_t" property map was never added to the graph.
    // Only property-maps that you create in the graph class template will exist in the graph.
    //  typedef typename boost::property_map<TGraph, boost::vertex_index_t>::type IndexMap;
    //  IndexMap index = get(boost::vertex_index, g);

    TComputeDescriptor descriptorComputer;
    
    typedef typename boost::graph_traits<TGraph>::vertex_iterator vertex_iter;
    using boost::vertices;
    
    std::pair<vertex_iter, vertex_iter> firstLastPair;
    for (firstLastPair = vertices(g); firstLastPair.first != firstLastPair.second; ++firstLastPair.first)
    {
      descriptorComputer.Compute(g, *firstLastPair.first);
    }
    std::cout << std::endl;
  }

private:
  TGraph g;

};


int main(int,char*[])
{
  //PatchBasedInpainting<FeatureVectorGraph> patchBasedInpainting;

  enum UserChoice {UseFeatureVectors, UseImagePatches, UseComposite};

  UserChoice userChoice = UseFeatureVectors;

  InpaintingParent* inpainting;
  if(userChoice == UseFeatureVectors)
  {
    inpainting = new PatchBasedInpainting<FeatureVectorGraph, ComputeFeatureVector >;
  }
  else if(userChoice == UseImagePatches)
  {
    inpainting = new PatchBasedInpainting<ImagePatchGraph, ComputeImagePatch >;
  }
  else if(userChoice == UseComposite)
  {
    inpainting = new PatchBasedInpainting<CompositeDescriptorGraph, ComputeFeatureVectorAndImagePatch >;
  }

  inpainting->CreateGraph();
  inpainting->ComputeAllDescriptors();

  return 0;
}

