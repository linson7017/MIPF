#include "VesselTemplateTracker.h"
#include "ImageVector.h"

// --------------------------------------------------------------------------------------------------
//! Constructor.
// --------------------------------------------------------------------------------------------------
VesselTemplateTracker::VesselTemplateTracker(InternalImageType* imData, BinaryImageType* maskData,
                                             const MedicalImageProperties& imProps, DIM TRACKDIM) :
VesselTracker(imData, maskData, imProps, TRACKDIM), _T(0), _W(0), _I(0), _trial_T(0), _windowSizeFactor(4.5) {}

// --------------------------------------------------------------------------------------------------
//! Levenberg-Marquardt method for solving the non-linear least squares problem
//! min || W(x)[I(x) - k*T(x;r,v,x0) - m] ||^2
//! where I(x) is an image patch, T(x;r,v,x0) is the tubular template and W(x) is a
//! weight function or a window centered (approximately) above x0.
//!
//! The algorithm iterates the following two steps:
//! 1) Levenberg-Marquardt method for finding the non-linear parameters r (radius)
//!    v (direction) and x0 (center point). The linear parametes k and m are kept fixed.
//! 2) Ordinary least squares for finding the linear parameters k (contrast) and
//!    m (mean voxel intensity). The non-linear parameters r,v and x0 are kept fixed.
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::_fitModel(VesselData& vd, const VesselData& vd_prev)
{

  // First, indicated that the model has been fitted
  vd.setFitted();

  // Create a VesselTemplate object out of the VesselData object
  _vt = vd;

  // Get position of the previous template. This is used to make sure that the
  // center point of the new fitted template is constrained to be exactly one
  // step length away from the previous center point.
  double x0_prev,y0_prev,z0_prev;
  vd_prev.getCenterPoint(x0_prev,y0_prev,z0_prev);

  // Get direction of previous template. This is used to ensure that the angle
  // between the previous template and the current template to be fitted stays
  // within the user-given max angle.
  double vx_prev, vy_prev, vz_prev;
  vd_prev.getDirection(vx_prev, vy_prev, vz_prev);

  // Get initial position of the current template
  double x0,y0,z0;
  _vt.getCenterPoint(x0,y0,z0);

  // Calculate step length between the templates. This step length will be preserved.
  const double stepLength = sqrt((x0_prev-x0)*(x0_prev-x0) +
                                 (y0_prev-y0)*(y0_prev-y0) +
                                 (z0_prev-z0)*(z0_prev-z0));

  // Vessel segment radius is only allowed to change 10% from step to step
  // (for the first step, find optimal radius without confinement)
  double minRadius = getMinRadius();
  double maxRadius = getMaxRadius();
  if(stepLength > 0) {
    minRadius = std::max( 0.9*vd_prev.getRadius(), minRadius );
    maxRadius = std::min( 1.1*vd_prev.getRadius(), maxRadius );
  }

  // Get image data
  _getImPatch(_I, _vt);
  const unsigned int nbrOfVoxels = _I.values.size();

  // Get appropriate weight function.
  // We also record the center position and angle of the template for which the
  // weight functions was calculated. The weight function will be updated if
  // these parameters change too much.
  _W.extent = _I.extent;
  _weightFunction(_W, _vt);
  double wx0, wy0, wz0;
  _vt.getCenterPoint(wx0,wy0,wz0);
  double wTheta = _vt.getTheta();
  double wPhi = _vt.getPhi();

  // Get template
  _T.extent = _I.extent;
  _vt.getValue(_T,_W);

  // Initial linear fit to find contrast and mean
  _lsFit(_vt,_T,_I,_W);

  // Outer optimization loop that iterates
  // 1) Levenberg-Marquardt method for finding the non-linear parameters r (radius)
  //    v (direction) and x0 (center point). The linear parametes k and m are kept fixed.
  // 2) Ordinary least squares for finding the linear parameters k (contrast) and
  //    m (mean voxel intensity). The non-linear parameters r,v and x0 are kept fixed.
  //
  // We do a maximum of 10 iterations and as long as we find Levenberg-Marquardt
  // steps that improve the fit.
  int outerIts = 0;
  bool foundLMStep = true;
  bool converged = false;

  while( (foundLMStep) && (outerIts < 10) && (!converged) )
  {
    // Increment iteration counter
    ++outerIts;

    // Get initial template parameters
    double contrast = _vt.getContrast();
    double mean = _vt.getMean();
    double radius = _vt.getRadius();
    _vt.getCenterPoint(x0,y0,z0);
    double vx, vy, vz;
    _vt.getDirection(vx, vy, vz);

    // Store current parameters so that the optimized paramters
    // can be compared with the current and so that convergence
    // can be assessed.
    double prevParams[8] = {contrast, mean, radius, _vt.getTheta(), _vt.getPhi(), x0, y0, z0};

    // Calculate residuals and loss function.
    // We only need to calculate for positions with non-zero weight.
    if(_wresiduals.size() != nbrOfVoxels) { _wresiduals.resize(nbrOfVoxels);}
    double lossFunction = 0.0;
    unsigned int i = 0;
    for(i=0; i<_W.nbrNonZeroIdxs; ++i) {
      const int index = _W.nonZeroIndex[i];
      const double wr = _W.values[index]*(contrast*_T.values[index] + mean - _I.values[index]);
      _wresiduals[index] = wr;
      lossFunction += wr*wr;
    }

    // Levenberg-Marquardt iterations to find optimal radius (r), direction (v), and center point (x0).
    // Build matrices J'*W2*J and J'*W2*r where J is the Jacobian matrix, i.e, a matrix with the
    // partial derivatives, and r is the residuals (i.e, residuals). In the 2D case
    // J = [dT/dr, dT/du, dT/dtheta]
    // and in the 3D case
    // J = [dT/dr, dT/du1, dT/du2, dT/dtheta, dT/dphi] stretched to vectors.
    // u1 and u2 are two orthogonal direction to the template direction v (only u1 is used in 2D).
    // These are used to determine
    // the center point x0 (it makes no sense to translate the center point along the direction v).
    // Theta and phi are the angles of v in a spherical coordinate system.

    const unsigned int NBRFITPARAMS = (_TRACKDIM == _3D)? 5 : 3;
    SymmetricMatrix JW2J(NBRFITPARAMS); JW2J = 0.0;
    ColumnVector JW2r(NBRFITPARAMS); JW2r = 0.0;
    if(_TRACKDIM == _2D) {
      std::valarray<double> j1(nbrOfVoxels),j2(nbrOfVoxels),j3(nbrOfVoxels);
      _vt.getDerivatives(j1, j2, j3, _T.values, _I.extent.xstart, _I.extent.dx, _I.extent.xsize, _I.extent.ystart, _I.extent.dy, _I.extent.ysize);
      // We only need to calculate for positions with non-zero weight.
      for(i=0; i<_W.nbrNonZeroIdxs; ++i) {
        const int index = _W.nonZeroIndex[i];
        const double wr = _wresiduals[index];
        const double w = _W.values[index];
        const double wj1 = w*j1[index];
        const double wj2 = w*j2[index];
        const double wj3 = w*j3[index];
        JW2J(1,1) += wj1*wj1; JW2J(1,2) += wj1*wj2; JW2J(1,3) += wj1*wj3;
        JW2J(2,2) += wj2*wj2; JW2J(2,3) += wj2*wj3;
        JW2J(3,3) += wj3*wj3;
        JW2r(1) += wj1*wr; JW2r(2) += wj2*wr; JW2r(3) += wj3*wr;
      }
    }
    else if (_TRACKDIM == _3D) {
      std::valarray<double> j1(nbrOfVoxels),j2(nbrOfVoxels),j3(nbrOfVoxels),j4(nbrOfVoxels),j5(nbrOfVoxels);
      _vt.getDerivatives(j1, j2, j3, j4, j5, _T.values, _I.extent.xstart, _I.extent.dx, _I.extent.xsize, _I.extent.ystart, _I.extent.dy, _I.extent.ysize, _I.extent.zstart, _I.extent.dz, _I.extent.zsize);
      // We only need to calculate for positions with non-zero weight.
      for(i=0; i<_W.nbrNonZeroIdxs; ++i) {
        const int index = _W.nonZeroIndex[i];
        const double wr = _wresiduals[index];
        const double w = _W.values[index];
        const double wj1 = w*j1[index];
        const double wj2 = w*j2[index];
        const double wj3 = w*j3[index];
        const double wj4 = w*j4[index];
        const double wj5 = w*j5[index];
        JW2J(1,1) += wj1*wj1; JW2J(1,2) += wj1*wj2; JW2J(1,3) += wj1*wj3; JW2J(1,4) += wj1*wj4; JW2J(1,5) += wj1*wj5;
        JW2J(2,2) += wj2*wj2; JW2J(2,3) += wj2*wj3; JW2J(2,4) += wj2*wj4; JW2J(2,5) += wj2*wj5;
        JW2J(3,3) += wj3*wj3; JW2J(3,4) += wj3*wj4; JW2J(3,5) += wj3*wj5;
        JW2J(4,4) += wj4*wj4; JW2J(4,5) += wj4*wj5;
        JW2J(5,5) += wj5*wj5;
        JW2r(1) += wj1*wr; JW2r(2) += wj2*wr; JW2r(3) += wj3*wr; JW2r(4) += wj4*wr; JW2r(5) += wj5*wr;
      }
    }
    JW2J *= contrast*contrast;
    JW2r *= contrast;


    // Find maximum of the diagonal of JW2J. This is needed as we below (in the Levenberg-Marquardt algorithm)
    // will add a small number to the diagonal (as a regularization), and we need to find out how small "small" is.
    // Specifically a first guess of "small" will be 1e-3*max(diag(J'*W2*J))
    double maxVal = -1;
    for(i=1; i<=NBRFITPARAMS; ++i) {
      if(JW2J(i,i) > maxVal) {maxVal = JW2J(i,i);}
    }
    // Check if maxVal is less or equal to zero. This means problems
    // as the diagonal of JW2J should contain positive elements.
    if(maxVal<=0) {break;}

    // Value to be added to the diagonal
    double mu = 1e-3*maxVal;
    // Factor to modify mu with in case Levenberg-Marquardt is rejected/accepted
    double ny = 2.0;

    // Orthogonal template direction is required in LM algorithm
    double u1x, u1y, u1z;
    double u2x, u2y, u2z;
    _vt.getOrthoDirection(u1x,u1y,u1z,u2x,u2y,u2z);

    // Perform Levenberg-Marquardt iterations
    foundLMStep = false;
    int lmIts = 0;
    while ((!foundLMStep) && (lmIts < 10)) {

      // Increment iteration counter
      ++lmIts;

      // Solve the equation system (JW2J + mu)*hlm = JW2I
      // hlm contains the parameters for [dT/dr, dT/du, dT/dtheta] in 2D
      // and [dT/dr, dT/du1, dT/du2, dT/dtheta, dT/dphi] in 3D
      SymmetricMatrix A = JW2J;
      ColumnVector hlm;
      for(i=1; i<=NBRFITPARAMS; ++i) {A(i,i) += mu;}
      try {
        hlm = A.i() * (-JW2r);
      }
      catch(BaseException) {
        //ML_PRINT_WARNING("VesselTemplateTracker::_fitModel", ML_PROGRAMMING_ERROR,  BaseException::what());
        break;
      }

      // Calculate norm of hlm. If the norm is too small we don't bother to go on
      double lmNorm = 0;
      for(i=1; i<=NBRFITPARAMS; ++i) {lmNorm += hlm(i)*hlm(i);}
      if(sqrt(lmNorm)<1e-2) {break;}

      // If the norm is too big with don't evaluate the goal function but calculate
      // a new solution with a higher regularization. We do this because it is costly
      // to calculate a new template and try if it fits better. If the solution is too
      // large it is not likely to bring an improvement. The solution is too large if:
      // i)   The radius increases with more than 20%
      // ii)  The spatial shift is larger than the radius
      // iii) The rotation is larger than 12 degrees (0.1 radians)
      bool solutionTooLarge = false;
      if(_TRACKDIM == _2D) {
        double radiusChange = hlm(1);
        double spatialShift = hlm(2);
        double rotation = hlm(3);
        if( (std::abs(radiusChange) > 0.2*radius) ||
            (std::abs(spatialShift) > radius) ||
            (std::abs(rotation) > 0.2)) {  // 0.1 radians ~ 12 degrees
          solutionTooLarge = true;
        }
      }
      else if(_TRACKDIM == _3D) {
        double radiusChange = hlm(1);
        double spatialShift1 = hlm(2);
        double spatialShift2 = hlm(3);
        double rotation1 = hlm(4);
        double rotation2 = hlm(5);
        if( (std::abs(radiusChange) > 0.2*radius) ||
            (std::abs(spatialShift1) > radius) ||
            (std::abs(spatialShift2) > radius) ||
            (std::abs(rotation1) > 0.2) ||
            (std::abs(rotation2) > 0.2)) {
          solutionTooLarge = true;
        }
      }

      // If new parameters are not too large, try them out.
      if(!solutionTooLarge)
      {

        // Create a new test vessel template
        VesselTemplate trial_vt = _vt;

        // Update radius of the test template.
        // The radius is the first parameter in both 2D and 3D.
        // TODO: Set min and max radius depending on the last step
        double t_radius = radius + hlm(1);
        if(t_radius < minRadius) { t_radius = minRadius;}
        if(t_radius > maxRadius) { t_radius = maxRadius;}
        trial_vt.setRadius(t_radius);

        // Update center point by moving in the directions u1 and u2 that are orthogonal to the current
        // template direction v. There is a not-so-elegant step length solution here: for the user-defined
        // start template the step length is 0 because no previous template exists. For this special case
        // there is no restriction on the template movement.
        if(_TRACKDIM == _2D) {
          const double sx = x0 + hlm(2)*u1x - x0_prev;
          const double sy = y0 + hlm(2)*u1y - y0_prev;

          // Calculate norm and make sure that it is well behaved
          const double norm = sqrt(sx*sx + sy*sy);
          //assert(norm>1e-7);

          const double t_x = (stepLength < 1e-8)? x0 + hlm(2)*u1x : x0_prev + sx/norm*stepLength;
          const double t_y = (stepLength < 1e-8)? y0 + hlm(2)*u1y : y0_prev + sy/norm*stepLength;
          trial_vt.setCenterPoint(t_x,t_y);

        }
        else if(_TRACKDIM == _3D) {
          const double sx = x0 + hlm(2)*u1x + hlm(3)*u2x - x0_prev;
          const double sy = y0 + hlm(2)*u1y + hlm(3)*u2y - y0_prev;
          const double sz = z0 + hlm(2)*u1z + hlm(3)*u2z - z0_prev;


          // Calculate norm and make sure that it is well behaved
          const double norm = sqrt(sx*sx + sy*sy + sz*sz);
          //assert(norm>1e-7);

          const double t_x = (stepLength < 1e-8)? x0 + hlm(2)*u1x + hlm(3)*u2x : x0_prev + sx/norm*stepLength;
          const double t_y = (stepLength < 1e-8)? y0 + hlm(2)*u1y + hlm(3)*u2y : y0_prev + sy/norm*stepLength;
          const double t_z = (stepLength < 1e-8)? z0 + hlm(2)*u1z + hlm(3)*u2z : z0_prev + sz/norm*stepLength;
          trial_vt.setCenterPoint(t_x,t_y,t_z);
        }


        // Rotate vessel template direction. Here we must make sure that the angle
        // to the direction of the previous template does not exceed the user given
        // max angle.
        if(_TRACKDIM == _2D) {
          // Rotatate direction according to the angles dtheta and dphi.
          trial_vt.rotate(hlm(3));
        }
        else if(_TRACKDIM == _3D) {
          // Rotatate direction according to the angles dtheta and dphi.
          // Must assure that rotation is not higher than user define max degree...
          trial_vt.rotate(hlm(4),hlm(5));
        }
        // Check angle to previous template with the formula cos(angle) = t_v'*v_prev (the directions
        // have unit length). If angle too big, undo the rotation, i.e., set the values to the vessel
        // template fitted so far.
        double t_vx, t_vy, t_vz;
        trial_vt.getDirection(t_vx, t_vy, t_vz);
        if(acos(t_vx*vx_prev + t_vy*vy_prev + t_vz*vz_prev) > getMaxSearchAngle( _vt.isInBranchingRegion())) {
          trial_vt.setDirection(vx, vy, vz);
        }

        // Get trial template
        _trial_T.extent = _T.extent;
        trial_vt.getValue(_trial_T,_W);

        // Calculate the loss function for the trial vessel template.
        // We only need to calculate for positions with non-zero weight.
        double t_lossFunction = 0.0;
        for(i=0; i<_W.nbrNonZeroIdxs; ++i) {
          const int index = _W.nonZeroIndex[i];
          const double wr = _W.values[index]*(contrast*_trial_T.values[index] + mean - _I.values[index]);
           t_lossFunction += wr*wr;
        }


        // Compare the loss function with the one after the linear fit to see if
        // the adjustment of the non-linear parameters have brought a reduction
        // of the loss function. If so, we accept the adjustments.
        if(t_lossFunction < lossFunction)
        {
          // Set current template to the test template
          _vt = trial_vt;
          // Copy the template values so that we don't have to calculate them again below
          _T = _trial_T;
          // Yes, we found a step.
          foundLMStep = true;
          // Let's go on to the linear step.
          break;
        }
      } // end-if solutionTooLarge

      // If the step was not accepted or the parameters were too large, we increase the regularization
      mu = mu*ny;
      ny = 2*ny;

    } // End while Levenberg-Marquardt iterations

    // Linear fitting part and weight update. We don't need to do this
    // if no Levenberg-Marquardt step was found.
    if(foundLMStep)
    {
      // Linear fit to find contrast and mean
      _lsFit(_vt,_T,_I,_W);

      // Evaluate convergence criteria. Previous parameters have been
      // saved in prevParams[8] = {contrast, mean, radius, _vt.getTheta(), _vt.getPhi(), x0, y0, z0};
      bool contrast_converged = std::abs((_vt.getContrast() - prevParams[0])/prevParams[0]) < 0.03 ? true : false;
      bool mean_converged = std::abs((_vt.getMean() - prevParams[1])/prevParams[1]) < 0.03 ? true : false;
      bool radius_converged = std::abs(_vt.getRadius() - prevParams[2]) < 0.05 ? true : false;
      bool theta_converged = std::abs(_vt.getTheta() - prevParams[3]) < 0.035 ? true : false;    // 2 degrees
      bool phi_converged = std::abs(_vt.getPhi() - prevParams[4]) < 0.035 ? true : false;        // 2 degrees
      _vt.getCenterPoint(x0,y0,z0);
      double dx = (x0-prevParams[5])/_worldVoxelSize[0];   // How far have we moved in voxels
      double dy = (y0-prevParams[6])/_worldVoxelSize[1];
      double dz = (z0-prevParams[7])/_worldVoxelSize[2];
      double dist = sqrt(dx*dx + dy*dy + dz*dz);
      bool dist_converged = dist < 0.1? true : false;     // Moved less than 0.05 voxels => converged

      // If all parameters have changed little, then we have convergence
      if(contrast_converged && mean_converged && radius_converged &&
        theta_converged && phi_converged && dist_converged) {
          converged = true;
      }


      // If we have not converged, check if we need to update the weight function.
      // This is done if the center point or angle has changed too much since
      // the last weight function update.
      if(!converged)
      {
        double dx = (x0-wx0)/_worldVoxelSize[0];   // How far have we moved in voxels
        double dy = (y0-wy0)/_worldVoxelSize[1];
        double dz = (z0-wz0)/_worldVoxelSize[2];
        double wdist = sqrt(dx*dx + dy*dy + dz*dz);
        double theta = _vt.getTheta();
        double phi = _vt.getPhi();
        double thetaDiff = std::abs(wTheta - theta);
        double phiDiff = std::abs(wPhi - phi);
        // If we have moved more than 1.5 voxels or if the vessel angle has
        // changed more than 5 degrees (~0.08 radians) we update the weights.
        if( (wdist > 1.5) || (thetaDiff > 0.08) || (phiDiff > 0.08)) {
          _weightFunction(_W, _vt);
          wx0 = x0; wy0 = y0; wz0 = z0;
          wTheta = theta; wPhi = phi;
        }
      }
    }
  } // End while out loop over linear fit and non-linear Levenberg-Marquardt iterations

  // Finally, calculate the score for the tubular template
  _calcScore(_vt,_T,_I,_W);

  // Set fitted parameters.
  vd = _vt;

}


// --------------------------------------------------------------------------------------------------
//! Weighted linear least squares to update k and m parameters.
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::_lsFit(VesselTemplate& vt, const ImPatch& T, const ImPatch& I, const ImPatch& W)
{
  // The normal equation for solving this problem involves
  // only 2x2 matrices, therefore, we do this "by hand". The normal equations are
  // [m_opt, k_opt] = inv(X'*W2*X)*X'*W2*I, where X = [1 T] stretched to vectors and the image data I also
  // strectched to a vector. Below, the components in inv(X'*W2*X) and X'*W2*I are calculated separately
  // and then multiplied together.
  double sw2=0, sw2t = 0,sw2t2 = 0, sw2ti = 0, sw2i = 0;
  for(unsigned int j=0; j<W.nbrNonZeroIdxs; ++j) {
    const int index = W.nonZeroIndex[j];
    const double w = W.values[index];
    const double t = T.values[index];
    const double i = I.values[index];
    const double wi = w*i;
    sw2 += w*w;
    sw2i += w*wi;
    if(t>0.0) {
      const double wt = w*t;
      sw2t += w*wt;
      sw2t2 += wt*wt;
      sw2ti += wi*wt;
    }
  }

  // Calculate a constant factor required twice below
  const double factor = 1/(sw2*sw2t2-sw2t*sw2t);

  // Note: We now have inv(X'*W2*X) = 1/factor [sW2T2, -sW2T; -sW2T, sW2]
  // and X'*W2*I =[sW2I; sW2TI]

  // Set fitted parameters m and k
  vt.setMean(factor*(sw2t2*sw2i - sw2t*sw2ti));
  vt.setContrast(factor*(sw2*sw2ti - sw2t*sw2i));


}

// --------------------------------------------------------------------------------------------------
//! Convenience _calcScore function that calls the overloaded _calcScore(...) above.
//! This function simply gets the image data and the template values and
//! then calls the overloaded function.
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::_calcScore(VesselData& vd)
{
  // Create a VesselTemplate object out of the VesselData object
  _vt = vd;

  // Get image data
  _getImPatch(_I, _vt);

  // Get weight
  _W.extent = _I.extent;
  _weightFunction(_W, vd);

  // Get template
  _T.extent = _I.extent;
  _vt.getValue(_T,_W);



  // DEBUG
  //vd.DEBUG_W = _W.values;
  //vd.DEBUG_I = _I.values;
  //vd.DEBUG_T = _T.values;


  //  Call overloaded _calcScore() function that calculates the score
  _calcScore(_vt,_T,_I,_W);
  vd.setScore(_vt.getScore());
}


// --------------------------------------------------------------------------------------------------
//! Calculate template score. This is based on the template contrast, i.e., how different from
//! zero the template contrast parameter is. The score equals the t-statistic
//!  t = contrast/std(contrast) or more generally c'beta/sqrt(var(c'beta))
//! where beta=[mean,contrast]' are the image model parameters
//! Image = contrast*Template + mean + noise
//! and c is a so-called contrast vector, in our case c = [0,1]'.
//! The larger the score, the more different from zero the contrast is.
//! The variance in the denominator is calculated using this very nice formula:
//! var(c'beta) = sigma2*c'*inv(X'*W^2*X)*X'*W^4*X*inv(X'*W^2*X)*c.
//! X = [1 T] is the design matrix with a column of ones and the tubular template stretched to a column.
//! sigma2 is the noise variance which is estimated from the residuals r = I - X*beta.
//! W is a diagonal matrix with a Gaussian weight window. This window focusses the score measurement to
//! the center of the tubular template and weights down areas far away from the center.
//! The estimate of sigma2 is weighted so that
//! sigma2 = sum(wi*ri^2)/sum(wi)
//! Note that the weighted estimate of the variance is ad-hoc, but it is a correct estimator.
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::_calcScore(VesselTemplate& vt, const ImPatch& T, const ImPatch& I, const ImPatch& W)
{

  // Assert that sizes of T, I and W are the same
  assert(T.values.size() == W.values.size());
  assert(T.values.size() == I.values.size());

  // Get template parameters
  const double contrast = vt.getContrast();
  const double mean = vt.getMean();

  // The expression c'*inv(X'*W^2*X)*X'*W^4*X*inv(X'*W^2*X)*c was entered in
  // the Matlab symbolic toolbox, which gave the formula shown further down.
  // The components in this formula are calculated below.
  // sigma2 estimator
  double sw=0, sw2=0, sw2t=0, sw2t2=0, sw4=0, sw4t=0, sw4t2=0;
  double sigma2 = 0;
  for(unsigned int j=0; j<W.nbrNonZeroIdxs; ++j) {
    const int index = W.nonZeroIndex[j];
    const double w = W.values[index];
    const double t = T.values[index];
    const double i = I.values[index];
    const double w2 = w*w;
    const double w4 = w2*w2;
    sw  += w;
    sw2 += w2;
    sw4 += w4;

    double r = i - mean;
    if(t>0) {
      const double w2t = w2*t;
      sw2t  += w2t;
      sw2t2 += w2t*t;
      sw4t  += w4*t;
      sw4t2 += w2t*w2t;
      r -= contrast*t;
    }

    // Weight with w2 here?
    sigma2 += w*r*r;
  }
  sigma2 /= sw;

  // The following expression for c'*inv(X'*W^2*X)*X'*W^4*X*inv(X'*W^2*X)*c was found using
  // the symbolic toolbox in Matlab.
  const double tmp = sw2t2*sw2-sw2t*sw2t;
  const double score = contrast/sqrt(sigma2*(sw2*sw2*sw4t2-2*sw2*sw2t*sw4t+sw2t*sw2t*sw4)/(tmp*tmp));
  vt.setScore(score);

}


// --------------------------------------------------------------------------------------------------
//! Calculates an anisotropic Gaussian weight function.
//! The weight function is an anisotropic Gaussian function that this broader orthogonal to the
//! template direction (i.e. in the plane spanned by u1 and u2) than in the template direction given by v.
//! Specifically, the weight function is
//! exp( - (x'*v)^2/(sigma_v^2) - (x'*u1)^2/(sigma_u^2) -(x'*u2)^2/(sigma_u^2))
//! where x = (x,y,z) is the world coordinate, v=(vx,vy,vz) the vessel direction and u1 = (u1x, u1y, u1z)
//! and u2 = (u2x,u2y,u2z) the directions orthogonal to the vessel. sigma_v is smaller than sigma_u.
//! That is
//! x'*v = x*vx + y*vy + z*vz
//! and
//! (x'*v)^2/(2*sigma_v^2) = ( (x*vx + y*vy + z*vz)/sigma_v )^2
//! To speed up the calculations vx, vy and vz are pre-scaled with sigma_v
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::_weightFunction(ImPatch& W, const VesselData& vd)
{

  // Get suitable width of weight window.
  const double worldSigma = _getWindowSigma(vd.getRadius());

  // Determine sigma_v and sigma_u, i.e., weight function extent along and across
  // the vessel respectively.
  const double aniso_factor = 2;
  const double worldSigma_u = worldSigma;
  const double worldSigma_v = worldSigma/aniso_factor;

  // The weight function is an exponential exp(factor). When the weight function is less than
  // a small constant eps, it is approximated with zero. That is when exp(-factor) < eps,
  // or when factor > -log(eps), then is the weight function set to zero.
  const double eps = _getWeightThreshold();
  const double factorThreshold = -log(eps);

  // Get template center point.
  double x0,y0,z0;
  vd.getCenterPoint(x0,y0,z0);

  // Get template direction.
  double vx,vy,vz;
  vd.getDirection(vx, vy, vz);

  // Get orthogonal directions.
  double u1x,u1y,u1z,u2x,u2y,u2z;
  vd.getOrthoDirection(u1x,u1y,u1z,u2x,u2y,u2z);

  // Variables that are used to calculate the weight function
  // Note: This limits the template size to 100x100x100 which
  // corresponds to a vessel of radius 20 voxels
  static float cxvx[100];
  static float cxu1x[100];
  static float cxu2x[100];
  static float cyvy[100];
  static float cyu1y[100];
  static float cyu2y[100];

  // Nbr of voxels. Extent along z-axis determines 2D or 3D
  const unsigned int nbrOfVoxels = (W.extent.ysize > 1)? W.extent.xsize*W.extent.ysize*W.extent.zsize : W.extent.xsize*W.extent.ysize;
  if(W.values.size() != nbrOfVoxels) {W.resize(nbrOfVoxels);}

  // Transform voxel coordinates into the coordinate system spanned
  // by the vessel template direction and the orthogonal direction.
  // Then calculate an anisotropic Gaussian function in this coordinate system.
  int xk=0,yk=0,zk=0;
  if(_TRACKDIM == _2D) {
    // Pre-calculate cxvx and cxu1x where cx is the centered x-coordinate
    const double xstart0 = W.extent.xstart - x0;
    for(xk=0; xk<W.extent.xsize; ++xk) {
      double cx = xstart0 + xk*W.extent.dx;                     // Centered x
      cxvx[xk] = cx*vx/worldSigma_v;
      cxu1x[xk] = cx*u1x/worldSigma_u;
    }

    int counter = 0;
    const double ystart0 = W.extent.ystart - y0;
    for(yk=0; yk<W.extent.ysize; ++yk) {
      const double cy = ystart0 + yk*W.extent.dy;                 // Centered y
      const double cyvy = cy*vy/worldSigma_v;
      const double cyu1y = cy*u1y/worldSigma_u;
      for(xk=0; xk<W.extent.xsize; ++xk) {
        const double tv = cxvx[xk] + cyvy;         // Distance in vessel direction
        const double tu = cxu1x[xk] + cyu1y;       // Distance orthogonal to vessel direction
        const double factor = tv*tv + tu*tu;
        W.values[counter] = (factor > factorThreshold) ? 0 : exp(-factor);
        ++counter;
      }
    }
  }
  else if (_TRACKDIM == _3D) {

    // Pre-calculate cx*vx and cx*u1x where cx is the centered x-coordinate
    const double xstart0 = W.extent.xstart - x0;
    for(xk=0; xk<W.extent.xsize; ++xk) {
      const double cx = xstart0 + xk*W.extent.dx;                     // Centered x
      cxvx[xk] =  cx*vx/worldSigma_v;
      cxu1x[xk] = cx*u1x/worldSigma_u;
      cxu2x[xk] = cx*u2x/worldSigma_u;
    }

    // Pre-calculate cxvx and cxu1x where cx is the centered x-coordinate
    const double ystart0 = W.extent.ystart - y0;
    for(yk=0; yk<W.extent.ysize; ++yk) {
      const double cy = ystart0 + yk*W.extent.dy;                     // Centered y
      cyvy[yk] = cy*vy/worldSigma_v;
      cyu1y[yk] = cy*u1y/worldSigma_u;
      cyu2y[yk] = cy*u2y/worldSigma_u;
    }

    int counter = 0;
    const double zstart0 = W.extent.zstart - z0;
    for(zk=0; zk<W.extent.zsize; ++zk) {
      const double cz = zstart0 + zk*W.extent.dz;            // Centered z
      const double czvz = cz*vz/worldSigma_v;
      const double czu1z = cz*u1z/worldSigma_u;
      const double czu2z = cz*u2z/worldSigma_u;
      for(yk=0; yk<W.extent.ysize; ++yk) {
        const double cyvy_czvz = cyvy[yk] + czvz;
        const double cyu1y_czu1z = cyu1y[yk] + czu1z;
        const double cyu2y_czu2z = cyu2y[yk] + czu2z;
        for(xk=0; xk<W.extent.xsize; ++xk) {
          const double tv = cxvx[xk] + cyvy_czvz;          // v - coordinate
          const double tu1 = cxu1x[xk] + cyu1y_czu1z;      // u1 - coordinate
          const double tu2 = cxu2x[xk] + cyu2y_czu2z;      // u2 - coordinate
          const double factor = tv*tv + tu1*tu1 + tu2*tu2;
          W.values[counter] = (factor > factorThreshold) ? 0.0 : exp(-factor);
          ++counter;
        } // end-for xk
      } // end-for yk
    } // end-for zk
  } // end-if _TRACKDIM == _3D

  // Find the non-zero indexes of W
  int nonZeroCounter = 0;
  for(unsigned int i=0; i<nbrOfVoxels; ++i) {
    if(W.values[i] != 0.0) {
      W.nonZeroIndex[nonZeroCounter] = i;
      ++nonZeroCounter;
    }
  }
  W.nbrNonZeroIdxs = nonZeroCounter;


}


// --------------------------------------------------------------------------------------------------
//! The weight window function is exp(-x^2/sigma^2).
//! This function returns a suitable sigma, i.e., width of the
//! window as a function of the radius.
// --------------------------------------------------------------------------------------------------
inline double VesselTemplateTracker::_getWindowSigma(double worldRadius)
{
  return _windowSizeFactor*worldRadius;   // Was 3.5 before, should probably be around 4. Depends on the profile!!
}


inline double VesselTemplateTracker::_getWeightThreshold()
{
  return 0.1;
}

// --------------------------------------------------------------------------------------------------
//! Sets the size of the vessel template as a function of the vessel radius
// --------------------------------------------------------------------------------------------------
inline void VesselTemplateTracker::_getPatchSize(int voxelHalfSize[3], double worldRadius)
{

  // The template size must be big enough to encompass the weight window.
  const double weightThreshold = _getWeightThreshold();

  // The following formula calculates the x coordinate for which
  // a Gaussian function y = exp(-x^2/sigma^2) attains the value y:
  // x = sigma*sqrt(log(1/y))
  // Note: there are two solutions, but only the positive one is considered.
  const double sigma = _getWindowSigma(worldRadius);
  const double xTh = sigma*sqrt(log(1/weightThreshold));

  // Deduce template size in voxels
  voxelHalfSize[0] = (int)ceil(xTh / _worldVoxelSize[0]);
  voxelHalfSize[1] = (int)ceil(xTh / _worldVoxelSize[1]);
  voxelHalfSize[2] = (_TRACKDIM==_3D)? (int)ceil(xTh/ _worldVoxelSize[2]) : 0;
}


// --------------------------------------------------------------------------------------------------
//! Get a patch of image data for fitting the vessel template vt.
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::_getImPatch(ImPatch& I, const VesselData& vd)
{
  // Get template parameters
  const double worldRadius = vd.getRadius();
  double x0=0, y0=0, z0=0;
  vd.getCenterPoint(x0,y0,z0);

  // Find the voxel coordinates for the current center point
  double voxelCenter[3] = {x0, y0, z0};
  const Vector3 vc = _imProps.mapWorldToVoxel(Vector3(voxelCenter[0], voxelCenter[1], voxelCenter[2]));
  voxelCenter[0] = vc[0];
  voxelCenter[1] = vc[1];
  voxelCenter[2] = vc[2];

  // Find the closest grid point (with a voxel value) in voxel coordinates
  // MeVisLab has its voxel origin at the upper left corner of the upper left voxel. Hence,
  // we need to apply a floor() operation to find integer indexes into the array holding the image.
  double voxelClosestVoxel[3] = {floor(voxelCenter[0]), floor(voxelCenter[1]), floor(voxelCenter[2])};

  // Find the closest grid point (with a voxel value) in world coordinates
  // We have to add 0.5 here as MeVisLab has the center of the voxels at 0.5, 1.5, 2.5 etc.
  double worldClosestVoxel[3] = {voxelClosestVoxel[0]+0.5, voxelClosestVoxel[1]+0.5, voxelClosestVoxel[2]+0.5};
  const Vector3 wc = _imProps.mapVoxelToWorld(Vector3(worldClosestVoxel[0], worldClosestVoxel[1], worldClosestVoxel[2]));
  worldClosestVoxel[0] = wc[0];
  worldClosestVoxel[1] = wc[1];
  worldClosestVoxel[2] = wc[2];

  // Get a suitable patch size.
  int voxelHalfSize[3];
  _getPatchSize(voxelHalfSize, worldRadius);
  int voxelFullSize[3] = {2*voxelHalfSize[0] + 1, 2*voxelHalfSize[1] + 1, 2*voxelHalfSize[2] + 1};
  const unsigned int nbrOfVoxels = voxelFullSize[0]*voxelFullSize[1]*voxelFullSize[2];

  // Resize the variables in I to have the correct size
  if(I.values.size() != nbrOfVoxels) {I.resize(nbrOfVoxels);}

  // Add coordinate info
  I.extent.xstart = worldClosestVoxel[0] - _worldVoxelSize[0]*voxelHalfSize[0];
  I.extent.ystart = worldClosestVoxel[1] - _worldVoxelSize[1]*voxelHalfSize[1];
  I.extent.zstart = worldClosestVoxel[2] - _worldVoxelSize[2]*voxelHalfSize[2];
  I.extent.dx = _worldVoxelSize[0];
  I.extent.dy = _worldVoxelSize[1];
  I.extent.dz = _worldVoxelSize[2];
  I.extent.xsize = voxelFullSize[0];
  I.extent.ysize = voxelFullSize[1];
  I.extent.zsize = voxelFullSize[2];


  // Fill image patch I with voxel values.
  int counter = 0;
  int extent[6];
  _imData->GetExtent(extent);
  for(int z=-voxelHalfSize[2]; z<=voxelHalfSize[2]; ++z) {
    for(int y=-voxelHalfSize[1]; y<=voxelHalfSize[1]; ++y) {
      int x = -voxelHalfSize[0];
      for( ; x<=voxelHalfSize[0]; ++x) {
          I.values[counter] = _imData->GetScalarComponentAsFloat(voxelClosestVoxel[0] + x, voxelClosestVoxel[1] + y, voxelClosestVoxel[2] + z, 0);
          ++counter;
      }
    }
  }
}


// --------------------------------------------------------------------------------------------------
//! Voxelize a vessel tube model in a binary image.
// --------------------------------------------------------------------------------------------------
void VesselTemplateTracker::setVisitedVoxels(const VesselData& vd, BinaryImageType& binaryImg)
{

  // Get template parameters
  const double worldRadius = vd.getRadius();
  double x0=0, y0=0, z0=0;
  vd.getCenterPoint(x0,y0,z0);

  // Find the voxel coordinates for the current center point
  double voxelCenter[3] = {x0, y0, z0};
  const Vector3 vc = _imProps.mapWorldToVoxel(Vector3(voxelCenter[0], voxelCenter[1], voxelCenter[2]));
  voxelCenter[0] = vc[0];
  voxelCenter[1] = vc[1];
  voxelCenter[2] = vc[2];

  // Find the closest grid point (with a voxel value) in voxel coordinates
  // MeVisLab has its voxel origin at the upper left corner of the upper left voxel. Hence,
  // we need to apply a floor() operation to find integer indexes into the array holding the image.
  double voxelClosestVoxel[3] = {floor(voxelCenter[0]), floor(voxelCenter[1]), floor(voxelCenter[2])};

  // Find the closest grid point (with a voxel value) in world coordinates
  // We have to add 0.5 here as MeVisLab has the center of the voxels at 0.5, 1.5, 2.5 etc.
  double worldClosestVoxel[3] = {voxelClosestVoxel[0]+0.5, voxelClosestVoxel[1]+0.5, voxelClosestVoxel[2]+0.5};
  const Vector3 wc = _imProps.mapVoxelToWorld(Vector3(worldClosestVoxel[0], worldClosestVoxel[1], worldClosestVoxel[2]));
  worldClosestVoxel[0] = wc[0];
  worldClosestVoxel[1] = wc[1];
  worldClosestVoxel[2] = wc[2];

  // Get a suitable patch size.
  int voxelHalfSize[3]={0,0,0};
  _getPatchSize(voxelHalfSize, worldRadius);
  int voxelFullSize[3] = {2*voxelHalfSize[0] + 1, 2*voxelHalfSize[1] + 1, 2*voxelHalfSize[2] + 1};

  // Define a template patch
  ImPatch T(0);

  // Define patch extents
  T.extent.xstart = worldClosestVoxel[0] - _worldVoxelSize[0]*voxelHalfSize[0];
  T.extent.ystart = worldClosestVoxel[1] - _worldVoxelSize[1]*voxelHalfSize[1];
  T.extent.zstart = worldClosestVoxel[2] - _worldVoxelSize[2]*voxelHalfSize[2];
  T.extent.dx = _worldVoxelSize[0];
  T.extent.dy = _worldVoxelSize[1];
  T.extent.dz = _worldVoxelSize[2];
  T.extent.xsize = voxelFullSize[0];
  T.extent.ysize = voxelFullSize[1];
  T.extent.zsize = voxelFullSize[2];

  // Define a weight patch
  ImPatch W(0);
  W.extent = T.extent;

  // Create a vessel template out of the vessel data
  VesselTemplate vt = vd;

  // Get a weight function
  _weightFunction(W, vt);

  // Get template
  vt.getValue(T,W);

  // Set voxels in binary image.
  int counter = 0;
  ImageVector<int> coord(0,0,0,0,0,0);
  int extent[6];
  binaryImg.GetExtent(extent);
  for(int z=-voxelHalfSize[2]; z<=voxelHalfSize[2]; ++z) {
    coord.z = voxelClosestVoxel[2] + z;
    for(int y=-voxelHalfSize[1]; y<=voxelHalfSize[1]; ++y) {
      coord.y = voxelClosestVoxel[1] + y;
      for(int x = -voxelHalfSize[0]; x<=voxelHalfSize[0]; ++x) {
        coord.x = voxelClosestVoxel[0] + x;
        // W(eight) is needed as limitation in the template direction
        if((T.values[counter] > 0.5) && (W.values[counter]>0.7) &&
           (coord.x<extent[1])&&(coord.y<extent[3])&&(coord.z<extent[5])&&
            (coord.x>extent[0]) && (coord.y>extent[2]) && (coord.z>extent[4])) 
        {
            binaryImg.SetScalarComponentFromFloat(coord.x,coord.y,coord.z,0,255);
        }
        ++counter;
      }
    }
  }

}
