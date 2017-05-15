#include "MathUtil.h"

#include "newmatap.h"

void MathUtil::FitLine(const std::vector<Vector3>& pointsList, Vector3& center, Vector3& normal)
{
    if (pointsList.size() < 3)
    {
        return;
    }
    //calculate average point as the center point
    double sumX, sumY, sumZ = 0;
    int pointSize = pointsList.size();
    for (size_t i = 0; i < pointSize; i++)
    {
        sumX += pointsList[i].x();
        sumY += pointsList[i].y();
        sumZ += pointsList[i].z();
    }
    center.setX(sumX / pointSize);
    center.setY(sumY / pointSize);
    center.setZ(sumZ / pointSize);

    //svd
    Matrix m(pointSize, 3);
    for (int i = 0; i < pointSize; i++)
    {
        m(i + 1, 1) = pointsList[i].x() - center.x();
        m(i + 1, 2) = pointsList[i].y() - center.y();
        m(i + 1, 3) = pointsList[i].z() - center.z();
    }
    Matrix uu, vv;
    DiagonalMatrix dd;
    SVD(m, dd, uu, vv);

    //get the line direction
    normal.setX(vv(1, 1));
    normal.setY(vv(2, 1));
    normal.setZ(vv(3, 1));
}

double MathUtil::CalculateIntersectionPoint(const Vector3& center1, const Vector3& normal1,
    const Vector3& center2, const Vector3& normal2,
    Vector3& intersect)
{
    Vector3   u(normal1.x(), normal1.y(), normal1.z());
    Vector3   v(normal2.x(), normal2.y(), normal2.z());
    Vector3   w = center1 - center2;
    u.normalize();
    v.normalize();
    //w.normalize();
    float    a = Vector3::dotProduct(u, u);
    float    b = Vector3::dotProduct(u, v);
    float    c = Vector3::dotProduct(v, v);
    float    d = Vector3::dotProduct(u, w);
    float    e = Vector3::dotProduct(v, w);
    float    D = a*c - b*b;        // always >= 0
    float    sc, tc;

    // compute the line parameters of the two closest points
    if (D < 1e-5) {          // the lines are almost parallel
        sc = 0.0;
        tc = (b > c ? d / b : e / c);    // use the largest denominator
    }
    else {
        sc = (b*e - c*d) / D;
        tc = (a*e - b*d) / D;
    }

    Vector3 p1 = Vector3(center1.x(), center1.y(), center1.z()) + sc*u;
    Vector3 p2 = Vector3(center2.x(), center2.y(), center2.z()) + tc*v;
    intersect.setX((p1.x() + p2.x()) / 2);
    intersect.setY((p1.y() + p2.y()) / 2);
    intersect.setZ((p1.z() + p2.z()) / 2);

    // get the difference of the two closest points
    Vector3   dP = w + (sc * u) - (tc * v);

    return dP.length();   // return the closest distance
}
