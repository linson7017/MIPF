#ifndef IQF_GuideWireMoulding_h__
#define IQF_GuideWireMoulding_h__
/********************************************************************
	FileName:    IQF_GuideWireMoulding
	Author:        Ling Song
	Date:           Month 5 ; Year 2018
	Purpose:	 Used for guidewire path simulation in vessel and the top moulding    
*********************************************************************/
#include "IQF_Object.h"
#include <vector>

const char Object_ID_GuideWireMoulding[] = "Object_ID_GuideWireMoulding";

class vtkPolyData;
struct WirePoint
{
    WirePoint();
    WirePoint(double* dPosition);
    WirePoint(double* dPosition, double* dDirection);
    WirePoint(const WirePoint& point);
    void SetPosition(const double* dPosition);
    void SetDirection(const double* dDirection);
    double position[3];
    double direction[3];
    bool isContact;
    double normalTheta;
    double dK;
};

typedef  std::vector<WirePoint> WirePointsType;
class IQF_GuideWireMoulding :public IQF_Object
{
public:
    /*********************************************************************************
      *Function:SetSurface
      * Description£ºSet the surface. Must be set !
    **********************************************************************************/
    virtual void SetSurface(vtkPolyData* pSurface) = 0;
    /*********************************************************************************
    *Function:SetCenterLine
    * Description£ºSet the centerline of the  surface. Must be set !
    **********************************************************************************/
    virtual void SetCenterLine(vtkPolyData* pCenterLine, bool bDirectionInvert = false) = 0;
    /*********************************************************************************
    *Function:SetAdvanceStep
    * Description£ºSet the advance step. Optional !
    ************************************************************************¡¤**********/
    virtual void SetAdvanceStep(double dStepLength) = 0;
    /*********************************************************************************
    *Function:SetGuideWireRadius
    * Description£ºSet the guide wire radius. Optional !
    **********************************************************************************/
    virtual void SetGuideWireRadius(double dRadius) = 0;
    /*********************************************************************************
    *Function:SetGuideWireRadius
    * Description£ºSet the bend factor(0.0~1.0) which control the bend of the wire. Optional !
    **********************************************************************************/
    virtual void SetBendFactor(double dBendFactor) = 0;
    /*********************************************************************************
      *Function:Simulation
      * Description£ºStart Simulation
      *Input:  pEntryPoint ---The start position
                  pAdvanceDirection --- The start direction
                  dLength --- Length of the path
      *Output: pathPoints --- Sequential position of the path 
      *Return: void 
    **********************************************************************************/
    virtual void Simulate(double* pEntryPoint, double* pAdvanceDirection, double dLength, WirePointsType& pathPoints)=0;
    /*********************************************************************************
    *Function:Moulde
    * Description£ºStart Moulding !
    *Input:  dReleaseRatio ---  Springback Ratio 
                dMouldeLength --- Length to moulde
    *Output: pathPoints --- Sequential position of the path  after moulding
    **********************************************************************************/
    virtual void Moulde(double dReleaseRatio, double dMouldeLength, WirePointsType& pathPoints) = 0;
    /*********************************************************************************
    *Function:Smooth
    * Description£ºSmooth the path
    *Input:  dAngleThreshold --- Rotation angle which is greater than dAngleThreshold will be smooth using Bezier
    *Output: pathPoints --- Sequential position of the path  after smoothing
    **********************************************************************************/
    virtual void Smooth(double dAngleThreshold, WirePointsType& pathPoints) = 0;
};



inline WirePoint::WirePoint()
{
    for (int i = 0; i < 3; i++)
    {
        position[i] = 0.0;
        direction[i] = 0.0;
    }
    isContact = false;
    dK = 0.0;
}
inline WirePoint::WirePoint(double* dPosition)
{
    SetPosition(dPosition);
    for (int i = 0; i < 3; i++)
    {
        direction[i] = 0.0;
    }
    isContact = false;
    normalTheta = 0.0;
    dK = 0.0;
}
inline WirePoint::WirePoint(double* dPosition, double* dDirection)
{
    SetPosition(dPosition);
    SetDirection(dDirection);
    isContact = false;
    normalTheta = 0.0;
    dK = 0.0;
}
inline WirePoint::WirePoint(const WirePoint& point)
{
    SetPosition(point.position);
    SetDirection(point.direction);
    isContact = point.isContact;
    normalTheta = point.normalTheta;
    dK = point.dK;
}

inline void WirePoint::SetPosition(const double* dPosition)
{
    for (int i = 0; i < 3; i++)
    {
        position[i] = dPosition[i];
    }
}
inline void WirePoint::SetDirection(const double* dDirection)
{
    for (int i = 0; i < 3; i++)
    {
        direction[i] = dDirection[i];
    }
}

#endif // IQF_GuideWireMoulding_h__
