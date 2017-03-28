#ifndef __VesselData_H
#define __VesselData_H

// Get rid of the min() / max () macro definitions in Visual Studio
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <math.h>
#include <valarray>

class VesselData
{
public:

  //! Standard constructor.
  VesselData(void);

  //! Virtual destructor since the class contains virtual functions
  virtual ~VesselData() {};

  //! Constructor for 2D circle shape.
  VesselData(double radius, double x, double y, double vx, double vy);
  //! Constructor for 3D circle shape.
  VesselData(double radius, double x, double y, double z, double vx, double vy, double vz);

  //! Set radius.
  virtual void setRadius(double r) {_radius1 = r; _radius2 = r;}
  //! Get radius.
  double getRadius(void) const {return _radius1;}

  //! Set 2D vessel direction.
  void setDirection(double vx, double vy);
  //! Set 3D vessel direction.
  void setDirection(double vx, double vy, double vz);
  //! Set 2D vessel direction.
  void getDirection(double& vx, double& vy) const {vx=_v[0]; vy=_v[1];}
  //! Set 3D vessel direction.
  void getDirection(double& vx, double& vy, double& vz) const {vx=_v[0]; vy=_v[1]; vz=_v[2];}
   //! Get angle representation of vessel direction in 2D.
  double getTheta(void) const {return _theta;}
  //! Get angle representation of vessel direction in 3D.
  double getPhi(void) const {return _phi;}

  //! Get the two vectors that are orthogonal to the tubular
  //! direction for which the derivatives are calculated.
  void getOrthoDirection(double& ux, double& uy) const {ux=_u1[0]; uy=_u1[1];}
  void getOrthoDirection(double& u1x, double& u1y, double& u1z, double& u2x, double& u2y, double& u2z) const
  {u1x=_u1[0]; u1y=_u1[1]; u1z=_u1[2]; u2x=_u2[0]; u2y=_u2[1]; u2z=_u2[2];}

  //! Set 2D center point.
  void setCenterPoint(double x, double y) {_x0[0] = x; _x0[1] = y;}
  //! Set 3D center point.
  void setCenterPoint(double x, double y, double z) {_x0[0] = x; _x0[1] = y; _x0[2] = z;}
  //! Get 2D center point.
  void getCenterPoint(double& x, double& y) const {x=_x0[0]; y=_x0[1];}
  //! Get 3D center point.
  void getCenterPoint(double& x, double& y, double& z) const {x=_x0[0]; y=_x0[1]; z=_x0[2];}

  //! Returns the template direction rotated (dPhi,dTheta) radians in a spherical coordinate system.
  void rotate(double theta);
  void rotate(double theta, double phi);

  //! Set and get score
  void setScore(double score) {_score = score;}
  double getScore(void) const {return _score;}

  //! Set this model as terminator.
  bool isTerminator(void) const {return _isTerminator;}
  //! Set this model as terminator.
  void setTerminator(void) {_isTerminator = true;}

  //! Check if this model is in a branching region.
  bool isInBranchingRegion(void) const {return _isInBranchingRegion;}
  //! Set this model as being in a branching region.
  void setBranchingState(bool branchState) {_isInBranchingRegion = branchState;}

  //! Set distance to junction or start.
  void setDistance(double distance) {_distance = distance;}
  //! Get distance to junction or start.
  double getDistance(void) const {return _distance;}

  //! Set when model has been fitted to image data.
  void setFitted(void) {_isFitted = true;}
  //! Check if model has been fitted.
  bool isFitted(void) const {return _isFitted;}

  //! DEBUG INFORMATION
  //std::valarray<double> DEBUG_W;
  //std::valarray<double> DEBUG_I;
  //std::valarray<double> DEBUG_T;
  //int DEBUG1,DEBUG2,DEBUG3;

protected:
  //! Get derivatives of vessel direction v=(vx, vy, vz) with respect to (theta,phi) angles, i.e.,
  //! the spherical coordinates representation.
  void _getDerivatives(double& dvx_dtheta, double& dvy_dtheta) const
  {dvx_dtheta=_dvx_dtheta; dvy_dtheta=_dvy_dtheta;}
  void _getDerivatives(double& dvx_dtheta, double& dvy_dtheta, double& dvx_dphi, double& dvy_dphi, double& dvz_dphi) const
  {dvx_dtheta=_dvx_dtheta; dvy_dtheta=_dvy_dtheta; dvx_dphi=_dvx_dphi; dvy_dphi=_dvy_dphi; dvz_dphi=_dvz_dphi;}

  //! Model specific parameters. For example, for a template model, _p1 is used for contrast and _p2 for mean image level.
  double _p1,_p2;

private:
  //!  Tubular radii for circle or ellipse.
  double _radius1;
  double _radius2;

  //! Tubular direction. Should always be normalized to unit length
  //! _theta and _phi are polar representations of the direction.
  double _v[3];
  double _theta;
  double _phi;

  //! Direction orthogonal to the tubular direction in _v. Should also be normalized to unit length.
  double _u1[3];
  double _u2[3];

  //! Center point of template
  double _x0[3];

  //! A score that says how well the template fits to the image.
  double _score;

  //! Derivative of the tubular direction _v with respect to  a change of the angle theta in spherical coordinates.
  //!
  //!      [sin(phi) * cos(theta)]
  //!  v = [sin(phi) * sin(theta)]
  //!      [      cos(phi)       ]
  //! _dvz_dtheta == 0, so it is not needed.
  double _dvx_dtheta;
  double _dvy_dtheta;

  //! Derivative of the tubular direction _v with respect to a change of the angle phi in spherical coordinates.
  double _dvx_dphi;
  double _dvy_dphi;
  double _dvz_dphi;

  //! _isTerminator is set to true if the tracker has attempted to find a continuation
  //! from this vessel segment but failed. This means that we don't have to try this again.
  bool _isTerminator;

  //! This variable is set if this model is located in a branching region.
  bool _isInBranchingRegion;

  //! Length of branch counted from initial point or last junction.
  double _distance;

  bool _isFitted;

};

#endif
