#ifndef IQF_MathUtil_h__
#define IQF_MathUtil_h__

#include "IQF_Object.h"
#include <stdlib.h>
#include <Vector3.h>

const char QF_Algorithm_MathUtil[] = "QF_Algorithm_MathUtil";

class IQF_MathUtil : public IQF_Object
{
public:
    virtual void FitLine(const std::vector<Vector3>& pointsList, Vector3& center, Vector3& normal)=0;
    virtual double CalculateIntersectionPoint(const Vector3& center1, const Vector3& normal1,
        const Vector3& center2, const Vector3& normal2,
        Vector3& intersect)=0;
};

#endif // IQF_MathUtil_h__
