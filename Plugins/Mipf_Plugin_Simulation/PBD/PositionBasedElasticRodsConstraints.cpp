#include "PositionBasedElasticRodsConstraints.h"
#include "Demos/Simulation/SimulationModel.h"
#include "PositionBasedDynamics/PositionBasedDynamics.h"
#include "PositionBasedDynamics/PositionBasedElasticRods.h"
#include "Demos/Simulation/IDFactory.h"
#include "Demos/Simulation/TimeManager.h"

#include <vtkDataArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntersectionPolyDataFilter.h>
#include <vtkLineSource.h>
#include <VTKImageProperties.h>
#include <vtkGenericCell.h>

#include <vtkPlane.h>
#include <vtkCell.h>
#include <vtkMath.h>


using namespace PBD;

int PerpendiculaBisectorConstraint::TYPE_ID = IDFactory::getId();
int GhostPointEdgeDistanceConstraint::TYPE_ID = IDFactory::getId();
int DarbouxVectorConstraint::TYPE_ID = IDFactory::getId();
int ParticalPolyDataConstraint::TYPE_ID = IDFactory::getId();

//////////////////////////////////////////////////////////////////////////
// PerpendiculaBisectorConstraint
//////////////////////////////////////////////////////////////////////////
bool PerpendiculaBisectorConstraint::initConstraint(SimulationModel &model,
	const unsigned int particle1, const unsigned int particle2, const unsigned int particle3)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;

	return true;
}

bool PerpendiculaBisectorConstraint::solvePositionConstraint(SimulationModel &model)
{
	PositionBasedElasticRodsModel &simModel = static_cast<PositionBasedElasticRodsModel&>(model);

	ParticleData &pd = model.getParticles();
	ParticleData &pg = simModel.getGhostParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];
	const unsigned i3 = m_bodies[2];

	Vector3r &x1 = pd.getPosition(i1);
	Vector3r &x2 = pd.getPosition(i2);
	Vector3r &x3 = pg.getPosition(i3);

	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);
	const Real invMass3 = pg.getInvMass(i3);

	Vector3r corr[3];
	const bool res = PositionBasedElasticRods::solve_PerpendiculaBisectorConstraint(
		x1, invMass1, 
		x2, invMass2, 
		x3, invMass3, 
		1.0, 
		corr[0], corr[1], corr[2]);

	if (res)
	{
		if (invMass1 != 0.0f)
			x1 += corr[0];

		if (invMass2 != 0.0f)
			x2 += corr[1];

		if (invMass2 != 0.0f)
			x3 += corr[2];
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
// GhostPointEdgeDistanceConstraint
//////////////////////////////////////////////////////////////////////////
bool GhostPointEdgeDistanceConstraint::initConstraint(PositionBasedElasticRodsModel &model, const unsigned int particle1, const unsigned int particle2, const unsigned int particle3)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;

	ParticleData &pd = model.getParticles();
	ParticleData &pg = model.getGhostParticles();

	Vector3r &x1 = pd.getPosition( particle1);
	Vector3r &x2 = pd.getPosition( particle2);
	Vector3r &x3 = pg.getPosition( particle3);

	Vector3r xm = 0.5 * (x1 + x2);

	m_restLength = (x3 - xm).norm();

	return true;
}

bool GhostPointEdgeDistanceConstraint::solvePositionConstraint(SimulationModel &model)
{
	PositionBasedElasticRodsModel &simModel = static_cast<PositionBasedElasticRodsModel&>(model);

	ParticleData &pd = model.getParticles();
	ParticleData &pg = simModel.getGhostParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];
	const unsigned i3 = m_bodies[2];

	Vector3r &x1 = pd.getPosition(i1);
	Vector3r &x2 = pd.getPosition(i2);
	Vector3r &x3 = pg.getPosition(i3);

	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);
	const Real invMass3 = pg.getInvMass(i3);
	
	Vector3r corr[3];
	const bool res = PositionBasedElasticRods::solve_GhostPointEdgeDistanceConstraint(x1, invMass1, x2, invMass2, x3, invMass3, 1.0, m_restLength, corr[0], corr[1], corr[2]);

	if (res)
	{
		if (invMass1 != 0.0f)
			x1 += corr[0];
		
		if (invMass2 != 0.0f)
			x2 += corr[1];

		if (invMass3 != 0.0f)
			x3 += corr[2];
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
// DarbouxVectorConstraint
//////////////////////////////////////////////////////////////////////////
bool DarbouxVectorConstraint::initConstraint(PositionBasedElasticRodsModel &model, const unsigned int particle1, const unsigned int particle2,
	const unsigned int particle3, const unsigned int particle4, const unsigned int particle5)
{	
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;
	m_bodies[3] = particle4; //ghost point id
	m_bodies[4] = particle5; //ghost point id

	ParticleData &pd = model.getParticles();
	ParticleData &pg = model.getGhostParticles();

	const Vector3r &x1 = pd.getPosition0(m_bodies[0]);
	const Vector3r &x2 = pd.getPosition0(m_bodies[1]);
	const Vector3r &x3 = pd.getPosition0(m_bodies[2]);
	const Vector3r &x4 = pg.getPosition0(m_bodies[3]);
	const Vector3r &x5 = pg.getPosition0(m_bodies[4]);

	PositionBasedElasticRods::computeMaterialFrame(x1, x2, x4, m_dA);
	PositionBasedElasticRods::computeMaterialFrame(x2, x3, x5, m_dB);
	Vector3r restDarbouxVector;
	PositionBasedElasticRods::computeDarbouxVector(m_dA, m_dB, 1.0, restDarbouxVector);
	model.setRestDarbouxVector(restDarbouxVector);

	return true;
}

bool DarbouxVectorConstraint::solvePositionConstraint(SimulationModel &model)
{
	PositionBasedElasticRodsModel &simModel = static_cast<PositionBasedElasticRodsModel&>(model);

	ParticleData &pd = model.getParticles();
	ParticleData &pg = simModel.getGhostParticles();

	Vector3r &x1 = pd.getPosition(m_bodies[0]);
	Vector3r &x2 = pd.getPosition(m_bodies[1]);
	Vector3r &x3 = pd.getPosition(m_bodies[2]);
	Vector3r &x4 = pg.getPosition(m_bodies[3]);
	Vector3r &x5 = pg.getPosition(m_bodies[4]);

	const Real invMass1 = pd.getInvMass(m_bodies[0]);
	const Real invMass2 = pd.getInvMass(m_bodies[1]);
	const Real invMass3 = pd.getInvMass(m_bodies[2]);
	const Real invMass4 = pg.getInvMass(m_bodies[3]);
	const Real invMass5 = pg.getInvMass(m_bodies[4]);

	Vector3r corr[5];

	bool res = PositionBasedElasticRods::solve_DarbouxVectorConstraint(
		x1, invMass1, x2, invMass2, x3, invMass3, x4, invMass4, x5, invMass5,
		simModel.getBendingAndTwistingStiffness(), 1.0, simModel.getRestDarbouxVector(), 
		corr[0], corr[1], corr[2], corr[3], corr[4]);

	if (res)
	{
		if (invMass1 != 0.0f)
			x1 += corr[0];
		
		if (invMass2 != 0.0f)
			x2 += corr[1];
		
		if (invMass3 != 0.0f)
			x3 += corr[2];
		
		if (invMass4 != 0.0f)
			x4 += corr[3];

		if (invMass5 != 0.0f)
			x5 += corr[4];
	}
	return res;
}

bool isInside(const Vector3r& p, vtkImageData* img)
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

bool ParticalPolyDataConstraint::isCollided(vtkCellLocator* locator, vtkPolyData* polydata, Vector3r& current, Vector3r& pre, Vector3r& o_collisionPoint, Vector3r& o_normal)     const
{

    o_collisionPoint = current;
    //Find the closest points to TestPoint
    Vector3r n = pre - current;
    n.normalize();
    Vector3r p1 = pre + n * 1.0;
    Vector3r p2 = current - n * 1.0;

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
        o_collisionPoint = Vector3r(pline[0], pline[1], pline[2]);
        //calculate normals
        vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
        if (normalDataFloat)
        {
            double* normal = normalDataFloat->GetTuple(cellId);
            o_normal = Vector3r(normal[0], normal[1], normal[2]);
            o_normal.normalize();
        }
        return true;
    }
    else
    {
        return false;
    }
}


bool ParticalPolyDataConstraint::initConstraint(PositionBasedElasticRodsModel &model)
{
    
    std::vector<vtkSmartPointer<vtkPolyData>> objs = model.getvtkPolyDataModels();

    for (int i=0;i<objs.size();i++)
    {
        vtkSmartPointer<vtkCellLocator> locator = vtkSmartPointer<vtkCellLocator>::New();
        locator->SetDataSet(objs[i]);
        locator->BuildLocator();
        m_locators.push_back(locator);
    }  
    return true;
}

bool ParticalPolyDataConstraint::solvePositionConstraint(SimulationModel &model)
{
    PositionBasedElasticRodsModel &simModel = static_cast<PositionBasedElasticRodsModel&>(model);
    ParticleData &pd = simModel.getParticles();
    ParticleData &gpd = simModel.getGhostParticles();
    Vector3r collision_p, normal;
    for (unsigned i = 0; i < pd.getNumberOfParticles(); ++i)
    {
        for (unsigned j = 0; j < simModel.getvtkPolyDataModels().size(); j++)
        {
            if (!isInside(pd.getPosition(i), simModel.m_imgs[j].Get()))
            {
                if (isInside(pd.getOldPosition(i), simModel.m_imgs[j].Get()))
                {
                    if (isCollided(m_locators[j].Get(), simModel.getvtkPolyDataModels()[j], pd.getPosition(i), pd.getOldPosition(i), collision_p, normal))
                    {
                        //pd.setPosition(i,pd.getLastPosition(i) - pd.getVelocity(i).dot(normal)*normal);
                        pd.setPosition(i, pd.getOldPosition(i) - TimeManager::getCurrent()->getTimeStepSize()*pd.getVelocity(i).dot(normal)*normal);
                        //pd.setVelocity(i,pd.getVelocity(i).dot(normal)*normal);
                        if (i<pd.getNumberOfParticles()-1)
                        {
                            gpd.setPosition(i, gpd.getOldPosition(i) - TimeManager::getCurrent()->getTimeStepSize()*pd.getVelocity(i).dot(normal)*normal);
                            //pd.setVelocity(i, gpd.getVelocity(i).dot(normal)*normal);
                        }
                    }
                }
            }
        }
    }
    return true;
}

