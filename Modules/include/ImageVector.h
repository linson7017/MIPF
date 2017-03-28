#ifndef IMAGE_VECTOR_H
#define IMAGE_VECTOR_H

//////////////////////////ImageVector///////////////////////////
enum { MaxImageDimension = 6 };
template<class Type>
class ImageVector
{
public:
    ImageVector(){}
    ~ImageVector(){}
    typedef Type ComponentType;

    //! Enumerator defining the dimension of this class.
    enum { NumberOfDimensions = MaxImageDimension };
    union {
      struct {
        //! X component of the vector.
        ComponentType x;
        //! Y component of the vector.
        ComponentType y;
        //! Z component of the vector.
        ComponentType z;
        //! Color component of the vector.
        ComponentType c;
        //! Time component of the vector.
        ComponentType t;
        //! Unit/Modality/User component of the vector.
        ComponentType u;
      };
      ComponentType array[NumberOfDimensions];
    };
    ImageVector(const ComponentType cx, const ComponentType cy, const ComponentType cz,
                            const ComponentType cp, const ComponentType tp, const ComponentType up)
    {
      x = cx;
      y = cy;
      z = cz;
      c = cp;
      t = tp;
      u = up;
    }
    ImageVector(const ComponentType value)
    {
        x = value;
        y = value;
        z = value;
        c = value;
        t = value;
        u = value;
    }
    ImageVector(const ImageVector& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        c = v.c;
        t = v.t;
        u = v.u;
    }

    Type& operator[](int index)
    {
        return array[index];
    }

    const Type& operator[](int index) const
    {
        return array[index];
    }
	//friend inline const ImageVector<Type> operator+(const ImageVector<Type> &v1, const ImageVector<Type> &v2);
   // friend inline const ImageVector<Type> operator-(const ImageVector<Type> &v1, const ImageVector<Type> &v2);
	ImageVector &operator+=(const ImageVector &v);
    ImageVector &operator-=(const ImageVector &v);

    static ImageVector compMax(ImageVector& v1, ImageVector&v2)
    {
        return ImageVector(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z), max(v1.c, v2.c), max(v1.t, v2.t), max(v1.u, v2.u));
    }

    static ImageVector compMin(ImageVector& v1, ImageVector&v2)
    {
        return ImageVector(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z), min(v1.c, v2.c), min(v1.t, v2.t), min(v1.u, v2.u));
    }
	
};

//template<class Type>
//inline const ImageVector<Type> operator+(const ImageVector<Type> &v1, const ImageVector<Type> &v2)
//{
//    return ImageVector<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.c + v2.c, v1.t + v2.t, v1.u + v2.u );
//}
//
//template<class Type>
//inline const ImageVector<Type> operator-(const ImageVector<Type> &v1, const ImageVector<Type> &v2)
//{
//    return ImageVector<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.c - v2.c, v1.t - v2.t, v1.u - v2.u );
//}

template<class Type>
inline ImageVector<Type> &ImageVector<Type>::operator+=(const ImageVector<Type> &v)
{
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    this->c += v.c;
    this->t += v.t;
    this->u += v.u;
    return *this;
}

template<class Type>
inline ImageVector<Type> &ImageVector<Type>::operator-=(const ImageVector<Type> &v)
{
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    this->c -= v.c;
    this->t -= v.t;
    this->u -= v.u;
    return *this;
}





#endif 