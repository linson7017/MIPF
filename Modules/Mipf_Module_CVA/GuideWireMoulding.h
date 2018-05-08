#ifndef GuideWireMoulding_h__
#define GuideWireMoulding_h__
/********************************************************************
	FileName:    GuideWireMoulding
	Author:        Ling Song
	Date:           Month 5 ; Year 2018
	Purpose:	     
*********************************************************************/
//vtk
#include <vtkPolyData.h>
#include <vtkPointLocator.h>
#include <vtkCellLocator.h>
#include <vtkSmartPointer.h>

//itk
#include <itkPoint.h>
#include <itkVector.h>

#include "CVA/IQF_GuideWireMoulding.h"

class GuideWireMoulding :public  IQF_GuideWireMoulding
{
public:
    GuideWireMoulding();
    ~GuideWireMoulding();

    virtual void SetSurface(vtkPolyData* pSurface);
    virtual void SetCenterLine(vtkPolyData* pCenterLine,bool bDirectionInvert=false);
    virtual void SetAdvanceStep(double dStepLength);
    virtual void SetGuideWireRadius(double dRadius);
    virtual void SetBendFactor(double dBendFactor);
    virtual void Simulate(double* pEntryPoint, double* pAdvanceDirection,double dLength, WirePointsType& pathPoints);
    virtual void Moulde(double dReleaseRatio, double dMouldeLength, WirePointsType& pathPoints);
    virtual void Smooth(double dAngleThreshold, WirePointsType& pathPoints);
    virtual void Release() { delete this; }
private:
    bool Advance(const itk::Point<double>& currentPoint, const itk::Vector<double>& direction, itk::Point<double>& nextPoint);
    bool isCollided(vtkCellLocator* locator, vtkPolyData* polydata, itk::Point<double>& current, itk::Point<double>& pre,
        itk::Point<double>& o_collisionPoint, itk::Vector<double>& o_normal)     const;
    void ProcessCollision(itk::Point<double>& current, itk::Point<double>& next, itk::Point<double>& collided, itk::Vector<double>& normal);
    bool CaculateFanIntersect(itk::Point<double>& center, itk::Vector<double>& startDirection, double radius, itk::Vector<double>& normal, itk::Point<double>& intersectPoint);
    void RotateVector(double angle, itk::Vector<double>& rotateAxis, itk::Vector<double>& o_vector);
    void RotateVector(double angle, double* rotateAxis, double* o_vector);
    bool GetNearestCenterLinePointAndDirection(const itk::Point<double>& p, itk::Point<double>& nearest, itk::Vector<double>& direct, double& radius);
    void Initialize();
    void InitializeCenterLine();
    void InitializeSurface();
private:
    double m_addvanceLength;
    double m_bendingAngle;
    double m_wireRadius;
    bool m_invert;
    bool m_bInitialized;
    double m_bendFactor;

    vtkSmartPointer<vtkPolyData> m_centerLine;
    vtkSmartPointer<vtkPolyData> m_vessel;
    vtkSmartPointer<vtkPointLocator> m_pointLocator;
    vtkSmartPointer<vtkCellLocator> m_cellLocator;
    vtkSmartPointer<vtkPolyData> m_centerLinePointsPolyData;

};

#endif // GuideWireMoulding_h__
