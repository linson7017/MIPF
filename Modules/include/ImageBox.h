#ifndef ImageBox_h__
#define ImageBox_h__

#include "ImageVector.h"

template<typename intT> class TImageBox
{

public:
    typedef ImageVector<intT> VectorType;

    VectorType v1;

    VectorType v2;

    inline TImageBox() : v1(0), v2(-1)
    {
    }

    //! Constructor: Creates subimage from two passed vectors \p vector1 and
    //! \p vector2. ImageVector components are left as they are;
    //! So the caller is responsible to create a valid/invalid subimage region.
    inline TImageBox(const VectorType& vector1, const VectorType& vector2) : v1(vector1), v2(vector2)
    {   

    }

    TImageBox intersect(const TImageBox &loc1, const TImageBox &loc2)
    {
        TImageBox result;

        result.v1 = VectorType::compMax(loc1.v1,loc2.v1);
        result.v2 = VectorType::compMin(loc1.v2,loc2.v2);
        return result;
    }

    VectorType getExtent() const
    {
        return (v2 - v1) + VectorType(1);
    }
};
#endif // ImageBox_h__