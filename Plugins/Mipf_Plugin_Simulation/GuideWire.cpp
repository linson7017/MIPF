#include "GuideWire.h"


#include <vtkDataArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntersectionPolyDataFilter.h>
#include <vtkLineSource.h>

#include <vtkPlane.h>
#include <vtkCell.h>
#include <vtkMath.h>

#include <VTKImageProperties.h>
#include <vtkGenericCell.h>

#include "Common/Common.h"
#include "Demos/Utils/Logger.h"
#include "Demos/Utils/Timing.h"

INIT_TIMING
INIT_LOGGING


GuideWireParams::GuideWireParams() : m_rodParams(NULL)
{ }

GuideWireParams::~GuideWireParams()
{
    if (m_rodParams != NULL)
    {
        delete m_rodParams;
    }
}


GuideWire::GuideWire()
{
    //    init default Rod params
    m_params = new GuideWireParams();
    m_params->m_length = 6;
    m_params->m_lengthVariance = 1;
    m_params->m_helicalRadius = 0.3;
    m_params->m_helicalPitch = 0.15;
    m_params->m_density = 0.1;
    m_params->m_thickness = 0.07;
    m_params->m_nParticles = 15;

    m_params->m_gravity.set(0, 5.0, 0);
    m_params->m_drag = 0.0000003;

    m_params->m_resolveCollisions = 1;
    m_params->m_coulombFriction = 0.2;

    m_params->m_resolveSelfInterations = 1;
    m_params->m_selfInterationDist = 0.4;
    m_params->m_selfStiction = 0.2;
    m_params->m_selfRepulsion = 0.000005;

    m_params->m_pbdIter = 6;

    mg::Real bendStiffness = 10.0;
    mg::Real twistStiffness = 0.05;
    mg::Real maxElasticForce = 1000;
    m_params->m_rodParams = new ElasticRodParams(bendStiffness, twistStiffness, maxElasticForce, ElasticRodParams::NONE);

    m_params->m_movingVel = mg::Vec3D(0, 0, 0);


    m_subject = QF::QF_CreateSubjectObject();

    m_startPoint = 0;
    m_endPoint = 1;
}


GuideWire::~GuideWire()
{
    reset();
    delete m_params;
}

void GuideWire::reset()
{
    if (m_grid != NULL)
    {
        delete m_grid;
    }
    delete m_strand;
    m_grid = NULL;
}

void GuideWire::initialize()
{
//  assert(m_object != NULL);
//    assert(m_grid == NULL);

    /*mg::Vec3D gridCenter = m_object->getCenter();
    mg::Vec3D offset = (m_object->getBoundingRadius() + m_params->m_length + m_params->m_lengthVariance) * mg::Vec3D(1, 1, 1);
    m_volume.reshape(gridCenter - offset, gridCenter + offset);

    m_grid = new VoxelGrid(m_volume, m_volume.getWidth() / m_params->m_selfInterationDist);
    m_grid->initialize();*/
    for (int i=0;i<m_objects.size();i++)
    {
        vtkSmartPointer<vtkCellLocator> locator = vtkSmartPointer<vtkCellLocator>::New();
        locator->SetDataSet(m_objects[i]);
        locator->BuildLocator();
        m_locators.push_back(locator);
    }

    m_startPoint = 0;
    m_endPoint = m_strand->m_ppos.size() ;
}

void GuideWire::resetGrid()
{
   // mg::Vec3D gridCenter = m_object->getCenter();
  //  mg::Vec3D offset = (m_object->getBoundingRadius() + m_params->m_length + m_params->m_lengthVariance) * mg::Vec3D(1, 1, 1);
    //m_volume.reshape(gridCenter - offset, gridCenter + offset);

   /* m_grid->reset();

    for (unsigned i = 1; i < m_strand->m_ppos.size(); ++i)
    {
        m_grid->insertDensity(m_strand->m_ppos[i], 1);
    }

    for (unsigned i = 1; i < m_strand->m_ppos.size(); ++i)
    {
        m_grid->insertVelocity(m_strand->m_ppos[i], m_strand->m_pvel[i]);
    }*/


}

void GuideWire::update(mg::Real dt)
{
    //assert(m_object != NULL);
  //  assert(m_grid != NULL);

    m_dt = dt;
    if (m_params->m_resolveSelfInterations)
    {
        resetGrid();
    }

#ifdef MULTI_THREAD
#pragma omp parallel for
#endif

    //m_strands[i]->m_ppos[0] = mg::transform_point(m_object->getTransform(), mesh->m_vertices[m_vindices[i]]);
    //updateRod(*m_strands[i], dt);
    updateRod(*m_strand, dt);
}

void GuideWire::printself()
{
    m_strand->printself();
}

/* semi-implicit Euler with Verlet scheme for velocity update */
void GuideWire::updateRod(ElasticRod& rod, mg::Real dt) const
{
    rod.m_pprepos = rod.m_ppos;

    std::vector<mg::Vec3D> forces(rod.m_ppos.size(), mg::Vec3D(0, 0, 0));
    rod.accumulateInternalElasticForces(forces);

    if (m_params->m_resolveSelfInterations)
    {
        accumulateExternalForcesWithSelfInterations(rod, forces);
    }
    else
    {
        accumulateExternalForces(rod, forces);
    }

    //    integrate centerline - semi- implicit Euler
    std::vector<mg::Vec3D> prevPos(rod.m_ppos.size());
    for (unsigned i = m_startPoint; i < m_endPoint; ++i)
    {
        prevPos[i] = rod.m_ppos[i];
        rod.m_pvel[i] += dt * forces[i] / rod.m_pmass[i];
        rod.m_pvel[i] += m_params->m_movingVel;
        rod.m_ppos[i] += rod.m_pvel[i] * dt;
    }

    if (m_params->m_resolveCollisions)
    {
        enforceConstraintsWithCollision(rod);
    }
    else
    {
        enforceConstraints(rod);
    }

    //    velocity correction using Verlet scheme:
    for (unsigned i = m_startPoint; i < m_endPoint; ++i)
    {
        rod.m_pvel[i] = (rod.m_ppos[i] - rod.m_pprepos[i]) / dt;
    }

    rod.updateCurrentState();
}

void GuideWire::enforceConstraints(ElasticRod& rod) const
{
    for (unsigned k = 0; k < m_params->m_pbdIter; ++k)
    {
        rod.applyInternalConstraintsIteration();
    }
}

void GuideWire::enforceConstraintsWithCollision(ElasticRod& rod) const
{
    for (unsigned k = 0; k < m_params->m_pbdIter; ++k)
    {
        //内力放后面会影响碰撞检测的结果，先把碰撞检测放后面吧
        rod.applyInternalConstraintsIteration();
        applyCollisionConstraintsIteration(rod);
    }
}


bool isInside(const mg::Vec3D& p, vtkImageData* img)
{
    double spacing[3];
    double origin[3];
    img->GetSpacing(spacing);
    img->GetOrigin(origin);
    int i = round((p[0] - origin[0]) / spacing[0]);
    int j = round((p[1] - origin[1]) / spacing[1]);
    int k = round((p[2] - origin[2]) / spacing[2]);

    return static_cast<unsigned char>(img->GetScalarComponentAsFloat(i, j, k, 0)) == 1;

}


bool GuideWire::isCollided(vtkCellLocator* locator,vtkPolyData* polydata, mg::Vec3D& current,mg::Vec3D& pre, mg::Vec3D& o_collisionPoint, mg::Vec3D& o_normal)     const
{
  
    o_collisionPoint = current;
    //Find the closest points to TestPoint
    mg::Vec3D n = pre - current;
    mg::Vec3D p1 = pre + n.normalize() * 1.0;
    mg::Vec3D p2 = current - n.normalize() * 1.0;

    double point0[3] = { p1.data()[0],p1.data()[1] ,p1.data()[2] };
    double point1[3] = { p2.data()[0],p2.data()[1] ,p2.data()[2] };
    double closestPoint[3] = { current.data()[0],current.data()[1] ,current.data()[2] };
    double closestPointDist2; 
    vtkIdType cellId; 
    int subId;
    double tolerance = 1.0;
    //locator->FindClosestPoint(point0, closestPoint, cellId, subId, closestPointDist2);
    auto  cellIDs = vtkSmartPointer<vtkIdList>::New();

    double t = 0.0;
    double pline[3] = { 0, 0, 0 };
    double pcoords[3] = { 0, 0, 0 };
    auto cell = vtkSmartPointer<vtkGenericCell>::New();

    if (locator->IntersectWithLine(point0, point1, 0.001, t, pline, pcoords, subId, cellId, cell) != 0) 
    {
        // Enters here if there is intersection
        o_collisionPoint.set(pline[0], pline[1], pline[2]);
        //calculate normals
        vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
        if (normalDataFloat)
        {
            double* normal = normalDataFloat->GetTuple(cellId);
            o_normal.set(normal[0], normal[1], normal[2]);
            o_normal.normalize();
        }
        return true;
    }
    else
    {
        return false;
    }

    //vtkFloatArray* normalDataFloat =
    //    vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
    //double* normal = normalDataFloat->GetTuple(cellId);
    //o_normal.set(normal[0], normal[1], normal[2]);
    //if (normalDataFloat)
    //{
    //    vtkCell* cell = polydata->GetCell(cellId);
    //    double tolerance = 0.001;
    //    // Outputs
    //    double t; // Parametric coordinate of intersection (0 (corresponding to p1) to 1 (corresponding to p2))
    //    double pcoords[3];
    //    int subId;
    //    if (cell->IntersectWithLine(point0, point1, tolerance, t, closestPoint, pcoords, subId))
    //    {
    //        o_collisionPoint.set(closestPoint[0], closestPoint[1], closestPoint[2]);
    //        return true;
    //    }
    //    else
    //    {
    //        return false;
    //    }
    //}
    //else
    //{
    //    return false;
    //}

}

void GuideWire::applyCollisionConstraintsIteration(ElasticRod& rod) const
{
    mg::Vec3D collision_p, normal;
    for (unsigned i = m_startPoint; i <m_endPoint; ++i)
    {
        for (unsigned j=0;j<m_objects.size();j++)
        {
            if (!isInside(rod.m_ppos[i],m_imgs[j].Get()))
            {
                if (isInside(rod.m_pprepos[i], m_imgs[j].Get()))
                {
                    if (isCollided(m_locators[j].Get(), m_objects[j], rod.m_ppos[i], rod.m_pprepos[i], collision_p, normal))
                    {
                        
                        rod.m_ppos[i] = rod.m_pprepos[i]+(m_dt/ (float)m_params->m_pbdIter)* (rod.m_pvel[i] - 2 * mg::dot(rod.m_pvel[i], normal)*normal);
                       
                    }
                }
                if (i > 0)
                {
                    if (isInside(rod.m_ppos[i - 1], m_imgs[j].Get()))
                    {
                        if (isCollided(m_locators[j].Get(), m_objects[j], rod.m_ppos[i], rod.m_ppos[i - 1], collision_p, normal))
                        {
                            //rod.m_ppos[i] = rod.m_ppos[i-1] + (collision_p-rod.m_ppos[i-1]).normalize()* (rod.m_ppos[i-1] - rod.m_ppos[i]).length();
                            rod.m_ppos[i] = rod.m_pprepos[i];
                        }
                    }
                }
            }
        }     
    }
}

void GuideWire::accumulateExternalForcesWithSelfInterations(ElasticRod& rod, std::vector<mg::Vec3D>& o_forces) const
{
    mg::Vec3D p1(0, 0, 0), p2(0, 0, 0), pressureGradient(0, 0, 0);
    mg::Real dr = m_params->m_selfInterationDist;
    mg::Real density_p1, density_p2;

    for (unsigned i = m_startPoint; i < m_endPoint; ++i)
    {
        //        gravity
        o_forces[i] += m_params->m_gravity * rod.m_pmass[i];
        //        drag
        o_forces[i] -= m_params->m_drag * rod.m_pvel[i].length() * rod.m_pvel[i];


        //        self interactions:

        //        self stiction acts as averaging of the velocity for near by particles
        //        the velocity however is not direcly averaged by trilinear interpolation is used to calculate the avg. value for particle's current position
        //m_grid->getInterpolatedDensity(rod.m_ppos[i], density_p1);
        //if (density_p1 > mg::ERR)
        //{
        //    m_grid->getInterpolatedVelocity(rod.m_ppos[i], p1);
        //    rod.m_pvel[i] = (1 - m_params->m_selfStiction) * rod.m_pvel[i] + m_params->m_selfStiction * p1 / density_p1;
        //}

        ////        self repulsion
        //p1.set(rod.m_ppos[i][0] - dr, rod.m_ppos[i][1], rod.m_ppos[i][2]);
        //p2.set(rod.m_ppos[i][0] + dr, rod.m_ppos[i][1], rod.m_ppos[i][2]);
        //m_grid->getInterpolatedDensity(p1, density_p1);
        //m_grid->getInterpolatedDensity(p2, density_p2);
        //pressureGradient[0] = (density_p1 - density_p2) * 0.5 / dr;

        //p1.set(rod.m_ppos[i][0], rod.m_ppos[i][1] - dr, rod.m_ppos[i][2]);
        //p2.set(rod.m_ppos[i][0], rod.m_ppos[i][1] + dr, rod.m_ppos[i][2]);
        //m_grid->getInterpolatedDensity(p1, density_p1);
        //m_grid->getInterpolatedDensity(p2, density_p2);
        //pressureGradient[1] = (density_p1 - density_p2) * 0.5 / dr;

        //p1.set(rod.m_ppos[i][0], rod.m_ppos[i][1], rod.m_ppos[i][2] - dr);
        //p2.set(rod.m_ppos[i][0], rod.m_ppos[i][1], rod.m_ppos[i][2] + dr);
        //m_grid->getInterpolatedDensity(p1, density_p1);
        //m_grid->getInterpolatedDensity(p2, density_p2);
        //pressureGradient[2] = (density_p1 - density_p2) * 0.5 / dr;

        //o_forces[i] += m_params->m_selfRepulsion * pressureGradient;

    }
}

void GuideWire::accumulateExternalForces(const ElasticRod& rod, std::vector<mg::Vec3D>& o_forces) const
{
    for (unsigned i = m_startPoint; i < m_endPoint; ++i)
    {
        //        gravity
        o_forces[i] += m_params->m_gravity * rod.m_pmass[i];
        //        drag
        o_forces[i] -= m_params->m_drag * rod.m_pvel[i].length() * rod.m_pvel[i];
    }
}
