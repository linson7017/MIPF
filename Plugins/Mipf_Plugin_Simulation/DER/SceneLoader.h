#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "Scene.h"

class SceneLoader
{
public:

    SceneLoader();
    ~SceneLoader();

    Scene* loadTestScene();
    GuideWire* createGuideWireScene(const mg::Vec3D& entrancePoint, const mg::Vec3D& headingDirection, const std::vector<vtkSmartPointer<vtkPolyData>>& objects,const std::vector<vtkSmartPointer<vtkImageData>>& imgs);
private:

    struct PImpl;
    PImpl* m_impl;
};

#endif // SCENELOADER_H
