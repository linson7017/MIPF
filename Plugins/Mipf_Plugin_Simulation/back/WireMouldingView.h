#ifndef WireMouldingView_h__
#define WireMouldingView_h__
/********************************************************************
	FileName:    WireMouldingView
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "MitkPluginView.h"
#include <QWidget>
#include "ui_WireMouldingView.h"
#include "vtkPointLocator.h"
#include "vtkCellLocator.h"
#include "vtkMath.h"
#include <vtkSelectEnclosedPoints.h>

struct WirePoint
{
    WirePoint()
    {
        for (int i=0;i<3;i++)
        {
            position[i] = 0.0;
            direction[i] = 0.0;
        }
        isContact = false;
        normalTheta = 0;
        dK = 0.0;
    }
    WirePoint(double* p)
    {
        SetPosition(p);
        for (int i = 0; i < 3; i++)
        {
            direction[i] = 0.0;
        }
        isContact = false;
        normalTheta = 0.0;
        dK = 0.0;
    }
    WirePoint(double* p,double* d)
    {
        SetPosition(p);
        SetDirection(d);
        isContact = false;
        normalTheta = 0.0;
        dK = 0.0;
    }     
    WirePoint(const WirePoint& point)
    {
        SetPosition(point.position);
        SetDirection(point.direction);
        isContact = point.isContact;
        normalTheta = point.normalTheta;
        dK = point.dK;
    }
    double position[3];
    double direction[3];
    bool isContact;
    double normalTheta;
    double dK;
    void SetPosition(const double* p)
    {
        position[0] = p[0];
        position[1] = p[1];
        position[2] = p[2];
    }
    void SetDirection(const double* d)
    {
        direction[0] = d[0];
        direction[1] = d[1];
        direction[2] = d[2];
    }
    void CaculateContactAngle(double* normal)
    {
        vtkMath::Normalize(normal);
        vtkMath::Normalize(direction);
        normalTheta = acos(vtkMath::Dot(direction,normal))*180.0/vtkMath::Pi();
    }
};


class WireMouldingView:public QWidget,public MitkPluginView
{                                           
    Q_OBJECT
public:
    WireMouldingView();
    ~WireMouldingView();
    void CreateView();
    WndHandle GetPluginHandle() { return this; }
    typedef  std::vector<WirePoint> WirePointsType;
protected slots:
    void Apply();
    void SplineParameterChanged(double value);
private:
    void Moulde(const mitk::Point3D& start,const mitk::Vector3D& direct,double length,WirePointsType& pathPoints);
    bool Advance(const mitk::Point3D& currentPoint, const mitk::Vector3D& direction, mitk::Point3D& nextPoint);
    static bool IsInside(double* p, vtkImageData* img);
    bool IsInside(double*p);
    bool isCollided(vtkCellLocator* locator, vtkPolyData* polydata, mitk::Point3D& current, mitk::Point3D& pre, 
        mitk::Point3D& o_collisionPoint, mitk::Vector3D& o_normal)     const;
    void ProcessCollision(mitk::Point3D& current, mitk::Point3D& next, mitk::Point3D& collided, mitk::Vector3D& normal);
    bool CaculateFanIntersect(mitk::Point3D& center, mitk::Vector3D& startDirection,double radius, mitk::Vector3D& normal, mitk::Point3D& intersectPoint);
    void RotateVector(double angle, mitk::Vector3D& rotateAxis, mitk::Vector3D& o_vector);
    void RotateVector(double angle, double* rotateAxis, double* o_vector);
    bool GetNearestCenterLinePointAndDirection(const mitk::Point3D& p, mitk::Point3D& nearest,mitk::Vector3D& direct,double& radius);
    void ExtractNeckProfile(double* planePoint,double* planeNormal,vtkPolyData* vesselData);
    void Bend(double bendRatio,double length, WirePointsType& wire);
    void Smooth(double angleThreshold, WirePointsType& wire);

    void DisplayDirectedWire( WirePointsType& wire,const char* name="");
private:
    Ui::WireMoudlingView m_ui;

    mitk::Point3D m_lxPoint;
    mitk::Point3D m_ljPoint;
    mitk::Point3D m_entryPoint;
    mitk::Point3D m_entryDirectionPoint;

    mitk::Surface* m_centerLine;
    mitk::Surface* m_vessel;
    mitk::Surface* m_intersect;
    mitk::Image* m_mask;

    vtkSmartPointer<vtkPointLocator> m_pointLocator;
    vtkSmartPointer<vtkCellLocator> m_cellLocator;
    vtkSmartPointer<vtkPolyData> m_centerLinePointsPolyData;
    vtkSmartPointer<vtkSelectEnclosedPoints> m_enclosedPoints;
    vtkSmartPointer<vtkCellLocator> m_capCellLocator;
    vtkSmartPointer<vtkPolyData> m_capedVessel;
    vtkSmartPointer<vtkPoints> m_pathPoints;

   // mitk::DataNode::Pointer m_helperNode;
    WirePointsType m_enterPathPoints;
    WirePointsType m_returnPathPoints;

    double m_addvanceLength;
    double m_bendingAngle;
    double m_wireRadius;
    bool m_invert;
};

#endif // WireMouldingView_h__
