#ifndef XMARKER_H
#define XMARKER_H

#include "Vector3.h"

class XMarker
{
public:
    XMarker ()
    {
        pos[0]=0;
        pos[1]=0;
        pos[2]=0;
        pos[3]=0;
        pos[4]=0;
        pos[5]=0;
        vec[0]=0;
        vec[1]=0;
        vec[2]=0;
    }
    XMarker(double x, double y, double z)
    {
        pos[0] = x;
        pos[1] = y;
        pos[2] = z;
        pos[3] = 0;
        pos[4] = 0;
        pos[5] = 0;
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = 0;
    }
    XMarker(double x,double y,double z,double c,double t,double u,double vx,double vy,double vz)
    {
        pos[0]=x;
        pos[1]=y;
        pos[2]=z;
        pos[3]=c;
        pos[4]=t;
        pos[5]=u;
        vec[0]=vx;
        vec[1]=vy;
        vec[2]=vz;
    }
    XMarker(const XMarker& m)
    {
        vec = m.vec;
        pos[0] = m.x();
        pos[1] = m.y();
        pos[2] = m.z();
        pos[3] = m.c();
        pos[4] = m.t();
        pos[5] = m.u();
    }
    double &x () { return pos[0]; }
    double &y () { return pos[1]; }
    double &z () { return pos[2]; }
    double &c () { return pos[3]; }
    double &t () { return pos[4]; }
    double &u () { return pos[5]; }

    const double &x () const { return pos[0]; }
    const double &y () const { return pos[1]; }
    const double &z () const { return pos[2]; }
    const double &c () const { return pos[3]; }
    const double &t () const { return pos[4]; }
    const double &u () const { return pos[5]; }

    double vx () const { return vec[0]; }
    double vy () const { return vec[1]; }
    double vz () const { return vec[2]; }


    XMarker& operator=(const XMarker& v)
    {
        if (this == &v)
            return *this; // may be (*this) == rhs if needs.

        vec = v.vec;
        pos[0] = v.x();
        pos[1] = v.y();
        pos[2] = v.z();
        pos[3] = v.c();
        pos[4] = v.t();
        pos[5] = v.u();
        return *this;
    }
    const Vector3& vector3() const { return vec; }
private:
    double pos[6];
    Vector3 vec;
};


class XMarkerList
{
public:
    XMarkerList(){}
    void appendItem(XMarker marker)
    {
        _markerList.push_back(marker);
    }
    void clearList()
    {
        _markerList.clear();
    }
    size_t size(){return _markerList.size();}
    XMarker& getItemAt(int index){return _markerList.at(index);}

private:
    std::vector<XMarker> _markerList;
};

#endif // UTIL_H