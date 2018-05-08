#ifndef GuideWire_h__
#define GuideWire_h__
/********************************************************************
	FileName:    GuideWire
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "DER/ElasticRod.h"
#include "DER/RenderObject.h"
#include "DER/VoxelGrid.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

#include "iqf_subject.h"

#include <vtkCellLocator.h>

struct GuideWireParams
{
    GuideWireParams();
    ~GuideWireParams();

    mg::Real m_length;
    mg::Real m_lengthVariance;
    mg::Real m_helicalRadius;
    mg::Real m_helicalPitch;
    mg::Real m_density;
    mg::Real m_thickness;
    unsigned m_nParticles;

    mg::Vec3D m_gravity;
    mg::Real m_drag;
    mg::Vec3D m_movingVel;

    bool m_resolveCollisions;
    mg::Real m_coulombFriction;

    bool m_resolveSelfInterations;
    mg::Real m_selfInterationDist;
    mg::Real m_selfStiction;
    mg::Real m_selfRepulsion;

    ///     constraints are enforced using PBD(Position Based Dynamics) framework
    ///     the parameter controls # of PBD iterations to be performed
    ///     NOTE that PBD DOES NOT GUARANTEE exact enforcement but converges towards the solution
    ///     higher value of iterations means higher precision but is more computationally expensive
    unsigned m_pbdIter;

    ElasticRodParams* m_rodParams;
};

class GuideWire   
{
public:
    GuideWire();
    ~GuideWire();

    void initialize();
    void reset();
    void update(mg::Real dt);
    void printself();

    GuideWireParams* m_params;
    std::vector<vtkSmartPointer<vtkPolyData>> m_objects;
    std::vector<vtkSmartPointer<vtkImageData>> m_imgs;
    std::vector<vtkSmartPointer<vtkCellLocator>> m_locators;
    ElasticRod* m_strand;
    QF::IQF_Subject* m_subject;
private:

    void resetGrid();
    void updateRod(ElasticRod& rod, mg::Real dt) const;
    void accumulateExternalForces(const ElasticRod &rod, std::vector<mg::Vec3D>& o_forces) const;
    void accumulateExternalForcesWithSelfInterations(ElasticRod &rod, std::vector<mg::Vec3D>& o_forces) const;
    void enforceConstraints(ElasticRod& rod) const;
    void enforceConstraintsWithCollision(ElasticRod& rod) const;
    void applyCollisionConstraintsIteration(ElasticRod& rod) const;


    bool isCollided(vtkCellLocator* locator, vtkPolyData* polydata, mg::Vec3D& current, mg::Vec3D& pre, mg::Vec3D& o_collisionPoint, mg::Vec3D& o_normal) const;
private:

    unsigned m_id;

    AABB m_volume;
    VoxelGrid* m_grid;

    int m_startPoint;
    int m_endPoint;

    mg::Real m_dt;
};

#endif // GuideWire_h__
