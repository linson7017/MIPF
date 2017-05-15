#include "tubulartracking.h"
#include <vtkSmartPointer.h>
#include <vtkExtractVOI.h>
#include <vtkImageIterator.h>
#include <vtkMetaImageWriter.h>
//#include <vtkImageMathematics.h>

TubularTracking::TubularTracking() :
_outputChanged(false),
_trackedBoostGraphs(0)
{

  // Add minimum radius field (millimeters)
  _minRadiusFld =  0.5;

  // Add maximum radius field (millimeters)
  _maxRadiusFld =  1;

  // Add initial radius field (millimeters)
  _initRadiusFld =  2;

  // Add flag field whether to use initial radius
  _useInitRadiusFld = false;

  // Add number of search angles field
  _nbrSearchAnglesFld =  3;

  //! Maximum angle relative to the previous tracking step (in degrees)
  _maxAngleFld =  0.001;

  // Threshold for a single tracking step.
  _pruningThresholdFld =  1;

  // Threshold for the average score of a tracking path of
  // length _searchDepth. Should be higher than _scoreThreshold.
  _terminationThresholdFld =  20;

  // Add maximum number of tracking steps field
  _maxNbrStepsFld =  100;

  // Maximum length in millimeter to track.
  _maxLengthFld = 100;

  // Set search depth in tracking tree
  _searchDepthFld =  3;

  // Minimum distance between two branchings in millimeters.
  _minBranchingDistanceFld =  5;

  // Tracking step length factor
  _stepLengthFactorFld =  1;

  // Size of the Gaussian window used for fitting
  _windowSizeFactorFld =  3;


  // Add toggle field that enables/disables vessel branching.
  _allowBranchingFld =  false;

  // Add toggle field for growing bidirectionally.
  _growBidirectionalFld =  false;

  //! Single hypothesis or multiple hypothesis tracking.
  _useMultipleHypothesesTrackingFld =  false;

  // Add toggle field that enables/disables termination after a certain number of tracking steps.
  _toggleMaxStepsFld =  false;

  // Add toggle field that enables/disables termination after a certain tracking length.
  _toggleMaxLengthFld =  false;


  _needInvertIntensityFld = false;
  //! Clear binary image.
  _binaryImg = BinaryImageType::New();
}

TubularTracking::~TubularTracking() {
  _trackedBoostGraphs.clear();
  _binaryImg->Delete();
  _trackedPointsXMarkerList.clearList();
}

void TubularTracking::clearBinaryImage(BinaryImageType* image)
{
    int extent[6];
    image->GetExtent(extent);
    vtkImageIterator<unsigned char> it(image, extent);
    while (!it.IsAtEnd())
    {
        unsigned char* valIt = it.BeginSpan();
        unsigned char *valEnd = it.EndSpan();
        while (valIt != valEnd)
        {
            // Increment for each component
            *valIt++ = 0;
        }
        it.NextSpan();
    }
}

void TubularTracking::clearImage(InternalImageType* image)
{
    int extent[6];
    image->GetExtent(extent);
    vtkImageIterator<float> it(image, extent);
    while (!it.IsAtEnd())
    {
        float* valIt = it.BeginSpan();
        float *valEnd = it.EndSpan();
        while (valIt != valEnd)
        {
            // Increment for each component
            *valIt++ = 0;
        }
        it.NextSpan();
    }
}

//----------------------------------------------------------------------------------
// Small helper functions to clamp field values.
//----------------------------------------------------------------------------------
inline void TT_DOUBLE_MIN_CLAMP(double& df, double minVal)
{
  if (df && (df < minVal)){
    df = minVal;
  }
}
inline void TT_DOUBLE_CLAMP(double& df, double minVal, double maxVal)
{
  TT_DOUBLE_MIN_CLAMP(df, minVal);
  if (df && (df > maxVal)){
    df = maxVal ;
  }
}
inline void TT_MLINT_CLAMP(int& intf, int minVal, int maxVal)
{
  if (intf){
    if (intf < minVal){
      intf=minVal;
    }
    if (intf > maxVal){
      intf = maxVal;
    }
  }
}

void TubularTracking::SetParameterField(ParameterField& parameterField)
{
    _minRadiusFld = parameterField.minRadius();
    _maxRadiusFld = parameterField.maxRadius();
    _initRadiusFld = parameterField.initRadius();
    _useInitRadiusFld = parameterField.useInitRadius();
    _terminationThresholdFld = parameterField.terminateThresold();
    _maxAngleFld = parameterField.maxAngle();
    _windowSizeFactorFld = parameterField.windowSizeFactor();
    _stepLengthFactorFld = parameterField.stepLength();
    _needInvertIntensityFld = parameterField.needInvertIntensity();
}

void TubularTracking::SetInputImage(ImageType image)
{
    _inputImage = InternalImageType::New();
    _inputImage->DeepCopy(image);
}

void TubularTracking::handleNotification() {
    // Clamp some fields against limits or intervals.
    TT_DOUBLE_MIN_CLAMP(_minRadiusFld, 0);
    TT_DOUBLE_MIN_CLAMP(_maxRadiusFld, 0);
    TT_MLINT_CLAMP(_nbrSearchAnglesFld, 0, 20);
    TT_DOUBLE_CLAMP(_maxAngleFld, 0, 85);
    TT_DOUBLE_CLAMP(_pruningThresholdFld, -1000, 1000);
    TT_DOUBLE_CLAMP(_terminationThresholdFld, -1000, 1000);
    TT_MLINT_CLAMP(_maxNbrStepsFld, 0, 100000);
    TT_DOUBLE_CLAMP(_maxLengthFld, 0, 1000000);
    TT_MLINT_CLAMP(_searchDepthFld, 1, 10);
    TT_DOUBLE_CLAMP(_minBranchingDistanceFld, 0, 1000);
    TT_DOUBLE_CLAMP(_stepLengthFactorFld, 0.01, 5);
    TT_DOUBLE_MIN_CLAMP(_windowSizeFactorFld, 3);

    // Track vessels
    if (true) {

        //    // Do we have input data? Otherwise no tracking.
        //    PagedImage *inImg = getUpdatedInputImage(0);

        //clear the result
        _trackedBoostGraphs.clear();

        if (_needInvertIntensityFld)
        {
            invertIntensity(_inputImage);
        }

        // Do we have 2D or 3D input data
        DIM inputDimension = (_inputImage->GetDataDimension() > 2) ? _3D : _2D;

        // Resize the binary image containing a mask of the segmented vessels.
        _binaryImg->SetExtent(_inputImage->GetExtent());
        _binaryImg->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        clearBinaryImage(_binaryImg);

        // Clear previous entries in the internal XMarkerLists
        _trackedPointsXMarkerList.clearList();

        // Create a typed subimages containing blocks of indata and maskdata
        InternalImageType* imData = vtkImageData::New();
        BinaryImageType* maskData = NULL;   // When the data pointer in maskData is NULL, no mask is supplied.

        // Get world coordinates system and other properties
        MedicalImageProperties imgProps;
        imgProps.setImageProperties(_inputImage);

        // Fill binary image with already segmented data in _trackedBoostGraphs
        for (unsigned int k = 0; k < _trackedBoostGraphs.size(); ++k)
        {
            // Get a more convenient name for the current boost graph
            const boostGraph& currentGraph = _trackedBoostGraphs[k].second;

            // We need a dummy instantiation of the VesselTemplateTracker because
            // the function we need is a virtual method in this class.
            // Not a beautiful solution, but works.
            // We do this already here because the binary image showing where the
            // tracking has been is required when tracking new vessels so as to
            // avoid segmenting the same areas again.
            VesselTemplateTracker dummyTracker(imData, maskData, imgProps, inputDimension);

            // Each tubular model in the vertices in currentGraph should be voxelized.
            // The typedefs etc below are very messy, blame the boost library for this!
            // All we want to do is to loop over all the vertices in the graph.
            typedef boost::property_map<boostGraph, boost::vertex_index_t>::type indexMap;
            indexMap index = boost::get(boost::vertex_index, currentGraph);
            typedef boost::graph_traits<boostGraph>::vertex_iterator vertex_iter;
            std::pair<vertex_iter, vertex_iter> vp;
            for (vp = boost::vertices(currentGraph); vp.first != vp.second; ++vp.first) {
                unsigned int vertexIdx = index[*vp.first];
                dummyTracker.setVisitedVoxels(currentGraph[vertexIdx], *_binaryImg);
            }
        }

        double initRadius = _useInitRadiusFld
            ? _initRadiusFld
            : 0.5*(_maxRadiusFld + _minRadiusFld);

        // Loop over all input xmarkers. Each xmarker point results in a root in the ML graph.
        const int numXMarkers = _initPointsXMarkerList.size();
        for (int markerNbr = 0; markerNbr < numXMarkers; ++markerNbr)
        {

            // Get XMarker from input list
            const XMarker currentMarker = _initPointsXMarkerList.getItemAt(markerNbr);
            //check if seed points are in volume
            if (!imgProps.containsWorldIndex(Vector3(currentMarker.x(), currentMarker.y(), currentMarker.z())))
            {
                continue;
            }


            // If no direction vector is given in the XMarker, we estimate the direction from
            // the image data using the function estimateOrientation()
            Vector3 vesselDirection;
            bool userVectorAvailable = true;
            if ((currentMarker.vector3()).lengthSquared() > 1e-5) {
                vesselDirection = currentMarker.vector3();
            }
            else {
                // Estimate image structure orientation
                Vector3 vec3(currentMarker.x(), currentMarker.y(), currentMarker.z());
                vesselDirection = estimateOrientation(vec3, imgProps, *_inputImage,
                    initRadius, inputDimension);

                // No user direction available
                userVectorAvailable = false;
            }

            // Normalize vessel direction
            vesselDirection.normalize();

            // Do we need to grow bidirectionally?
            const bool growBidirectional = !userVectorAvailable || _growBidirectionalFld;

            // Create an initial VesselData structure from the first input XMarker
            VesselData firstModel;
            firstModel.setCenterPoint(currentMarker.x(), currentMarker.y(), currentMarker.z());
            firstModel.setDirection(vesselDirection[0], vesselDirection[1], vesselDirection[2]);
            firstModel.setRadius(initRadius);

            // Declare a boost graph containing the tubular models tracked from this start point
            boostGraph treeGraph;

            // Decalre a vector containing the root models, could be 1 or 2 depending on whether
            // we are tracking uni-directional or bi-directional
            std::vector<Vertex> rootVertices;

            // Add the start-model to the boost tree graph
            Vertex firstVertex = boost::add_vertex(treeGraph);
            rootVertices.push_back(firstVertex);
            treeGraph[firstVertex] = firstModel;

            // If no user vector for the tracking direction was added, we track in two directions. We also
            // do this if the user has indicated the Bidirectional tracking option.
            if (growBidirectional) {

                // Create a vessel model segment that is the same as the initial model but with reversed direction.
                VesselData secondModel;
                secondModel.setCenterPoint(currentMarker.x(), currentMarker.y(), currentMarker.z());
                secondModel.setDirection(-vesselDirection[0], -vesselDirection[1], -vesselDirection[2]);
                secondModel.setRadius(initRadius);

                // Add a new vertex to the boost graph containing the secondModel.
                // Connect this new vertex with the first vertex containing the initial model. The second vertex
                // will be seen as a non-terminated leaf that should be tracked from below.
                Vertex secondVertex = boost::add_vertex(treeGraph);
                treeGraph[secondVertex] = secondModel;
                //rootVertices.push_back(secondVertex);                   // TODO: Remove this line and the for loop below is unnecessary
                boost::add_edge(firstVertex, secondVertex, treeGraph);
            }

            // Set the maximum number of steps and the maximum length to track in millimeters.
            // If the user has defined values we use them, otherwise we set large values.
            const std::pair<int, double> maxTrackingLength(_toggleMaxStepsFld ? _maxNbrStepsFld : 30000,
                _toggleMaxLengthFld ? _maxLengthFld : 1000000.0);

            // Max image block size we are allowed to use.
            const int maxBlockSize = 200;

            // Loop over 1 or 2 root models depending on if we are tracking bi-directionally or not.
            for (unsigned int rootVertexNbr = 0; rootVertexNbr < rootVertices.size(); ++rootVertexNbr) {   // TODO: This loop can be removed!

              // Image block size in voxels. Image blocks with side of this size will be supplied
              // to the tracking algorithm.
                int blockSize = 100;

                // Number of tracked steps and tracked path length in millimeters
                std::pair<int, double> trackingLength(0, 0);

                // Initial vertex is set to the root vertex
                const Vertex rootVertex = rootVertices[rootVertexNbr];
                Vertex initVertex = rootVertex;

                bool trackingDone = false;
                while (!trackingDone)
                {
                    // Fill image area with data from input image
                    double x, y, z;
                    treeGraph[initVertex].getCenterPoint(x, y, z);
                    double currentPoint[3] = { x,y,z };
                    _getImageData(*imData, *maskData, imgProps, currentPoint, blockSize, inputDimension);

                    // Set up tracker
                    VesselTemplateTracker tracker(imData, maskData, imgProps, inputDimension);
                    tracker.setMinRadius(_minRadiusFld);
                    tracker.setMaxRadius(_maxRadiusFld);
                    tracker.setMaxSearchAngle(_maxAngleFld / 360 * 2 * 3.14159265358);
                    tracker.setTerminationThreshold(_terminationThresholdFld);
                    tracker.setMinBranchingDistance(_minBranchingDistanceFld);
                    tracker.setStepLength(_stepLengthFactorFld);
                    tracker.setWindowSizeFactor(_windowSizeFactorFld);
                    // Tracker setup for multiple hypothesis tracking
                    if (_useMultipleHypothesesTrackingFld) {
                        tracker.setSearchDepth(_searchDepthFld);
                        tracker.setNbrSearchAngles(_nbrSearchAnglesFld);
                        tracker.setPruningThreshold(_pruningThresholdFld);
                        if (_allowBranchingFld) { tracker.allowBranching(); }
                        else { tracker.disallowBranching(); }
                    }
                    // Tracker setup for single hypothesis tracking
                    else {
                        tracker.setSearchDepth(1);              // Search depth 1 for single hypothesis
                        tracker.setNbrSearchAngles(0);          // 0 search angles => 2*0+1 = 1 prediction along the tangential line
                        tracker.disallowBranching();            // Branching cannot be detected in single hypothesis mode
                        tracker.setPruningThreshold(-100000.0); // Set pruning threshold low so that the prediction is always fitted
                    }


                    // Track
                    //handleNotificationOff();
                    const int prevNbrSteps = trackingLength.first;
                    unsigned int retCode = tracker.track(treeGraph, initVertex, trackingLength, maxTrackingLength, *_binaryImg);
                    const int nbrStepsTaken = trackingLength.first - prevNbrSteps;
                    //handleNotificationOn();

                    //std::cout << "Terminatation reason: " << tracker.getTerminationMessage(retCode) << std::endl;

                    // Have taken the maximum number of steps or tracked the maximum length, aborting
                    if ((MAX_NBR_STEPS_TAKEN == retCode) || (MAX_LENGTH_REACHED == retCode)) {
                        break;
                    }

                    // The image data block supplied to the tracking algorithm is not big enough to carry out the tracking,
                    // try increasing the block size. If not allowed, abort.
                    if (INSUFFICIENT_IMAGE_DATA == retCode) {
                        blockSize += 50;
                        if (blockSize > maxBlockSize) {
                            //ML_PRINT_WARNING("TubularTracking", ML_BAD_PARAMETER, "Block size for tracking too big.");
                            break;
                        }
                    }

                    // If tracking is terminating because we reach the border in just a few steps (<10)
                    // we should increase the image block size.
                    if ((TOO_CLOSE_TO_BORDER == retCode) && (nbrStepsTaken < 10) && (blockSize + 50 <= maxBlockSize)) {
                        blockSize += 50;
                    }

                    // Search the resulting _treeGraph for leafs that are no terminators, that is, branches
                    // where the tracking was terminated because we hit the allocated image border. We need
                    // to extract these leafs retrack from these points with an updated image block.
                    // The ActiveLeafRecorder class is also used within the VesselTracker class, and this code
                    // was copied from there. The VertexScorePairs is a vertex index and score pair defined
                    // in VesselGraph.h. Here we do not need the score or the leafDepth, just the vertex indexes.
                    VertexScorePairs leafs;
                    int leafDepth = 0;
                    boost::breadth_first_search(treeGraph, rootVertex, boost::visitor(ActiveLeafRecorder(&leafs, &leafDepth)));
                    if (leafs.size() > 0) {
                        initVertex = leafs[0].first;
                    }
                    else {
                        trackingDone = true;
                    }
                } // end-while !trackingDone from the current input XMarker
            } // end-for over roots (for bi-directional tracking)

            // Add XMarker and tracked boostGraph to the _trackedBoostGraphs vector
            _trackedBoostGraphs.push_back(std::make_pair(currentMarker, treeGraph));

        }  // end-for over input XMarkers

        // Set flag that output has changed.
        _outputChanged = true;


        // ==================
        // DONE WITH TRACKING
        // ==================

        // Convert all boost graphs in _trackedBoostGraphs to output XMarkers and a Vessel Graph
        //for (unsigned int k = 0; k < _trackedBoostGraphs.size(); ++k)
        //{

        //    // Get a more convenient name for the current boost graph
        //    const boostGraph& currentGraph = _trackedBoostGraphs[k].second;

        //    // Convert tracking graph, which is a boost graph, to an ML Graph.
        //    // Copy points from boost graph to XMarkerList
        //    typedef boost::property_map<boostGraph, boost::vertex_index_t>::type indexMap;
        //    indexMap index = boost::get(boost::vertex_index, currentGraph);

        //    typedef boost::graph_traits<boostGraph>::vertex_iterator vertex_iter;
        //    std::pair<vertex_iter, vertex_iter> vp;
        //    for (vp = boost::vertices(currentGraph); vp.first != vp.second; ++vp.first) {
        //        size_t vertexIdx = index[*vp.first];
        //        double px, py, pz, vx, vy, vz;
        //        currentGraph[vertexIdx].getCenterPoint(px, py, pz);
        //        currentGraph[vertexIdx].getDirection(vx, vy, vz);
        //        const double radius = currentGraph[vertexIdx].getRadius();
        //        vx *= radius; vy *= radius; vz *= radius; // Scale direction with the radius
        //        _trackedPointsXMarkerList.appendItem(XMarker(px, py, pz, 0, 0, 0, vx, vy, vz));
        //        //std::cout<< " Radius: " << currentGraph[vertexIdx].getRadius() << ". Score: " << currentGraph[vertexIdx].getScore() << std::endl;
        //    }

        //    boost::graph_traits<boostGraph>::edge_iterator ei, ei_end;
        //    for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
        //        std::cout << "(" << index[source(*ei, g)] << "," << index[target(*ei, g)] << ") ";
        //}
    }
}


Vector3 TubularTracking::estimateOrientation(const Vector3& pos, const MedicalImageProperties& imgProps, InternalImageType& virtDataVol,
                                             double radius, DIM trackDim)
{

  // Calculate Hessian for 3 different scales [hme: where? I only see one scale...]

  // Get voxel size
  const Vector3 voxelSize = imgProps.getVoxelSize();

  // Get closest voxel position
  double closestVoxel_x = pos[0];
  double closestVoxel_y = pos[1];
  double closestVoxel_z = pos[2];
  const Vector3 vc = imgProps.mapWorldToVoxel(Vector3(closestVoxel_x, closestVoxel_y, closestVoxel_z));
  closestVoxel_x = vc[0];
  closestVoxel_y = vc[1];
  closestVoxel_z = vc[2];

  closestVoxel_x = floor(closestVoxel_x);
  closestVoxel_y = floor(closestVoxel_y);
  closestVoxel_z = floor(closestVoxel_z);

  // Find size of image so that we don't read outside the border
 // ImageVector imExt = virtDataVol.getVirtualVolume().getExtent();
  int imExt[6];
  virtDataVol.GetExtent(imExt);

  // For each scale
  std::valarray<double> kernelx;
  std::valarray<double> kernely;
  std::valarray<double> kernelz;

  // Get sigma in voxels
  const double sigma_x = radius/voxelSize[0];
  const double sigma_y = radius/voxelSize[1];
  const double sigma_z = (trackDim == _2D)? 0.0 : radius/voxelSize[2];

  // Array for keeping 2D (indexes 0,1,2) or 3D (all indexes) Hessian matrix.
  double hessian[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  if(trackDim == _2D) {
    // Calculate Hessian components
    for(unsigned int hessianComponent=0; hessianComponent<3; ++hessianComponent) {

      get1DGaussKernel(sigma_x, kernelx, hessian2DOrder[hessianComponent][0]);
      get1DGaussKernel(sigma_y, kernely, hessian2DOrder[hessianComponent][1]);
      const int halfSize_x = ((int)kernelx.size() - 1)/2;
      const int halfSize_y = ((int)kernely.size() - 1)/2;

      for(int y=-halfSize_y; y<=halfSize_y; ++y) {
        int x = -halfSize_x;
        //virtDataVol.setCursorPosition(ImageVector(closestVoxel_x + x, closestVoxel_y + y, closestVoxel_z, 0, 0, 0));
        for( ; x<=halfSize_x; ++x) {
          // Check first that position is within image, otherwise we have a crash in .getCursorVoxel()
            const float imVal = ((closestVoxel_x + x >= imExt[0]) && (closestVoxel_x + x < imExt[1]) &&
                               (closestVoxel_y + y >= imExt[2]) && (closestVoxel_y + y < imExt[3])) ?
                             virtDataVol.GetScalarComponentAsDouble(closestVoxel_x + x, closestVoxel_y + y, closestVoxel_z, 0) : 0.0; //songling
             hessian[hessianComponent] += kernelx[halfSize_x + x]*kernely[halfSize_y + y]*imVal;
          //virtDataVol.moveCursorX();
        } // end-for over x
      } // end-for over y
    } // end-for over 3 Hessian components



    // Eigen-decomposition of Hessian matrix, want eigenvector associated with
    // the eigenvalue with the smallest magnitude. This vector points out the
    // direction in which the image varies the least. On a tube-shaped object,
    // this should be the tube direction.
    SymmetricMatrix H(2);
    H(1,1) = hessian[0]; H(1,2) = hessian[1];
    H(2,2) = hessian[2];
    DiagonalMatrix D(2);
    Matrix E(2,2);
    eigenvalues(H,D,E);

    // The eigenvalues are sorted so that D(1)<D(2). Find the eigenvalue with
    // the smallest magnitude and return the associated eigenvector.
    if( std::abs(D(1)) < std::abs(D(2))) {
      return Vector3(E(1,1), E(2,1), 0.0);
    }
    else { // Magnitude of D(2) is smaller than magnitude of D(1), return eigenvector 2.
      return Vector3(E(1,2), E(2,2), 0.0);
    }

    // mlDebug("Hessian matrix: " << hessian[0] << " " << hessian[1] << " " << hessian[2]);
    // mlDebug("l1: " << D(1,1) << ". l2: " << D(2,2));
    // mlDebug("e1: ("<<E(1,1)<<", " << E(2,1) << ")");
    // mlDebug("e2: ("<<E(1,2)<<", " << E(2,2) << ")");



  } //end-if trackDim == _2D
  else if (trackDim == _3D) {

    // Calculate Hessian components
    for(unsigned int hessianComponent=0; hessianComponent<6; ++hessianComponent) {

      get1DGaussKernel(sigma_x, kernelx, hessian3DOrder[hessianComponent][0]);
      get1DGaussKernel(sigma_y, kernely, hessian3DOrder[hessianComponent][1]);
      get1DGaussKernel(sigma_z, kernelz, hessian3DOrder[hessianComponent][2]);
      const int halfSize_x = ((int)kernelx.size() - 1)/2;
      const int halfSize_y = ((int)kernely.size() - 1)/2;
      const int halfSize_z = ((int)kernelz.size() - 1)/2;

      for(int z=-halfSize_z; z<=halfSize_z; ++z) {
        for(int y=-halfSize_y; y<=halfSize_y; ++y) {
          int x = -halfSize_x;
          //virtDataVol.setCursorPosition(ImageVector(closestVoxel_x + x, closestVoxel_y + y, closestVoxel_z + z, 0, 0, 0));
          const double yzKernelValue = kernely[y + halfSize_y]*kernelz[z + halfSize_z];
          for( ; x<=halfSize_x; ++x) {
            // Check first that position is within image, otherwise we have a crash in .getCursorVoxel()
                const float imVal = ((closestVoxel_x + x >= imExt[0]) && (closestVoxel_x + x < imExt[1]) &&
                                        (closestVoxel_y + y >= imExt[2]) && (closestVoxel_y + y < imExt[3]) &&
                                        (closestVoxel_z + z >= imExt[4]) && (closestVoxel_z + z < imExt[5])) ?
                                    virtDataVol.GetScalarComponentAsDouble(closestVoxel_x + x, closestVoxel_y + y, closestVoxel_z + z, 0) : 0.0;
                hessian[hessianComponent] += kernelx[x + halfSize_x]*yzKernelValue*imVal;
            //virtDataVol.moveCursorX();
          } // end-for over x
        } // end-for over y
      } // end-for over z
    } // end-for over 6 Hessian components

    // Eigen-decomposition of Hessian matrix, want eigenvector associated with
    // the eigenvalue with the smallest magnitude. This vector points out the
    // direction in which the image varies the least. On a tube-shaped object,
    // this should be the tube direction.
    SymmetricMatrix H(3);
    H(1,1) = hessian[0]; H(1,2) = hessian[1]; H(1,3) = hessian[2];
    H(2,2) = hessian[3]; H(2,3) = hessian[4];
    H(3,3) = hessian[5];
    DiagonalMatrix D(3);
    Matrix E(3,3);
    eigenvalues(H,D,E);

    // The eigenvalues are sorted so that D(1)<D(2)<D(3). Find the eigenvalue with
    // the smallest magnitude and return the associated eigenvector.
    if( (std::abs(D(1)) < std::abs(D(2))) && (std::abs(D(1)) < std::abs(D(3))) ) {
      return Vector3(E(1,1), E(2,1), E(3,1));
    }
    else if( (std::abs(D(2)) < std::abs(D(1))) && (std::abs(D(2)) < std::abs(D(3))) ) {
      return Vector3(E(1,2), E(2,2), E(3,2));
    }
    else {
      return Vector3(E(1,3), E(2,3), E(3,3));
    }
  } //end-if trackDim == _3D

  return Vector3(0,0,0);
}


void TubularTracking::_getImageData(InternalImageType &imData, BinaryImageType &maskData, const MedicalImageProperties &imgProps, double worldCenterPoint[3], int blockSize, DIM trackDim)
{

  // Free previously allocated image data
    try {
        imData.Initialize();
    }
    catch (int x) {}

  // Convert the centerPoint to the voxel coordinate system
  double voxelCenterPoint[3] = {worldCenterPoint[0], worldCenterPoint[1], worldCenterPoint[2]};
  const Vector3 vc = imgProps.mapWorldToVoxel(Vector3(voxelCenterPoint[0], voxelCenterPoint[1], voxelCenterPoint[2]));
  voxelCenterPoint[0] = vc[0];
  voxelCenterPoint[1] = vc[1];
  voxelCenterPoint[2] = vc[2];

  // Round up and down respectively to get voxel index
  int voxelBox_v1[3];
  int voxelBox_v2[3];
  voxelBox_v1[0] = (int)floor(voxelCenterPoint[0] - blockSize/2);
  voxelBox_v1[1] = (int)floor(voxelCenterPoint[1] - blockSize/2);
  voxelBox_v1[2] = (trackDim==_2D) ? (int)voxelCenterPoint[2] : (int)floor(voxelCenterPoint[2] - blockSize/2);
  voxelBox_v2[0] = (int)floor(voxelCenterPoint[0] + blockSize/2 + 1);
  voxelBox_v2[1] = (int)floor(voxelCenterPoint[1] + blockSize/2 + 1);
  voxelBox_v2[2] = (trackDim==_2D) ? (int)voxelCenterPoint[2] : (int)floor(voxelCenterPoint[2] + blockSize/2 + 1);

  // Get suitable box for tracking
  // MeVisLab has its voxel origin at the upper left corner of the upper left voxel. Hence,
  // we need to apply a floor() operation to find integer indexes into the array holding the image.
  int extent[] = {voxelBox_v1[0],voxelBox_v2[0],voxelBox_v1[1],voxelBox_v2[1],voxelBox_v1[2],voxelBox_v2[2]};

  imData.SetExtent(extent);
  imData.AllocateScalars(VTK_FLOAT, 1);
  clearImage(&imData);
  // Copy image data
  try{
      copySubImage(&imData, imgProps.getImage());
  }
  catch(int x){};
  //vtkMetaImageWriter* writer = vtkMetaImageWriter::New();
  //writer->SetInputData(&imData);
 // writer->SetRAWFileName("D:/imdata/imdata.raw");
 // writer->SetFileName("D:/imdata/imdata.mhd");
 // writer->Write();

}


void TubularTracking::copySubImage(InternalImageType* image, InternalImageType* srcImage)
{
    if (!srcImage)
    {
        return;
    }
    int extentOutput[6], extentSrc[6];
    image->GetExtent(extentOutput);
    srcImage->GetExtent(extentSrc);
    int extentIntersect[6];
    extentIntersect[0] = max(extentOutput[0], extentSrc[0]);
    extentIntersect[1] = min(extentOutput[1], extentSrc[1]);
    extentIntersect[2] = max(extentOutput[2], extentSrc[2]);
    extentIntersect[3] = min(extentOutput[3], extentSrc[3]);
    extentIntersect[4] = max(extentOutput[4], extentSrc[4]);
    extentIntersect[5] = min(extentOutput[5], extentSrc[5]);

    for (int i=extentIntersect[0];i<=extentIntersect[1];i++)
    {
        for (int j = extentIntersect[2]; j <= extentIntersect[3]; j++)
        {
            for (int k = extentIntersect[4]; k <= extentIntersect[5]; k++)
            {
                image->SetScalarComponentFromDouble(i, j, k, 0, srcImage->GetScalarComponentAsDouble(i, j, k, 0));
            }
        }
    }
}

void TubularTracking::invertIntensity(InternalImageType* image)
{
    double* range = image->GetScalarRange();
    int* extent = image->GetExtent();
    double *ptr = static_cast<double *>(image->GetScalarPointer(0, 0, 0));
    for (int z = 4; z <= extent[5]; z++)
    {
        for (int y = 2; y <= extent[3]; y++)
        {
            for (int x = extent[0]; x <= extent[1]; x++)
            {
                double value = image->GetScalarComponentAsDouble(x, y, z, 0);
                image->SetScalarComponentFromDouble(x, y, z, 0, range[1] - value);
            }
        }
    }
}


void TubularTracking::GetOutput(std::vector< std::vector<Vector3> >& result)
{
    result.clear();
    for (unsigned int k = 0; k < _trackedBoostGraphs.size(); ++k)
    {
        std::vector<Vector3> list;
        const boostGraph& currentGraph = _trackedBoostGraphs[k].second;
        typedef boost::property_map<boostGraph, boost::vertex_index_t>::type indexMap;
        indexMap index = boost::get(boost::vertex_index, currentGraph);
        typedef boost::graph_traits<boostGraph>::vertex_iterator vertex_iter;
        std::pair<vertex_iter, vertex_iter> vp;
        for (vp = boost::vertices(currentGraph); vp.first != vp.second; ++vp.first)
        {
            size_t vertexIdx = index[*vp.first];
            double px, py, pz, vx, vy, vz;
            currentGraph[vertexIdx].getCenterPoint(px, py, pz);
            currentGraph[vertexIdx].getDirection(vx, vy, vz);
            const double radius = currentGraph[vertexIdx].getRadius();
            vx *= radius; vy *= radius; vz *= radius;
            list.push_back(Vector3(px, py, pz));
        }
        result.push_back(list);
    }
}
