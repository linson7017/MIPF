#ifndef MathUtil_h__
#define MathUtil_h__

#pragma once

#include "qf_config.h"
#include "Vector3.h"
#include "IQF_MathUtil.h"

class MathUtil : public IQF_MathUtil
{
public:
    void FitLine(const std::vector<Vector3>& pointsList, Vector3& center, Vector3& normal);

    double CalculateIntersectionPoint(const Vector3& center1, const Vector3& normal1,
        const Vector3& center2, const Vector3& normal2,
        Vector3& intersect);
};




#endif // MathUtil_h__
