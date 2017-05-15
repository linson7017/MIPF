#ifndef __VesselTemplate_H
#define __VesselTemplate_H

// Get rid of the min() / max () macro definitions in Visual Studio
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "VesselData.h"
#include "VesselProfile.h"
#include "ImPatch.h"
#include <valarray>
#include <assert.h>

#define MAXSIZE 100

class VesselTemplate : public VesselData, public VesselProfile
{
public:
  //! Standard constructor.
  VesselTemplate(void);

  //! Standard destructor.
  virtual ~VesselTemplate() {};

  //! Constructor for converting a VesselData object into a
  //! VesselTemplate object.
  VesselTemplate(const VesselData& vd);

  //! Get template value for the input coordinate (x,y,z).
  //! The weight W acts as a mask here, the template is calculated for all non-zero W.
  void getValue(ImPatch& T,   const ImPatch& W);
  void getValue2D(ImPatch& T, const ImPatch& W);
  void getValue3D(ImPatch& T, const ImPatch& W);

  //! Get template value and derivatives 2D.
  void  getDerivatives(std::valarray<double>& dT_dr,
                       std::valarray<double>& dT_du,
                       std::valarray<double>& dT_dtheta,
                       const std::valarray<double>& T,
                       double xstart, double dx, int xsize,
                       double ystart, double dy, int ysize);

  //! Get template value and derivatives 3D.
  void  getDerivatives(std::valarray<double>& dT_dr,
    std::valarray<double>& dT_du1,
    std::valarray<double>& dT_du2,
    std::valarray<double>& dT_dtheta,
    std::valarray<double>& dT_dphi,
    const std::valarray<double>& T,
    double xstart, double dx, int xsize,
    double ystart, double dy, int ysize,
    double zstart, double dz, int zsize);

  //! Set contrast. Use protected member in VesselData class for this.
  void setContrast(double contrast) {_p1 = contrast;}
  //! Get contrast.
  double getContrast(void) const {return _p1;}

  //! Set mean. Use protected member in VesselData class for this.
  void setMean(double mean) {_p2 = mean;}
  //! Get mean.
  double getMean(void) const {return _p2;}

};
#endif
