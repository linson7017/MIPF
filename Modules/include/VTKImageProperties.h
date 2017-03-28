#ifndef MEDICALIMAGEPROPERTIES_H
#define MEDICALIMAGEPROPERTIES_H

#include "Vector3.h"
#include "vtkImageData.h"

class VTKImageProperties
{
public:
    VTKImageProperties(){}
    ~VTKImageProperties(){}
    Vector3 mapWorldToVoxel(const Vector3& v) const
    {
        double spacing[3];
        double origin[3];
        _img->GetSpacing(spacing);
        _img->GetOrigin(origin);
        int i = round((v.x() - origin[0]) / spacing[0]);
        int j = round((v.y() - origin[1]) / spacing[1]);
        int k = round((v.z() - origin[2]) / spacing[2]);
        return Vector3(i,j,k);
    }
    Vector3 mapVoxelToWorld(const Vector3& v) const
    {
        double spacing[3];
        double origin[3];
        _img->GetSpacing(spacing);
        _img->GetOrigin(origin);
        double x = origin[0] + v.x() * spacing[0];
        double y = origin[1] + v.y() * spacing[1];
        double z = origin[2] + v.z() * spacing[2];
        return Vector3(x,y,z);
    }
    bool containsVoxelIndex(const Vector3& v) const
    {   
        int extent[6];
        _img->GetExtent(extent);
        return v.x() > extent[0] && v.x() < extent[1] && v.y() > extent[2] && v.y() < extent[3] && v.z() > extent[4] && v.z() < extent[5];
    }
    bool containsWorldIndex(const Vector3& v) const
    {
        return containsVoxelIndex(mapWorldToVoxel(v));
    }
    Vector3 getVoxelSize() const
    {
        double spacing[3];
        _img->GetSpacing(spacing);
        return Vector3(spacing[0], spacing[1], spacing[2]);
    }
    void setImageProperties(vtkImageData* img){_img = img;}
    vtkImageData* getImage() const
    { return _img; }
private:
    vtkImageData* _img;
};

#endif