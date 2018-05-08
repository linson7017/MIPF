#include "Rod.h"

#include "Utils.h"


RodParams::RodParams() : m_rodParams(NULL)
{ }

RodParams::~RodParams()
{
    if (m_rodParams != NULL)
    {
        delete m_rodParams;
    }
}

Rod::Rod() : m_id(-1), m_grid(NULL)
{
    //    init default Rod params
    m_params = new RodParams();
    m_params->m_length = 6;
    m_params->m_lengthVariance = 1;
    m_params->m_helicalRadius = 0.3;
    m_params->m_helicalPitch = 0.15;
    m_params->m_density = 0.001;
    m_params->m_thickness = 0.07;
    m_params->m_nParticles = 15;

    m_params->m_gravity.set(0, -9.81, 0);
    m_params->m_drag = 0.0000003;

    m_params->m_resolveCollisions = 1;
    m_params->m_coulombFriction = 0.2;

    m_params->m_resolveSelfInterations = 1;
    m_params->m_selfInterationDist = 0.4;
    m_params->m_selfStiction = 0.2;
    m_params->m_selfRepulsion = 0.000005;

    m_params->m_pbdIter = 6;

    mg::Real bendStiffness = 0.00006;
    mg::Real twistStiffness = 0.0005;
    mg::Real maxElasticForce = 1000;
    m_params->m_rodParams = new ElasticRodParams(bendStiffness, twistStiffness, maxElasticForce, ElasticRodParams::NONE);
}

Rod::~Rod()
{
    reset();
    delete m_params;
}

void Rod::reset()
{
    if (m_grid != NULL)
    {
        delete m_grid;
    }
    typedef std::vector<ElasticRod*>::iterator Iter;
    for (Iter it = m_strands.begin(); it != m_strands.end(); ++it)
    {
        delete (*it);
    }
    m_strands.clear();
    m_findices.clear();
    m_vindices.clear();
    m_grid = NULL;
}

void Rod::initialize()
{
    assert(m_object != NULL);
    assert(m_grid == NULL);

    mg::Vec3D gridCenter = m_object->getCenter();
    mg::Vec3D offset = (m_object->getBoundingRadius() + m_params->m_length + m_params->m_lengthVariance) * mg::Vec3D(1, 1, 1);
    m_volume.reshape(gridCenter - offset, gridCenter + offset);

    m_grid = new VoxelGrid(m_volume, m_volume.getWidth() / m_params->m_selfInterationDist);
    m_grid->initialize();
}

void Rod::resetGrid()
{
    mg::Vec3D gridCenter = m_object->getCenter();
    mg::Vec3D offset = (m_object->getBoundingRadius() + m_params->m_length + m_params->m_lengthVariance) * mg::Vec3D(1, 1, 1);
    m_volume.reshape(gridCenter - offset, gridCenter + offset);

    m_grid->reset();

    typedef std::vector<ElasticRod*>::iterator Iter;
    for (Iter it = m_strands.begin(); it != m_strands.end(); ++it)
    {
        ElasticRod* rod = *it;
        for (unsigned i = 1; i < rod->m_ppos.size(); ++i)
        {
            m_grid->insertDensity(rod->m_ppos[i], 1);
        }
    }

    for (Iter it = m_strands.begin(); it != m_strands.end(); ++it)
    {
        ElasticRod* rod = *it;
        for (unsigned i = 1; i < rod->m_ppos.size(); ++i)
        {
            m_grid->insertVelocity(rod->m_ppos[i], rod->m_pvel[i]);
        }
    }

}

void Rod::update(mg::Real dt)
{
    assert(m_object != NULL);
    assert(m_grid != NULL);

    if (m_params->m_resolveSelfInterations)
    {
        resetGrid();
    }

    const Mesh* mesh = m_object->getMesh();

#ifdef MULTI_THREAD
#pragma omp parallel for
#endif
    for (unsigned i = 0; i < m_vindices.size(); ++i)
    {
        m_strands[i]->m_ppos[0] = mg::transform_point(m_object->getTransform(), mesh->m_vertices[m_vindices[i]]);
        updateRod(*m_strands[i], dt);
    }
}

void Rod::printself()
{
    typedef std::vector<ElasticRod*>::iterator Iter;
    for (Iter it = m_strands.begin(); it != m_strands.end(); ++it)
    {
        ElasticRod* rod = *it;
        rod->printself();
    }

}

/* semi-implicit Euler with Verlet scheme for velocity update */
void Rod::updateRod(ElasticRod& rod, mg::Real dt) const
{
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
    for (unsigned i = 1; i < rod.m_ppos.size(); ++i)
    {
        prevPos[i] = rod.m_ppos[i];
        rod.m_pvel[i] += dt * forces[i] / rod.m_pmass[i];
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
    for (unsigned i = 1; i < rod.m_ppos.size(); ++i)
    {
        rod.m_pvel[i] = (rod.m_ppos[i] - prevPos[i]) / dt;
    }

    rod.updateCurrentState();
}

void Rod::enforceConstraints(ElasticRod& rod) const
{
    for (unsigned k = 0; k < m_params->m_pbdIter; ++k)
    {
        rod.applyInternalConstraintsIteration();
    }
}

void Rod::enforceConstraintsWithCollision(ElasticRod& rod) const
{
    for (unsigned k = 0; k < m_params->m_pbdIter; ++k)
    {
        applyCollisionConstraintsIteration(rod);
        rod.applyInternalConstraintsIteration();
    }
}

void Rod::applyCollisionConstraintsIteration(ElasticRod& rod) const
{
    mg::Vec3D collision_p, normal;
    for (unsigned i = 1; i < rod.m_ppos.size(); ++i)
    {
        if (m_object->isInsideObject(rod.m_ppos[i], collision_p, normal))
        {
            rod.m_ppos[i] = collision_p;
        }
    }
}

void Rod::accumulateExternalForcesWithSelfInterations(ElasticRod& rod, std::vector<mg::Vec3D>& o_forces) const
{
    mg::Vec3D p1(0, 0, 0), p2(0, 0, 0), pressureGradient(0, 0, 0);
    mg::Real dr = m_params->m_selfInterationDist;
    mg::Real density_p1, density_p2;

    for (unsigned i = 1; i < o_forces.size(); ++i)
    {
        //        gravity
        o_forces[i] += m_params->m_gravity * rod.m_pmass[i];
        //        drag
        o_forces[i] -= m_params->m_drag * rod.m_pvel[i].length() * rod.m_pvel[i];


        //        self interactions:

        //        self stiction acts as averaging of the velocity for near by particles
        //        the velocity however is not direcly averaged by trilinear interpolation is used to calculate the avg. value for particle's current position
        m_grid->getInterpolatedDensity(rod.m_ppos[i], density_p1);
        if (density_p1 > mg::ERR)
        {
            m_grid->getInterpolatedVelocity(rod.m_ppos[i], p1);
            rod.m_pvel[i] = (1 - m_params->m_selfStiction) * rod.m_pvel[i] + m_params->m_selfStiction * p1 / density_p1;
        }

        //        self repulsion
        p1.set(rod.m_ppos[i][0] - dr, rod.m_ppos[i][1], rod.m_ppos[i][2]);
        p2.set(rod.m_ppos[i][0] + dr, rod.m_ppos[i][1], rod.m_ppos[i][2]);
        m_grid->getInterpolatedDensity(p1, density_p1);
        m_grid->getInterpolatedDensity(p2, density_p2);
        pressureGradient[0] = (density_p1 - density_p2) * 0.5 / dr;

        p1.set(rod.m_ppos[i][0], rod.m_ppos[i][1] - dr, rod.m_ppos[i][2]);
        p2.set(rod.m_ppos[i][0], rod.m_ppos[i][1] + dr, rod.m_ppos[i][2]);
        m_grid->getInterpolatedDensity(p1, density_p1);
        m_grid->getInterpolatedDensity(p2, density_p2);
        pressureGradient[1] = (density_p1 - density_p2) * 0.5 / dr;

        p1.set(rod.m_ppos[i][0], rod.m_ppos[i][1], rod.m_ppos[i][2] - dr);
        p2.set(rod.m_ppos[i][0], rod.m_ppos[i][1], rod.m_ppos[i][2] + dr);
        m_grid->getInterpolatedDensity(p1, density_p1);
        m_grid->getInterpolatedDensity(p2, density_p2);
        pressureGradient[2] = (density_p1 - density_p2) * 0.5 / dr;

        o_forces[i] += m_params->m_selfRepulsion * pressureGradient;

    }
}

void Rod::accumulateExternalForces(const ElasticRod& rod, std::vector<mg::Vec3D>& o_forces) const
{
    for (unsigned i = 1; i < o_forces.size(); ++i)
    {
        //        gravity
        o_forces[i] += m_params->m_gravity * rod.m_pmass[i];
        //        drag
        o_forces[i] -= m_params->m_drag * rod.m_pvel[i].length() * rod.m_pvel[i];
    }
}

