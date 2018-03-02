#ifndef VECTOR3_H
#define VECTOR3_H

#include "AlgorithmsConfig.h"
#include <vector>

class Vector3
{
public:
    Vector3(double x, double y, double z)
    {
        mv.resize(4);
        mv[0] = x;
        mv[1] = y;
        mv[2] = z;
        mv[3] = 1.0;
    }
    Vector3(double x, double y, double z,double w)
    {
        mv.resize(4);
        mv[0] = x;
        mv[1] = y;
        mv[2] = z;
        mv[3] = 1.0;
    }
    Vector3()
    {
        mv.resize(4);
        mv[0] = 0.0;
        mv[1] = 0.0;
        mv[2] = 0.0;
        mv[3] = 1.0;
    }

    ~Vector3() 
    {

    }
    double x() const { return mv[0]; }
    double y() const { return mv[1]; }
    double z() const { return mv[2]; }
    void setX(double x) { mv[0] = x; }
    void setY(double y) { mv[1] = y; }
    void setZ(double z) { mv[2] = z; }
    bool isNull()
    {
        return (abs(mv[0] - 0.0) < 1e-15) && (abs(mv[1] - 0.0) < 1e-15) && (abs(mv[2] - 0.0) < 1e-15);
    }
    void initialize()
    {
        mv[0] = 0.0;
        mv[1] = 0.0;
        mv[2] = 0.0;
        mv[3] = 1.0;
    }


    double lengthSquared() const { return mv[0]*mv[0] + mv[1]*mv[1] + mv[2]*mv[2]; }
    double length() const { return sqrt(mv[0] * mv[0] + mv[1] * mv[1] + mv[2] * mv[2]); }


    double& operator[](int index)
    {
        return mv[index];
    }

    const double& operator[](int index) const
    {
        return mv[index];
    }

    Vector3& operator=(const Vector3& v)
    {
        if (this == &v)
            return *this; // may be (*this) == rhs if needs.

        mv = v.mv;
        return *this;
    }

    friend inline bool operator==(const Vector3 &v1, const Vector3 &v2);
    friend inline bool operator!=(const Vector3 &v1, const Vector3 &v2);
    friend inline const Vector3 operator+(const Vector3 &v1, const Vector3 &v2);
    friend inline const Vector3 operator-(const Vector3 &v1, const Vector3 &v2);
    friend inline const Vector3 operator*(double factor, const Vector3 &vector);
    friend inline const Vector3 operator*(const Vector3 &vector, double factor);
    friend const Vector3 operator*(const Vector3 &v1, const Vector3& v2);
    friend inline const Vector3 operator-(const Vector3 &vector);
    friend inline const Vector3 operator/(const Vector3 &vector, double divisor);

    static double dotProduct(const Vector3& v1, const Vector3& v2)
    {
        return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
    }
    static Vector3 crossProduct(const Vector3& v1, const Vector3& v2)
    {
        return Vector3(v1.y() * v2.z() - v1.z() * v2.y(),
            v1.z() * v2.x() - v1.x() * v2.z(),
            v1.x() * v2.y() - v1.y() * v2.x());
    }
    Vector3 normalized() const
    {
        // Need some extra precision if the length is very small.
        double len = double(mv[0]) * double(mv[0]) +
            double(mv[1]) * double(mv[1]) +
            double(mv[2]) * double(mv[2]);
        if (FUZZY_IS_NULL(len - 1.0f))
            return *this;
        else if (!FUZZY_IS_NULL(len))
            return *this / sqrt(len);
        else
            return Vector3();
    }
    void normalize()
    {
        // Need some extra precision if the length is very small.
        double len = double(mv[0]) * double(mv[0]) +
            double(mv[1]) * double(mv[1]) +
            double(mv[2]) * double(mv[2]);
        if (FUZZY_IS_NULL(len - 1.0f) || FUZZY_IS_NULL(len))
            return;
        len = sqrt(len);

        mv[0] /= len;
        mv[1] /= len;
        mv[2] /= len;
    }
private:
    std::vector<double> mv;
};

inline bool operator==(const Vector3 &v1, const Vector3 &v2)
{
    return v1.x() == v2.x() && v1.y() == v2.y() && v1.z() == v2.z();
}

inline bool operator!=(const Vector3 &v1, const Vector3 &v2)
{
    return v1.x() != v2.x() || v1.y() != v2.y() || v1.z() != v2.z();
}

inline const Vector3 operator+(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z(), 1);
}

inline const Vector3 operator-(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z(), 1);
}

inline const Vector3 operator*(double factor, const Vector3 &vector)
{
    return Vector3(vector.x() * factor, vector.y() * factor, vector.z() * factor, 1);
}

inline const Vector3 operator*(const Vector3 &vector, double factor)
{
    return Vector3(vector.x() * factor, vector.y() * factor, vector.z() * factor, 1);
}

inline const Vector3 operator*(const Vector3 &v1, const Vector3& v2)
{
    return Vector3(v1.x() * v2.x(), v1.y() * v2.y(), v1.z() * v2.z(), 1);
}

inline const Vector3 operator-(const Vector3 &vector)
{
    return Vector3(-vector.x(), -vector.y(), -vector.z(), 1);
}

inline const Vector3 operator/(const Vector3 &vector, double divisor)
{
    return Vector3(vector.x() / divisor, vector.y() / divisor, vector.z() / divisor, 1);
}


#endif // UTIL_H