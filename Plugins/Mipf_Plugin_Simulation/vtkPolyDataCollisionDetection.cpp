#include "vtkPolyDataCollisionDetection.h"
#include "PBD/PositionBasedElasticRodsModel.h"

using namespace PBD;

vtkPolyDataCollisionDetection::vtkPolyDataCollisionDetection() :
    DistanceFieldCollisionDetection()
{
}


vtkPolyDataCollisionDetection::~vtkPolyDataCollisionDetection()
{
}

void vtkPolyDataCollisionDetection::collisionDetection(SimulationModel &model)
{
    DistanceFieldCollisionDetection::collisionDetection(model);
    PositionBasedElasticRodsModel* pberModel = dynamic_cast<PositionBasedElasticRodsModel*>(&model);
    if (!pberModel)
    {
        return;
    }
    std::vector<vtkSmartPointer<vtkPolyData>> vtkPolyDataBodies = pberModel->getvtkPolyDataModels();
    const ParticleData &pd = model.getParticles();

}
