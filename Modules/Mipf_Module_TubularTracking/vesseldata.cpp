#include "VesselData.h"

//! General constructor.
VesselData::VesselData(void):
 _p1(0), _p2(0),  _radius1(1), _radius2(1), _score(0), _isTerminator(false), _isInBranchingRegion(false), _distance(0), _isFitted(false)
//,DEBUG1(0),DEBUG2(0),DEBUG3(0)
{
  // Set default vessel direction
  this->setDirection(1,0,0);

  // Set default vessel center
  // Set vessel center
  _x0[0] = 0;
  _x0[1] = 0;
  _x0[2] = 0;
}

//! Constructor for 2D circular shape.
VesselData::VesselData(double radius, double x, double y, double vx, double vy):
_p1(0), _p2(0), _radius1(radius), _radius2(radius), _score(0), _isTerminator(false), _isInBranchingRegion(false), _distance(0), _isFitted(false)
//,DEBUG1(0),DEBUG2(0),DEBUG3(0)
{
  // Set vessel direction
  this->setDirection(vx, vy);

  // Set vessel center
  _x0[0] = x;
  _x0[1] = y;
  _x0[2] = 0;   // _x0[2] not used in 2D
}

//! Constructor for 3D circular shape.
VesselData::VesselData(double radius, double x, double y, double z, double vx, double vy, double vz):
_p1(0), _p2(0), _radius1(radius), _radius2(radius), _score(0), _isTerminator(false), _isInBranchingRegion(false), _distance(0), _isFitted(false)
//,DEBUG1(0),DEBUG2(0),DEBUG3(0)
{
  // Set vessel direction
  this->setDirection(vx, vy, vz);

  // Set vessel center
  _x0[0] = x;
  _x0[1] = y;
  _x0[2] = z;
}


//! Set vessel direction in 2D.
void VesselData::setDirection(double vx, double vy)
{
  // Set 2D direction
  const double norm = sqrt(vx*vx + vy*vy);
  _v[0] = vx/norm;
  _v[1] = vy/norm;
  _v[2] = 0;

  // Set angle representation (spherical coordinates)
  _theta = atan2((double)_v[1],(double)_v[0]);
  _phi = 1.57079632679490;  // pi/2

  // Orthogonal 2D vector
  _u1[0] = -_v[1];
  _u1[1] = _v[0];
  _u1[2] = 0;

  // The second orhtogonal vector is not used in 2D
  // but since we know that [0,0,1] is the orthogonal
  // vector, we set _u2 to this.
  _u2[0] = 0;
  _u2[1] = 0;
  _u2[2] = 1;

  // Calculate derivatives
  _dvx_dtheta = -_v[1];
  _dvy_dtheta = _v[0];

  _dvx_dphi = 0;
  _dvy_dphi = 0;
  _dvz_dphi = -1;

}


//! Set vessel direction in 3D.
void VesselData::setDirection(double vx, double vy, double vz)
{

  // Set 3D direction
  const double norm = sqrt(vx*vx + vy*vy + vz*vz);
  _v[0] = vx/norm;
  _v[1] = vy/norm;
  _v[2] = vz/norm;

  // Find polar representation of main direction v
  //      [sin(phi) * cos(theta)]
  //  v = [sin(phi) * sin(theta)]
  //      [      cos(phi)       ]
  _theta = atan2((double)_v[1],(double)_v[0]);
  _phi = acos((double)_v[2]);

  // Pre-calculate some quantities
  const double cosphi = cos(_phi);
  const double sinphi = sin(_phi);
  const double costheta = cos(_theta);
  const double sintheta = sin(_theta);

  // _u2 is a 90 degree rotation of v: phi = phi - pi/2.
  // For example, if v = (1,0,0) _u2 will be (0,0,1)
  // Important: This rotation must be put in _u2 because
  // we want to avoid having a z-component in _u1 if tracking
  // is carried out in 2D.
  _u2[0] = (double)(-cosphi*costheta);
  _u2[1] = (double)(-cosphi*sintheta);
  _u2[2] = (double)(sinphi);

  // _u1 = _u2 x v (vector product).
  // Example:
  // v =(1,0,0) -> _u2=(0,0,1) -> _u1=(0,1,0)
  _u1[0] = _u2[1]*_v[2] - _u2[2]*_v[1];
  _u1[1] = _u2[2]*_v[0] - _u2[0]*_v[2];
  _u1[2] = _u2[0]*_v[1] - _u2[1]*_v[0];

  // Set derivatives
  _dvx_dtheta = (double)(-sinphi*sintheta);     // -sin(phi)sin(theta)
  _dvy_dtheta = (double)(sinphi*costheta);      // sin(phi)cos(theta)

  _dvx_dphi = (double)(cosphi*costheta);        //cos(phi)*cos(theta)
  _dvy_dphi = (double)(-cosphi*sintheta);       //-cos(phi)*sin(theta)
  _dvz_dphi = (double)(-sinphi);                // -sin(phi)

}

//! Returns the vessel direction rotated dtheta radians (2D).
void VesselData::rotate(double theta)
{
  // Increment theta-angle
  _theta += theta;

  // Convert to cartesian coordinates
  const double vx = (double)cos(_theta);
  const double vy = (double)sin(_theta);
  setDirection(vx, vy);
}

//! Returns the vessel direction rotated (dphi,dtheta) radians.
void VesselData::rotate(double theta, double phi)
{
  // Increment angles of the current direction
  _phi += phi;
  _theta += theta;

  // Update vessel direction
  //      [sin(phi) * cos(theta)]
  //  v = [sin(phi) * sin(theta)]
  //      [      cos(phi)       ]

  // Pre-calculate some quantities
  const double cosphi = cos(_phi);
  const double sinphi = sin(_phi);
  const double costheta = cos(_theta);
  const double sintheta = sin(_theta);

  const double vx = (double)(sinphi*costheta);
  const double vy = (double)(sinphi*sintheta);
  const double vz = (double)(cosphi);
  setDirection(vx, vy, vz);
}


