#include "VesselTracker.h"

//! Constructor
VesselTracker::VesselTracker(InternalImageType* imData, BinaryImageType* maskData,
                             const MedicalImageProperties& imProps, DIM TRACKDIM) :
_imData(imData),
_maskData(maskData),
_imProps(imProps),
_TRACKDIM(TRACKDIM),
_branchGraph(0),
_branchingState(false),
_pruningThreshold(1),
_terminationThreshold(3),
_searchDepth(3),
_allowBranching(true),
_nbrAngles(7),
_maxSearchAngle(70.0/360.0*2.0*3.14159265358f),
_maxBranchRegionAngle(10.0/360.0*2.0*3.14159265358f),
_maxRadius(1),
_minRadius(0.5),
_stepLengthFactor(1),
_minBranchingDistance(5)
{

  // Initialize voxel size
  Vector3 voxelSize = _imProps.getVoxelSize();
  _worldVoxelSize[0] = voxelSize[0];
  _worldVoxelSize[1] = voxelSize[1];
  _worldVoxelSize[2] = voxelSize[2];

  // Find the largest voxel size
  _largestVoxelSize = (_worldVoxelSize[1] > _worldVoxelSize[0]) ? _worldVoxelSize[1] : _worldVoxelSize[0];
  _smallestVoxelSize = (_worldVoxelSize[1] < _worldVoxelSize[0]) ? _worldVoxelSize[1] : _worldVoxelSize[0];
  if( (TRACKDIM == _3D) && (_worldVoxelSize[2] > _largestVoxelSize)) {
    _largestVoxelSize = _worldVoxelSize[2];
  }
  if( (TRACKDIM == _3D) && (_worldVoxelSize[2] < _smallestVoxelSize)) {
    _smallestVoxelSize = _worldVoxelSize[2];
  }

}

// --------------------------------------------------------------------------------------------------
//! Destructor, free allocated image area.
// --------------------------------------------------------------------------------------------------
VesselTracker::~VesselTracker()
{
  // Commented out 13.03.2008  _imData.free();
  _branchGraph.clear();
}

// --------------------------------------------------------------------------------------------------
//! Set minimum radius. The smallest vessel diameter allowed is 1 voxel, so the radius
//! must be at least 0.5 voxels.
// --------------------------------------------------------------------------------------------------
void VesselTracker::setMinRadius(double minRadius)
{
  // Compare the user-given min radius with the voxel sizes.
  minRadius = (minRadius > _worldVoxelSize[0]/2)? minRadius : _worldVoxelSize[0]/2;
  minRadius = (minRadius > _worldVoxelSize[1]/2)? minRadius : _worldVoxelSize[1]/2;
  if(_TRACKDIM == _3D) {
    minRadius = (minRadius > _worldVoxelSize[2]/2)? minRadius : _worldVoxelSize[2]/2;
  }

  // Set the minimum radius.
  _minRadius = minRadius;
}

const char* VesselTracker::getTerminationMessage(unsigned int retCode)
{
  switch(retCode) {
    case 0: return "Vessel contrast too low - normal tracking termination.";
    case 1: return "Initial model does not fit image data.";
    case 2: return "Maximum number of tracking steps reached.";
    case 3: return "Too close to image data border.";
    case 4: return "Not enough image data supplied.";
    case 5: return "Maximum tracking length reached.";
    case 6: return "Outside mask.";
    default: return "Unknown termination code.";
  }
}

// --------------------------------------------------------------------------------------------------
//! Return codes:
//! 0 Terminated OK
//! 1 No fit for first model found
//! 2 Max number of steps reached
//! 3 Too close to data border
//! 4 Not enough image data supplied
//! 5 Max tracking length reached
//! 6 Outside mask
// --------------------------------------------------------------------------------------------------
unsigned int VesselTracker::track(boostGraph& treeGraph,
                                  Vertex initVertex,
                                  std::pair<int,double>& trackingLength,               // <number of steps taken, length tracked in millimeters>
                                  const std::pair<int,double>& maxTrackingLength,      // Max values for the above quantities
                                  BinaryImageType& binaryImg)
{

  // Check that initial point is within user-defined mask
  if(!withinMask(treeGraph[initVertex])) {
    treeGraph[initVertex].setTerminator();
    return OUTSIDE_MASK;
  }

  // Check that the image data supplied is sufficient
  if(!borderCheck(treeGraph[initVertex])) {
    return INSUFFICIENT_IMAGE_DATA;
  }

  // Make a fit of the first model.
  if(!treeGraph[initVertex].isFitted()) {
    _fitModel(treeGraph[initVertex],treeGraph[initVertex]);
    _calcScore(treeGraph[initVertex]);
    setVisitedVoxels(treeGraph[initVertex], binaryImg);
    // If the first model doesn't fit, we abort immediately.
    if(treeGraph[initVertex].getScore() < _pruningThreshold) {
      treeGraph[initVertex].setTerminator();
      return BAD_INITIAL_MODEL;
    }
  }

  // A list for vertexes to be tracked from. A vertex is added
  // to the list when a branching is detected. Add the initalModel-
  // vertex to this list.
  // Note: waitingList contains indexes into the treeGraph.
  std::deque<Vertex> waitingList(0);
  waitingList.push_back(initVertex);

  // Repeat as long as there are elements (=untracked branches) in the waitingList.
  do {

    // Get the next vertex in the waiting list.
    Vertex currentTreeVertex = waitingList.front();
    waitingList.pop_front();

    // The branch graph is a temporary boost graph for storing
    // all the steps that are tried out for the current branch.
    // Add the current tree vertex as the first vertex to this graph.
    _branchGraph.clear();
    Vertex currentBranchVertex = boost::add_vertex(_branchGraph);
    _branchGraph[currentBranchVertex] = treeGraph[currentTreeVertex];

    // Build initial tracking tree
    VertexScorePairs trialLeafs;
    for(int i=0; i<_searchDepth-1; ++i) {
      _extendGraph(currentBranchVertex, trialLeafs);
    }

    // Start tracking
    while(true) {

      // Check if number of taken steps exceeds user-given maximum
      if(trackingLength.first > maxTrackingLength.first) {
        return MAX_NBR_STEPS_TAKEN;
      }

      // Check if tracking length exceeds user-given maximum
      if(trackingLength.second > maxTrackingLength.second) {
        return MAX_LENGTH_REACHED;
      }

      // Check that point is within user-defined mask
      if(!withinMask(treeGraph[currentTreeVertex])) {
        treeGraph[currentTreeVertex].setTerminator();
        return OUTSIDE_MASK;
      }

      // Check if the currentTreeVertex is too close to the image border.
      // In such case we abort the tracking of the current branch. The currentTreeVertex
      // will (still) be marked as non-terminator if we are too close to the border
      // so that tracking can be resumed from this point with a new data block.
      if(!borderCheck(treeGraph[currentTreeVertex])) {
        return TOO_CLOSE_TO_BORDER;
      }

      // Extend the tracking tree one step.
      // Note: trialLeafs is *not* sorted.
      _extendGraph(currentBranchVertex, trialLeafs);

      // Find paths to all downstream vertexes connected to the current vertex.
      std::map<Vertex, std::vector<Vertex> > vertexPaths;
      boost::breadth_first_search(_branchGraph, currentBranchVertex, boost::visitor(FindVertexPaths(&vertexPaths, currentBranchVertex)));


      // The tracking of the current branch is terminated when no new valid leafs are found.
      // If we take the last steps in branchGraph towards the best leaf.
      if(trialLeafs.empty()) {

        // If we are using a search depth of 1 there are nothing in the search tree, just terminate.
        if(_searchDepth == 1) {
          treeGraph[currentTreeVertex].setTerminator();
          break;
        }

        // Calculate a score for each leaf-path
        double highScore = -1e10;
        Vertex bestLeafVertex = 0;
        bool bestLeafFound = false;
        bool leafFoundDebug = false;
        std::map<Vertex, std::vector<Vertex> >::iterator it = vertexPaths.begin();
        for (; it != vertexPaths.end(); ++it) {

          // Is vertex a leaf?
          if((int)boost::out_degree(it->first, _branchGraph) == 0) {
            leafFoundDebug = true;
            // Calculate average score for path
            double pathScore = 0.0;
            for(unsigned int k=1; k< (it->second).size(); ++k) {
              pathScore += _branchGraph[it->first].getScore();
            }
            pathScore /= ((it->second).size()-1);

            // Compare score with previous highscore
            if(pathScore > highScore) {
              highScore = pathScore;
              bestLeafVertex = it->first;
              bestLeafFound = true;
            }
          }
        }

        // The case where no best leaf was found has been observed. The reason is unknown, should be investigated
        // The solution for now is simply not to add anything.
        if(bestLeafFound) {
          // Get the path to the best leaf
          std::vector<Vertex> bestLeafPath = vertexPaths[bestLeafVertex];

          // Add the path to treeGraph, taking into account that we should not exceed the path length constraints.
          for(unsigned int k=1; k<bestLeafPath.size(); ++k) {
            // Ignore the first vertex in the path, it is the one we are standing on, it has already been added.
            currentBranchVertex = bestLeafPath[k];

            // Add to treeGraph
            currentTreeVertex = _addTreeGraphVertex(treeGraph, trackingLength, binaryImg, currentTreeVertex, _branchGraph[currentBranchVertex]);

            // Check that point is within user-defined mask
            if(!withinMask(treeGraph[currentTreeVertex])) {
              treeGraph[currentTreeVertex].setTerminator();
              return OUTSIDE_MASK;
            }

            // Check if number of taken steps exceeds user-given maximum
            if(trackingLength.first > maxTrackingLength.first) {
              treeGraph[currentTreeVertex].setTerminator();
              return MAX_NBR_STEPS_TAKEN;
            }

            // Check if tracking length exceeds user-given maximum
            if(trackingLength.second > maxTrackingLength.second) {
              treeGraph[currentTreeVertex].setTerminator();
              return MAX_LENGTH_REACHED;
            }
          }
        }
        // OK, terminate tracking at the current point
        treeGraph[currentTreeVertex].setTerminator();
        break;
      }



      // Check for branching. branchLeafs will contain the possible branching leafs,
      // sorted so that the leaf with highest score is first (i.e., branchLeafs[0]).
      // In case no branching is detected, branchLeafs contains a single leaf (the
      // best one). In case a branching is detected, branchLeafs will contain the
      // two best leafs in each branch.
      std::vector<Vertex> branchLeafs;
      bool branchingDetected = _detectBranching(branchLeafs, trialLeafs);

      // Get path to best leaf
      std::vector<Vertex> bestLeafPath = vertexPaths[branchLeafs[0]];

      // If a branching is detected we first investigate if the first steps of the paths to the detected
      // leafs coincide. If the paths to the leafs differ we take care of the branching immediately
      // but if the paths has a common beginning we take care of the branching later. If the first
      // steps differ, we add all detected nodes in the branch graph to the tree graph and terminate
      // the tracking of the current branch. Tracking of the two new branches will then be carried
      // out in a new branch-tracking loops.
      if(branchingDetected && _allowBranching && !_branchGraph[currentBranchVertex].isInBranchingRegion())
      {
        // Get path to the leaf in the second branch.
        std::vector<Vertex> bestLeafPath2 = vertexPaths[branchLeafs[1]];
        if ( bestLeafPath[1] != bestLeafPath2[1] ) {

          // Add branching paths to the tracking tree
          Vertex treeVertex1 = currentTreeVertex;
          Vertex treeVertex2 = currentTreeVertex;
          for(int step=1; step<=_searchDepth; ++step) {

            // Get vertexes on the paths to the branch leafs.
            Vertex branchVertex1 = bestLeafPath[step];
            Vertex branchVertex2 = bestLeafPath2[step];

            // Indicate that the vertexes are in a branching region
            _branchGraph[branchVertex1].setBranchingState(true);
            _branchGraph[branchVertex2].setBranchingState(true);

            // Add vertexes to tree graph
            treeVertex1 = _addTreeGraphVertex(treeGraph, trackingLength, binaryImg, treeVertex1, _branchGraph[branchVertex1]);
            treeVertex2 = _addTreeGraphVertex(treeGraph, trackingLength, binaryImg, treeVertex2, _branchGraph[branchVertex2]);
          }

          // Add the tree leafs to the waiting list
          waitingList.push_back(treeVertex1);
          waitingList.push_back(treeVertex2);

          // Break tracking loop and start over with a new vertex from the waiting list.
          break;
        }
      }

      // If no branching was detected, take a step along the current branch towards the most promising leaf.
      // Get first vertex on the path to this best leaf.
      currentBranchVertex = bestLeafPath[1];
      currentTreeVertex = _addTreeGraphVertex(treeGraph, trackingLength, binaryImg, currentTreeVertex, _branchGraph[currentBranchVertex]);


    } // end tracking the current branch
  } while(!waitingList.empty());

  return NORMAL_TERMINATION;
}



Vertex VesselTracker::_addTreeGraphVertex(boostGraph& treeGraph,
                                          std::pair<int,double>& trackingLength,
                                          BinaryImageType& binaryImg,
                                          const Vertex& currentTreeVertex,
                                          const VesselData& vd)
{
  // Add new vertex
  Vertex newTreeVertex = boost::add_vertex(treeGraph);

  // Copy model contents
  treeGraph[newTreeVertex] = vd;

  // Add an edge connecting the new vertex with the current vertex
  boost::add_edge(currentTreeVertex, newTreeVertex, treeGraph);

  // Add one step to the tracking length
  ++trackingLength.first;       // Number of steps taken
  trackingLength.second += getStepLength(vd.getRadius());

  // Mark voxels in the binary image.
  setVisitedVoxels(vd, binaryImg);

  return newTreeVertex;
}

// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
void VesselTracker::_extendGraph(const Vertex& rootVertex, VertexScorePairs& trialLeafs)
{

  /*
  //               Before _extendGraph()     After _extendGraph()
  //
  //   Root vertex         *                        *
  //                     /   \                    /   \
  //   Current leafs    x     x                  x     x
  //                                            / \     \
  //      New leafs                            n   n     n
  */


  // TODO: REPLACE THIS WITH THE PATH RECORDER, THEN MORE ELABORATE PATH EVALUATIONS SCHEMES CAN BE IMPLEMENTED
  // Find all leafs beneath the rootVertex in the graph.
  // A breadth first search is used, could have used a depth first
  // search as well. The leaf vertexes and the sum of the scores along the
  // edges from the current vertex to the leaf vertex are stored as a
  // std::pair structure in the predefined variable type VertexScorePair.
  VertexScorePairs leafs;
  int leafDepth = 0;
  boost::breadth_first_search(_branchGraph,
    rootVertex,
    boost::visitor(ActiveLeafRecorder(&leafs, &leafDepth)));


  // The leafDepth is set to -1 if graph inconsitencies are discovered in the search
  // Should use another termination here, assert()?
  if(leafDepth == -1) {
    //ML_PRINT_ERROR("VesselTracker::_extendGraph()", ML_PROGRAMMING_ERROR, "Leafs don't have the same depth!");
  }

  // Next, from each leaf, find the best template matches and extend the graph
  // First, clear the vector where all new leafs surviving the pruning conditions are stored
  trialLeafs.clear();

  // Create vector for storing the leafs from which tracking will continue, i.e., the best leafs.
  std::vector<VesselData> bestLeafs;

  // Loop over the leafs
  VertexScorePairs::iterator itLeaf;
  for(itLeaf = leafs.begin(); itLeaf != leafs.end(); ++itLeaf)
  {

    // Separate the vertex-score pair
    Vertex leafVertex = (*itLeaf).first;
    double leafSumScore = (*itLeaf).second;

    // Produce guesses from the vessel template in this leaf
    std::vector<VesselData> guesses;
    _generateGuesses(guesses, _branchGraph[leafVertex]);

    // Find the best guesses
    std::vector<VesselData> bestGuesses;
    _analyzeScores(bestGuesses, guesses);

    // If (no score function maxima are found) OR (the scores of the maxima are too low)
    // the tracking is not continued along this branch. This is indicated
    // by setting terminator to true for the current leaf.
    if( bestGuesses.empty() ||  (bestGuesses[0].getScore() < _pruningThreshold)) {
      _branchGraph[leafVertex].setTerminator();
    }
    // Otherwise, process the best guesses further and add them to the branch tree.
    else
    {

      // If no leafs are added due to too low average scores along the paths
      // the current leaf should be marked as a terminator. This variable
      // indicates if any new leafs are added.
      bool newLeafAdded = false;

      // Make a full fit of the best extension guesses from the current leaf. We only
      // bother to fit the guesses that survive the score threshold.
      unsigned int nbrBestGuesses = bestGuesses.size();
      unsigned int nbrFittedGuesses = 0;
      while((nbrFittedGuesses < nbrBestGuesses) && (bestGuesses[nbrFittedGuesses].getScore() > _pruningThreshold))
      {
        _fitModel(bestGuesses[nbrFittedGuesses], _branchGraph[leafVertex]);
        ++nbrFittedGuesses;
      }
      // Throw away the guesses that do not exceed the threshold
      bestGuesses.resize(nbrFittedGuesses);

      // Calculate distance between all fitted guesses, it is likely that they lie
      // closely spaced after the fit. Set the terminator flag for the guesses that
      // are redundant.
      unsigned int i = 0;
      unsigned int j = 0;
      if(nbrFittedGuesses > 1)
      {

        for(i = 0; i<nbrFittedGuesses-1; ++i)
        {
          // Get center point for guess with index i
          double ix, iy, iz;
          bestGuesses[i].getCenterPoint(ix,iy,iz);
          for(j = i+1; j<nbrFittedGuesses; ++j)
          {
            // Get center point for guess with index i
            double jx, jy, jz;
            bestGuesses[j].getCenterPoint(jx,jy,jz);
            // Calculate distance in millimeters
            double d = sqrt((ix-jx)*(ix-jx) + (iy-jy)*(iy-jy) + (iz-jz)*(iz-jz));
            // If distance less than 1 voxel indicate that the guess with
            // lowest score should be deleted.
            if(d < _smallestVoxelSize)
            {
              if(bestGuesses[j].getScore() > bestGuesses[i].getScore())
              {
                bestGuesses[i].setTerminator();
              }
              else
              {
                bestGuesses[j].setTerminator();
              }
            } // end if d<0.5
          } // end loop over j
        } // end loop over i
      }

      // Loop over the fitted guesses and add the one fulfilling the conditions.
      for(i=0; i<nbrFittedGuesses; ++i)
      {
        // Add guess only if it was not found redundant above.
        if(!bestGuesses[i].isTerminator())
        {

          // Calculate mean score for the path including the current maxima
          double meanScore = (leafSumScore + bestGuesses[i].getScore()) / (leafDepth + 1);

          // Add leaf only if the average score of the entire path is above
          // the user given threshold. Moreover, we have to check the step threshold again
          // as the score may have changed in the fitting procedure. A bad guess may sometimes
          // have a high score before the fit because it has inherited good parameters from the previous
          // step, but when actually fitted to the image data in the _fitModel() function, the score
          // will be adjusted downwards. We also have to check that the model is within the user-given mask.
          if( (meanScore > _terminationThreshold) && ( bestGuesses[i].getScore() > _pruningThreshold))
          {
              // OK, all conditions fulfilled, the guess is accepted.

              // Calculate distance from current leaf to new guess and update
              // the distance to the starting point or previous junction for the guess.
              double xc, yc, zc;
              double xn, yn, zn;
              _branchGraph[leafVertex].getCenterPoint(xc, yc, zc);
              bestGuesses[i].getCenterPoint(xn, yn, zn);
              double distance = sqrt((xc-xn)*(xc-xn) + (yc-yn)*(yc-yn) + (zc-zn)*(zc-zn));
              bestGuesses[i].setDistance(_branchGraph[leafVertex].getDistance() + distance);

              // If have passed a junction/branching, special conditions apply for the tracking.
              // Here it is checked if we have tracked sufficiently far from the branching location,
              // where sufficiently is determined by the user-given value in  _minBranchingDistance.
              // If so, the normal tracking condition where a search for junctions is active is enabled.
              if(bestGuesses[i].isInBranchingRegion() && (bestGuesses[i].getDistance() > _minBranchingDistance))
              {
                bestGuesses[i].setBranchingState(false);
              }

              // Add vertex to the _branchGraph
              Vertex newVertex = boost::add_vertex(_branchGraph);
              _branchGraph[newVertex] = bestGuesses[i];

              // Add edge between the leaf vertex and the new vertex
              std::pair<Edge, bool> newEdge = boost::add_edge(leafVertex,newVertex, _branchGraph);

              // Add new vertex to the trialLeafs structure
              trialLeafs.push_back(std::make_pair(newVertex, meanScore));

              // Indicate that we have added at least one leaf
              newLeafAdded = true;
          }  // end-if for score threshold checks
        } // end-if for isTerminator() check
      } // end loop over fitted guesses

      // Were any new leafs added to the current leaf?
      // Otherwise, mark the current leaf as a terminator.
      if(!newLeafAdded)
      {
        _branchGraph[leafVertex].setTerminator();
      }
    } // end if-else - there were good guesses found from the current leaf
  } // end-for over the leafs
}

// --------------------------------------------------------------------------------------------------
//! Predict new vessel templates from the current one (vt) in 3D
// --------------------------------------------------------------------------------------------------
void VesselTracker::_generateGuesses(std::vector<VesselData>& guesses, const VesselData& vd)
{

  // Get template parameters
  const double theta0 = vd.getTheta();
  const double phi0 = vd.getPhi();
  const double radius = vd.getRadius();
  double x0,y0,z0;
  vd.getCenterPoint(x0,y0,z0);


  // Number of guesses we will produce. _nbrOfAngles specifies the number
  // of angles "left" and "right", that is the total number of angles
  // is 2*_nbrAngles + 1. However, if we now that we have just passed
  // a branching/junction, we use only 2 (i.e. 2*2+1=5) angles (and
  // a smaller angle width set below) because we assume that no further
  // junctions lie ahead.
  //const int nbrAngles = vd.isInBranchingRegion()? 2 : _nbrAngles;
  const int nbrAngles =  _nbrAngles;
  const int fullAngles = 2*nbrAngles + 1;

  // Calculate angle resolution. If we just past a branching, the maximum
  // search angle is only 10 degrees.
  const double deltaAngle = ( nbrAngles > 0 ) ? getMaxSearchAngle(vd.isInBranchingRegion())/nbrAngles : 0.0;

  // In 3D we have to square the number of angles as we have to search in two dimensions,
  // i.e., we are sampling a patch on a sphere instead of a segment of a circle.
  const int totAngles = (_TRACKDIM == _2D)? fullAngles : fullAngles*fullAngles;

  // Prepare vector holding the guesses.
  guesses.resize(totAngles);

  // Get step-length
  const double stepLength = getStepLength(vd.getRadius());

  // Generate guesses
  if (_TRACKDIM == _2D)
  {
    unsigned int index = 0;
    for(int i=-nbrAngles; i<=nbrAngles; ++i) {

      // Find angle in spherical coordinates
      const double theta = theta0 + i*deltaAngle;

      // Translate spherical coordinates to a cartesian {x, y, z} vector of unit length
      const double vx = cos(theta);
      const double vy = sin(theta);

      // Find trial center point in world coordinates
      const double x = x0 + stepLength*vx;
      const double y = y0 + stepLength*vy;

      // Set parameters
      guesses[index] = vd;
      guesses[index].setRadius(radius);
      guesses[index].setDirection(vx,vy);
      guesses[index].setCenterPoint(x,y);

      // Calculate and set the score for the vessel template
      _calcScore(guesses[index]);

      // Increment counter
      ++index;
    }
  }
  else if (_TRACKDIM == _3D)
  {
    unsigned int index = 0;
    for(int i=-nbrAngles; i<=nbrAngles; ++i) {
      for(int k=-nbrAngles; k<=nbrAngles; ++k) {

        // Find angles in spherical coordinates
        const double theta = theta0 + i*deltaAngle;
        const double phi = phi0 + k*deltaAngle;

        // Translate spherical coordinates to a cartesian {x, y, z} vector of unit length
        const double vx = cos(theta)*sin(phi);
        const double vy = sin(theta)*sin(phi);
        const double vz = cos(phi);

        // Find trial center point in world coordinates
        const double x = x0 + stepLength*vx;
        const double y = y0 + stepLength*vy;
        const double z = z0 + stepLength*vz;

        // Set parameters
        guesses[index] = vd;
        guesses[index].setRadius(radius);
        guesses[index].setDirection(vx,vy,vz);
        guesses[index].setCenterPoint(x,y,z);

        // Calculate and set the score for the vessel template
        _calcScore(guesses[index]);

        // Increment counter
        ++index;
      }
    }
  }
}


// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
bool VesselTracker::_detectBranching(std::vector<Vertex>& branchLeafs, const VertexScorePairs& leafs)
{



  // If there is only one leaf, no branching detection needs to be done.
  if(leafs.size()  == 1) {
    branchLeafs.resize(1);
    branchLeafs[0] = leafs[0].first;
    return false;
  }

  // If there are two leafs, no clustering needs to be done.
  branchLeafs.resize(2);
  if (leafs.size()  == 2) {
    branchLeafs[0] = (leafs[0].second > leafs[1].second) ?  leafs[0].first : leafs[1].first;
    branchLeafs[1] = (leafs[0].second > leafs[1].second) ?  leafs[1].first : leafs[0].first;
  }
  else {

    // Calculate similarity measure between all points in the leaf vertexes
    // The similarity measure is exp(-distance/radius)
    const unsigned int nbrOfVertexes = leafs.size();
    SymmetricMatrix W(nbrOfVertexes); W = 0;
    unsigned int i,j;
    for(i=0; i<nbrOfVertexes-1; ++i) {
      for(j=i+1; j<nbrOfVertexes; ++j) {
        W(i+1,j+1) = exp(-_calcWorldDistance(leafs[j].first, leafs[i].first, _branchGraph));  // Note: newmat indexing is 1-based and not 0-based
      }
    }

    // Calculate Laplacian L = D - W, where D is a diagonal matrix with the sum
    // of each row/column of W in the diagonal.
    DiagonalMatrix D(nbrOfVertexes);
    for(i=1; i<=nbrOfVertexes; ++i) {
      double s = 0;
      for(j=1; j<nbrOfVertexes; ++j) {
        s += W(i,j);
      }
      D(i) = s;
    }
    SymmetricMatrix L(nbrOfVertexes);
    L << D - W;

    // Perform eigenvalue decomposition of the Laplacian
    // The eigenvalues are sorted in ascending order (i.e., smallest first)
    Matrix V(nbrOfVertexes,nbrOfVertexes);
    try {
      eigenvalues(L,D,V);                               // D is reused here to get the eigenvalues
    }
    catch(BaseException) {
     // ML_PRINT_WARNING("VesselTracker::_detectBranching", ML_PROGRAMMING_ERROR,  BaseException::what());
    }


    // We are interested in the eigenvector belonging to the second smallest eigenvalue, i.e., V(*,2).
    // This vector clusters the points into two clusters: if V(i,2) is negative point i belongs
    // to cluster 1 and if V(i,2) is positive the point belongs to cluster 2.
    const int initScore = -1000;
    double cluster1BestScore = initScore;
    double cluster2BestScore = initScore;
    for(i=0; i<nbrOfVertexes; ++i) {
      if( (V(i+1,2)<=0) && (leafs[i].second > cluster1BestScore)) {
        branchLeafs[0] = leafs[i].first;
        cluster1BestScore = leafs[i].second;
      }
      else if ((V(i+1,2)>0) && (leafs[i].second > cluster2BestScore)) {
        branchLeafs[1] = leafs[i].first;
        cluster2BestScore = leafs[i].second;
      }
    }

    // if cluster1BestLeafIdx or cluster2BestLeafIdx still equals -1, then there is a programming error
    assert( (cluster1BestScore != initScore) & (cluster2BestScore != initScore) );


    // Sort the branchLeafs so the one with the highest score is first
    if(cluster1BestScore < cluster2BestScore) {
      Vertex tmp = branchLeafs[0];
      branchLeafs[0] = branchLeafs[1];
      branchLeafs[1] = tmp;
    }

  }

  // Are the best vertexes sufficiently far apart to warrant a branching?
  // The branchLeafs should contain 2 leaf vertexes at this point, sorted
  // so that branchLeafs[0] has the highest score. If the distance between
  // the woorld coordinate points represented by the vertexes is large enough,
  // we have detected a branching, otherwise not.
  double distance = _calcWorldDistance(branchLeafs[0], branchLeafs[1], _branchGraph);
  if(distance > 2) {
    return true;        // Branching detected
  }
  else {
    branchLeafs.resize(1);    // No branching, keep branchLeafs[0] and remove branchLeafs[1]
    return false;
  }

}

// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
inline double VesselTracker::_calcWorldDistance(const Vertex& v1, const Vertex& v2, const boostGraph& graph)
{


  double radius1 = graph[v1].getRadius();
  double radius2 = graph[v2].getRadius();
  double x1, y1, z1;
  double x2, y2, z2;
  graph[v1].getCenterPoint(x1,y1,z1);
  graph[v2].getCenterPoint(x2,y2,z2);

  const double meanRadius = 0.5*(radius1 + radius2);
  const double dx = x1 - x2;
  const double dy = y1 - y2;
  const double dz = z1 - z2;
  return sqrt(dx*dx + dy*dy + dz*dz) / meanRadius;


}

// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
void VesselTracker::_analyzeScores(std::vector<VesselData>& bestGuesses, const std::vector<VesselData>& guesses)
{

  // Clear and get number of guesses
  bestGuesses.clear();
  const size_t nbrGuesses = guesses.size();

  // If there is only one guess, this is also the best guess.
  if(nbrGuesses == 1) {
    bestGuesses.push_back(guesses[0]);
    return;
  }

  // Treat the posterior as an image volume
  // Create stride vector with offset indexes into the posterior vector for the neighbors
  int strideVector[2];
  strideVector[0] = 1;                                   // Increment 1 step to find next angle in phi-dimension
  strideVector[1] = 2*_nbrAngles+1;                      // Increment 2*_nbrAngles+1 steps to find next angle in theta-dimension

  // Build a vector of offsets to the neighbors
  // 2D tracking -> 1D score function (phi)       -> 2 neighbors
  // 3D tracking -> 2D score function (phi,theta) -> 8 neighbors
  const int nbrOfNeighbors = (_TRACKDIM == _2D)? 2 : 8;
  std::vector<int> offsetVector(nbrOfNeighbors);
  if(_TRACKDIM == _2D)
  {
    offsetVector[0] = -1;
    offsetVector[1] = 1;
  }
  if(_TRACKDIM == _3D)
  {
    int counter = 0;
    for(int y=-1; y<=1; ++y) {
      for(int x=-1; x<=1; ++x) {
        if( (y!=0) || (x!=0)) {       // x=y=0 point is the current point and not a neighbor
          offsetVector[counter] = x*strideVector[0] + y*strideVector[1];
          ++counter;
        }
      }
    }
  }

  // Find local maxima in the score function
  int i,j;
  for(i=0; i<nbrGuesses; ++i) {
    bool isLocalMaxima = true;                                       // Assume current posterior sample is a local maximum
    double currentScore = guesses[i].getScore();
    for(j=0; j<nbrOfNeighbors; ++j) {                                // Loop over the neighbors of the current sample
      int neighborIdx = i+offsetVector[j];                           // Index for the neighbor
      if( (neighborIdx < nbrGuesses) && (neighborIdx>=0)) {          // If neighbor is not outside the score function "image"
        if(guesses[neighborIdx].getScore() >= currentScore) {        // Check if score of neighbor is larger
          isLocalMaxima = false;                                     // If so, the current posterior sample is not a local maximum
          break;
        }
      }
    }

    // If the current tubular segment is a local maximu, add it to the output.
    if(isLocalMaxima) {
      bestGuesses.push_back(guesses[i]);
    }
  }

  // Before we return, sort the output vessel segments so that
  // the best (highest score) is the first one. Use
  // a simple selection sort for this (the list is very short, 1-4 items)
  int nbrOfMaxima = bestGuesses.size();
  if(nbrOfMaxima > 1) {               // Should be at least 2 local maxima, otherwise is there no need for sorting
    VesselData tmpSwap;
    int maxIdx=0;
    for (i=0; i<nbrOfMaxima-1; ++i) {
      maxIdx = i;
      for (j = i+1; j<nbrOfMaxima; ++j) {
        if (bestGuesses[j].getScore() > bestGuesses[maxIdx].getScore()) {
          maxIdx = j;
        }
      }
      tmpSwap = bestGuesses[i];
      bestGuesses[i] = bestGuesses[maxIdx];
      bestGuesses[maxIdx] = tmpSwap;
    }
  }
}



// --------------------------------------------------------------------------------------------------
//!
// --------------------------------------------------------------------------------------------------
bool VesselTracker::borderCheck(const VesselData& vd)
{

  // Get position of vertex v in the voxel coordinate system
  double x=0, y=0, z=0;
  vd.getCenterPoint(x,y,z);
  Vector3 vc = _imProps.mapWorldToVoxel(Vector3(x, y, z));
  x=vc[0];
  y=vc[1];
  z=vc[2];

  // Get box of data
  int extent[6];
  _imData->GetExtent(extent);
  //ML_NAMESPACE::SubImageBox box = _imData.getBox();

  // Calculate distances to box sides in voxels
//  const double dx1 = std::abs(x-box.v1.x);
//  const double dx2 = std::abs(x-box.v2.x);
//  const double dy1 = std::abs(y-box.v1.y);
//  const double dy2 = std::abs(y-box.v2.y);
//  const double dz1 = std::abs(z-box.v1.z);
//  const double dz2 = std::abs(z-box.v2.z);
  const double dx1 = std::abs(x-extent[0]);
  const double dx2 = std::abs(x-extent[1]);
  const double dy1 = std::abs(y-extent[2]);
  const double dy2 = std::abs(y-extent[3]);
  const double dz1 = std::abs(z-extent[4]);
  const double dz2 = std::abs(z-extent[5]);

  // Calculate security distance: step length + half of a template size.
  // First, get step length in voxels (worst case scenario)
  const double vstepLength = getStepLength(vd.getRadius())/_smallestVoxelSize;

  // Get the data size that will be used for evaluating the vessel model
  int voxelHalfSize[3];
  _getPatchSize(voxelHalfSize, vd.getRadius());

  // Find minimum sizes we need to be safe.
  // Note: The point (x,y,z) is where we currently are standing. From this point
  // we will take _searchDepth steps.
  const double marginx = _searchDepth*vstepLength + voxelHalfSize[0] + 6; // Add an extra margin of 6 voxels to be sure
  const double marginy = _searchDepth*vstepLength + voxelHalfSize[1] + 6;
  const double marginz = _searchDepth*vstepLength + voxelHalfSize[2] + 6;

  // If we are too close to the border, return false
  if( (_TRACKDIM == _3D) &&
    ( (dx1<marginx) || (dx2<marginx) || (dy1<marginy) || (dy2<marginy) || (dz1<marginz) || (dz2<marginz) ) )
  {
      return false;
  }
  else if ( (_TRACKDIM == _2D) &&
    ( (dx1<marginx) || (dx2<marginx) || (dy1<marginy) || (dy2<marginy)) )
  {
      return false;
  }

  // If we came this far, we are within the mask and within the data block, return true.
  return true;

}


bool VesselTracker::withinMask(const VesselData& vd)
{
  // If the user supplied a mask, check if we are within this mask
  if(_maskData) {

    // Get position of vertex v in the voxel coordinate system
    double x=0, y=0, z=0;
    vd.getCenterPoint(x,y,z);
    Vector3 vc = _imProps.mapWorldToVoxel(Vector3(x, y, z));
    x=vc[0];
    y=vc[1];
    z=vc[2];

    // Get mask value for model.
    uint maskVal = (uint)_maskData->GetScalarComponentAsDouble((int)floor(x), (int)floor(y), (int)floor(z),0);
    //uint maskVal = _maskData.getImageValue((int)floor(x), (int)floor(y), (int)floor(z));

    // Is mask value zero? Then this position is not allowed.
    if(maskVal == 0) {
      return false;
    }
  }

  // Ok if we reach this point.
  return true;

}
