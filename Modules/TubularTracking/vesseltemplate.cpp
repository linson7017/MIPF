#include "VesselTemplate.h"

// -----------------------------------------------------------------------------------
//! Get template value for the input coordinate (x,y).
//! Calculates the distance from the point (x,y,z) to the line parameterized
//! by the center point (x0,y0,z0) and direction v of the vessel template.
//! The distance formula is
//! d^2 = ||x-x0||^2 - (v'(x-x0))^2 (obtained simply via Pythagoras' theorem)
//! or written out with cx = x-x0, cy = y-y0 and cz = z-z0 the centered coordinates:
//! d^2 = cx^2 + cy^2 + cz^2 - (vx*cx + vy*cy + vz*cz)^2
//! In the calculations below the notation
//! K = vx*cx + vy*cy + vz*cz
//! is used.
// -----------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------
//! Default constructor
// -----------------------------------------------------------------------------------
VesselTemplate::VesselTemplate(void)
{
  // Update vessel profile parameters
  updateProfileParameters(static_cast<const VesselData*> (this));
}



// -----------------------------------------------------------------------------------
//! Constructor converting a VesselData object into a VesselTemplate
// -----------------------------------------------------------------------------------
VesselTemplate::VesselTemplate(const VesselData& vd) : VesselData(vd)
{
  // Update vessel profile parameters
  updateProfileParameters(static_cast<const VesselData*> (this));
}

// -----------------------------------------------------------------------------------
//! Determines whether 2D or 3D template should be calculated.
// -----------------------------------------------------------------------------------
void VesselTemplate::getValue(ImPatch &T, const ImPatch& W)
{

  // The extent along the z-axis determines if template is calculated for 2D or 3D.
  if(T.extent.zsize < 2) {
    getValue2D(T,W);
  }
  else {
    getValue3D(T,W);
  }
}

// -----------------------------------------------------------------------------------
//! Get 2D template. See comments at the top of this file for notes on the calculation.
//! Input parameters:  x,y containing coordinates spanning a square.
//! Output parameters: value containing the template values for the input coordinates.
// -----------------------------------------------------------------------------------
void VesselTemplate::getValue2D(ImPatch& T, const ImPatch& W)

{
  // Get number of elements
  const unsigned int nbrElements = T.extent.xsize * T.extent.ysize;

  // Assert that W is of  this size.
  assert(nbrElements == W.values.size());

  // Update vessel profile parameters so that it can precalculate
  // some parameters.
  updateProfileParameters(static_cast<const VesselData*> (this));

  // Resize output variable if necessary
  if(T.values.size() != nbrElements) {
    T.resize(nbrElements);
  }

  // Get center point
  double x0,y0;
  getCenterPoint(x0,y0);

  // Get template direction
  double vx, vy;
  getDirection(vx,vy);

  // Get vessel profile limits
  double lowLimit;
  double highLimit;
  getProfileLimits(lowLimit, highLimit);

  // Loop variables
  int xk=0, yk=0;

  // Variables that are used to calculate the squared distance to a line.
  // Note: This limits the template size to 100x100x100 which
  // corresponds to a vessel of radius 20 voxels
  static double cx2[MAXSIZE];
  static double vxcx[MAXSIZE];

  // Pre-calculate cx2 and vx*cx where cx is the centered x-coordinate
  const double xstart0 = T.extent.xstart - x0;
  for(xk=0; xk<T.extent.xsize; ++xk) {
    double cx = xstart0 + xk*T.extent.dx;
    cx2[xk] = cx*cx;
    vxcx[xk] = cx*vx;
  }

  // Calculate (squared) distance to line in 2D
  // See notes on top for explanation of calculations.
  int counter = 0;
  int nonZeroCounter = 0;
  const double ystart0 = T.extent.ystart - y0;
  for(yk=0; yk<T.extent.ysize; ++yk) {
    const double cy = ystart0 + yk*T.extent.dy;
    const double cy2 = cy*cy;
    const double Ky = vy*cy;
    for(xk=0; xk<T.extent.xsize; ++xk) {
      if(W.values[counter] > 0.0) {
        const double K = vxcx[xk] + Ky;
        const double d2 = cx2[xk] + cy2 - K*K;
        if(d2<highLimit) {
          T.values[counter] = getProfile(d2);
          T.nonZeroIndex[nonZeroCounter] = counter;
          ++nonZeroCounter;
        }
        else {
          T.values[counter] = 0.0;
        }
      }
      else {
        T.values[counter] = 0.0;
      }
      ++counter;
    }
  }
  T.nbrNonZeroIdxs = nonZeroCounter;
}

// -----------------------------------------------------------------------------------
//! Get 3D template. See comments at the top of this file for notes on the calculation.
//! Input parameters:  x,y,z containing coordinates spanning a cube.
//! Output parameters: value containing the template values for the input coordinates.
// -----------------------------------------------------------------------------------
void VesselTemplate::getValue3D(ImPatch& T, const ImPatch& W)
{
  // Get number of elements
  const unsigned int nbrElements = T.extent.xsize * T.extent.ysize * T.extent.zsize;

  // Assert that W is of  this size.
  assert(nbrElements == W.values.size());

  // Update profile parameters
  updateProfileParameters(static_cast<const VesselData*> (this));

  // Resize output variable if necessary
  if(T.values.size() != nbrElements) {
    T.resize(nbrElements);
  }

  // Get center point
  double x0,y0,z0;
  getCenterPoint(x0,y0,z0);

  // Get vessel profile limits
  double lowLimit;
  double highLimit;
  getProfileLimits(lowLimit, highLimit);

  // Get template direction
  double vx, vy, vz;
  getDirection(vx,vy,vz);

  // Loop variables
  int xk=0, yk=0, zk=0;

  // Variables that are used to calculate the squared distance to a line.
  // Note: This limits the template size to 100x100x100 which
  // corresponds to a vessel of radius 20 voxels
  static double cx2[MAXSIZE];
  static double vxcx[MAXSIZE];
  static double cy2[MAXSIZE];
  static double vycy[MAXSIZE];

  // Pre-calculate cx2 and vx*cx where cx is the centered x-coordinate
  const double xstart0 = T.extent.xstart - x0;
  for(xk=0; xk<T.extent.xsize; ++xk) {
    double cx = xstart0 + xk*T.extent.dx;
    cx2[xk] = cx*cx;
    vxcx[xk] = cx*vx;
  }

  // Pre-calculate  cxy and vy*cy where cy is the centered y-coordinate
  const double ystart0 = T.extent.ystart - y0;
  for(yk=0; yk<T.extent.ysize; ++yk) {
    double cy = ystart0 + yk*T.extent.dy;
    cy2[yk] = cy*cy;
    vycy[yk] = cy*vy;
  }

  // Calculate (squared) distance to line in 3D.
  // See notes on top for explanation of calculations.
  int counter = 0;
  int nonZeroCounter = 0;
  const double zstart0 = T.extent.zstart - z0;
  for(zk=0; zk<T.extent.zsize; ++zk) {
    const double cz = zstart0 + zk*T.extent.dz;
    const double cz2 = cz*cz;
    const double Kz = vz*cz;
    for(yk=0; yk<T.extent.ysize; ++yk) {
      const double cy2cz2 = cy2[yk] + cz2;
      const double KyKz = vycy[yk] + Kz;
      for(xk=0; xk<T.extent.xsize; ++xk) {
        if(W.values[counter] > 0.0) {
          const double K = vxcx[xk] + KyKz;
          const double d2 = cx2[xk] + cy2cz2 - K*K;
          if(d2<highLimit) {
            T.values[counter] = getProfile(d2);
            T.nonZeroIndex[nonZeroCounter] = counter;
            ++nonZeroCounter;
          }
          else {
            T.values[counter] = 0.0;
          }
        }
        else {
          T.values[counter] = 0.0;
        }
        ++counter;
      }
    }
  }
  T.nbrNonZeroIdxs = nonZeroCounter;

  // Error message if no non-zero template values were calculated.
  assert(nonZeroCounter > 0);

}


// -----------------------------------------------------------------------------------
//! T(r,x0,v; x) = p(d2(x;x0,v),r)
//!
//! dT/dr = dp/dr
//! dT/dphi = dp/dd2 * (dd2/dv' * dv/dphi)
//! dT/dtheta = dp/dd2 * (dd2/dv' * dv/dtheta)
//! dT/du = u'*dT/dx0 = u'(dp/dd2 * dd2/dx0)
//!
//! dv/dphi and dv/dtheta are calculated in base class VesselData.
//! dd2/dv and dd2/dx0 is calculated in this function
//! dp/dd2 and dp/dr are special to the vessel profile and is calculated in virtual function
// -----------------------------------------------------------------------------------
void VesselTemplate::getDerivatives(std::valarray<double>& dT_dr,
                                    std::valarray<double>& dT_du,
                                    std::valarray<double>& dT_dtheta,
                                    const std::valarray<double>& T,
                                    double xstart, double dx, int xsize,
                                    double ystart, double dy, int ysize)
{
  // Get number of elements
  const unsigned int nbrElements = xsize * ysize;

  // Check if arrays needs to be resized
  if(dT_dr.size() != nbrElements) {dT_dr.resize(nbrElements);}
  if(dT_du.size() != nbrElements) {dT_du.resize(nbrElements);}
  if(dT_dtheta.size() != nbrElements) {dT_dtheta.resize(nbrElements);}

  // Update profile parameters
  updateProfileParameters(static_cast<const VesselData*> (this));

  // Get center point
  double x0,y0;
  getCenterPoint(x0,y0);

  // Get template direction
  double vx, vy;
  getDirection(vx,vy);

  // Get orthogonal directions
  double ux,uy;
  getOrthoDirection(ux,uy);

  // Get derivatives of vessel direction v with respect
  // to spherical coordinates angles theta and phi.
  double dvx_dtheta, dvy_dtheta;
  _getDerivatives(dvx_dtheta, dvy_dtheta);

  // Loop variables
  int xk=0, yk=0;

  // Variables that are used to calculate the squared distance to a line.
  // Note: This limits the template size to 100x100x100 which
  // corresponds to a vessel of radius 20 voxels
  static double cx[MAXSIZE];
  static double cx2[MAXSIZE];
  static double vxcx[MAXSIZE];

  // Pre-calculate cx2 and vx*cx where cx is the centered x-coordinate
  const double xstart0 = xstart - x0;
  for(xk=0; xk<xsize; ++xk) {
    double _cx_ = xstart0 + xk*dx;
    cx[xk] = _cx_;
    cx2[xk] = _cx_*_cx_;
    vxcx[xk] = _cx_*vx;
  }

  // Loop over input coordinates
  int counter = 0;
  const double ystart0 = ystart - y0;
  for(yk=0; yk<ysize; ++yk) {
    const double cy = ystart0 + yk*dy;
    const double cy2 = cy*cy;
    const double Ky = vy*cy;
    for(xk=0; xk<xsize; ++xk) {

      // Calculate K constant, see top.
      const double K = vxcx[xk] + Ky;

      // Calculate squared distance
      const double d2 = cx2[xk] + cy2 - K*K;

      // Calculate derivative of profile with respect to squared distance
      const double dp_dd2 = getProfile_dx2(d2,T[counter]);

      // Calculate derivative of squared distance with respect to vessel direction
      const double dd2_dvx = -2*K*cx[xk];
      const double dd2_dvy = -2*K*cy;

      // Calculate derivative of squared distance with respect to vessel center point
      const double dd2_dx0 = 2*(K*vx - cx[xk]);
      const double dd2_dy0 = 2*(K*vy - cy);

      // dT/dr = dp/dr
      dT_dr[counter] = getProfile_dr(d2, T[counter]);

      //! dT/dtheta = dp/dd2 * (dd2/dv' * dv/dtheta)
      dT_dtheta[counter] = dp_dd2*(dd2_dvx * dvx_dtheta + dd2_dvy * dvy_dtheta);

      //! dT/du1 = u1'*dT/dx0 = dp/dd2*(u1'*dd2/dx0)
      dT_du[counter] = dp_dd2*(ux*dd2_dx0 + uy*dd2_dy0);

      ++counter;
    }
  }
}



//! Get template value and derivatives 3D.
// TODO: Send entire ImPatch T and calculate derivatives
// only for non-zero template values.
void VesselTemplate::getDerivatives(std::valarray<double>& dT_dr,
                                    std::valarray<double>& dT_du1,
                                    std::valarray<double>& dT_du2,
                                    std::valarray<double>& dT_dtheta,
                                    std::valarray<double>& dT_dphi,
                                    const std::valarray<double>& T,
                                    double xstart, double dx, int xsize,
                                    double ystart, double dy, int ysize,
                                    double zstart, double dz, int zsize)

{
  // Get number of elements
  const unsigned int nbrElements = xsize * ysize * zsize;

  // Check if arrays needs to be resized
  if(dT_dr.size() != nbrElements) {dT_dr.resize(nbrElements);}
  if(dT_du1.size() != nbrElements) {dT_du1.resize(nbrElements);}
  if(dT_du2.size() != nbrElements) {dT_du2.resize(nbrElements);}
  if(dT_dtheta.size() != nbrElements) {dT_dtheta.resize(nbrElements);}
  if(dT_dphi.size() != nbrElements) {dT_dphi.resize(nbrElements);}

  // Update profile parameters
  updateProfileParameters(static_cast<const VesselData*> (this));

  // Get center point
  double x0,y0,z0;
  getCenterPoint(x0,y0,z0);

  // Get template direction
  double vx, vy, vz;
  getDirection(vx,vy,vz);

  // Get orthogonal directions
  double u1x,u1y,u1z;
  double u2x,u2y,u2z;
  getOrthoDirection(u1x,u1y,u1z,u2x,u2y,u2z);

  // Get derivatives of vessel direction v with respect
  // to spherical coordinates angles theta and phi.
  double dvx_dtheta, dvy_dtheta;
  double dvx_dphi, dvy_dphi, dvz_dphi;
  _getDerivatives(dvx_dtheta, dvy_dtheta, dvx_dphi, dvy_dphi, dvz_dphi);

  // Loop variables
  int xk=0, yk=0, zk=0;

  // Variables that are used to calculate the squared distance to a line.
  // Note: This limits the template size to 100x100x100 which
  // corresponds to a vessel of radius 20 voxels
  static double cx[MAXSIZE];
  static double cx2[MAXSIZE];
  static double vxcx[MAXSIZE];
  static double cy[MAXSIZE];
  static double cy2[MAXSIZE];
  static double vycy[MAXSIZE];

  // Pre-calculate cx2 and vx*cx where cx is the centered x-coordinat
  const double xstart0 = xstart - x0;
  for(xk=0; xk<xsize; ++xk) {
    double _cx_ = xstart0 + xk*dx;
    cx[xk] = _cx_;
    cx2[xk] = _cx_*_cx_;
    vxcx[xk] = _cx_*vx;
  }

  // Pre-calculate  cxy and vy*cy where cy is the centered y-coordinate
  const double ystart0 = ystart - y0;
  for(yk=0; yk<ysize; ++yk) {
    double _cy_ = ystart0 + yk*dy;
    cy[yk] = _cy_;
    cy2[yk] = _cy_*_cy_;
    vycy[yk] = _cy_*vy;
  }


  // Loop over input coordinates
  // Calculate (squared) distance to line in 3D
  int counter = 0;
  const double zstart0 = zstart - z0;
  for(zk=0; zk<zsize; ++zk) {
    const double cz = zstart0 + zk*dz;
    const double cz2 = cz*cz;
    const double Kz = vz*cz;
    for(yk=0; yk<ysize; ++yk) {
      const double cy2cz2 = cy2[yk] + cz2;
      const double KyKz = vycy[yk] + Kz;
      for(xk=0; xk<xsize; ++xk) {

        // Calculate K constant, see top.
        const double K = vxcx[xk] + KyKz;

        // Calculate squared distance
        const double d2 = cx2[xk] + cy2cz2 - K*K;

        // Calculate derivative of profile with respect to squared distance
        const double dp_dd2 = getProfile_dx2(d2, T[counter]);

        // Calculate derivative of squared distance with respect to vessel direction
        const double dd2_dvx = -2*K*cx[xk];
        const double dd2_dvy = -2*K*cy[yk];
        const double dd2_dvz = -2*K*cz;

        // Calculate derivative of squared distance with respect to vessel center point
        const double dd2_dx0 = 2*(K*vx - cx[xk]);
        const double dd2_dy0 = 2*(K*vy - cy[yk]);
        const double dd2_dz0 = 2*(K*vz - cz);

        // dT/dr = dp/dr
        dT_dr[counter] = getProfile_dr(d2, T[counter]);

        //! dT/dphi = dp/dd2 * (dd2/dv' * dv/dphi)
        dT_dphi[counter] = dp_dd2*(dd2_dvx * dvx_dphi + dd2_dvy * dvy_dphi + dd2_dvz * dvz_dphi);

        //! dT/dtheta = dp/dd2 * (dd2/dv' * dv/dtheta)
        dT_dtheta[counter] = dp_dd2*(dd2_dvx * dvx_dtheta + dd2_dvy * dvy_dtheta);

        //! dT/du1 = u1'*dT/dx0 = dp/dd2*(u1'*dd2/dx0)
        dT_du1[counter] = dp_dd2*(u1x*dd2_dx0 + u1y*dd2_dy0 + u1z*dd2_dz0);

        //! dT/du2 = u2'*dT/dx0 = dp/dd2*(u2'*dd2/dx0)
        dT_du2[counter] = dp_dd2*(u2x*dd2_dx0 + u2y*dd2_dy0 + u2z*dd2_dz0);

        ++counter;
      }
    }
  }
}
