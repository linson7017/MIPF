#ifndef vtkPolyDataCollisionDetection_h__
#define vtkPolyDataCollisionDetection_h__
/********************************************************************
	FileName:    vtkPolyDataCollisionDetection
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "Common/Common.h"
#include "Demos/Simulation/DistanceFieldCollisionDetection.h"
#include "Demos/Simulation/AABB.h"
#include "Demos/Simulation/BoundingSphereHierarchy.h"
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace PBD
{

class vtkPolyDataCollisionDetection :public DistanceFieldCollisionDetection
{
public:
    struct vtkPolyDataCollisionObject : public DistanceFieldCollisionObject
    {
        bool m_testMesh;
        Real m_invertSDF;
        vtkSmartPointer<vtkPolyData> m_polydata;

        vtkPolyDataCollisionObject() { m_testMesh = true; m_invertSDF = 1.0; }
        virtual ~vtkPolyDataCollisionObject() {}
        virtual bool collisionTest(const Vector3r &x, const Real tolerance, Vector3r &cp, Vector3r &n, Real &dist, const Real maxDist = 0.0);
        virtual void approximateNormal(const Eigen::Vector3d &x, const Real tolerance, Vector3r &n);
        virtual Real distance(const Eigen::Vector3d &x, const Real tolerance) = 0;
    };

public:
    vtkPolyDataCollisionDetection();
    virtual ~vtkPolyDataCollisionDetection();

    virtual void collisionDetection(SimulationModel &model);

};


}

#endif // vtkPolyDataCollisionDetection_h__
