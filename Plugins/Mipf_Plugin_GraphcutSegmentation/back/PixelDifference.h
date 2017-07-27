#ifndef PixelDifference_h__
#define PixelDifference_h__

#include <cmath>

/** Compute the difference between two gray pixels. */
template <typename TPixel>
class
    GrayPixelDifference
{
public:
    float Difference(const TPixel& a, const TPixel& b)
    {
        // Compute the Euclidean distance between N dimensional pixels
        return abs(a - b);
    }
};


/** Compute the difference between two RGB pixels. */
template <typename TPixel>
class
    RGBPixelDifference
{
public:
    float Difference(const TPixel& a, const TPixel& b)
    {
        // Compute the Euclidean distance between N dimensional pixels
        float difference = 0;

        for (unsigned int i = 0; i < 3; i++)
        {
            difference += pow(a[i] - b[i], 2);
        }

        return sqrt(difference);
    }
};

/** Compute the difference between two pixels whose first 3 components are
* RGB and then there are more components afterwards. E.g. RGBD. */
template <typename TPixel>
class NDPixelDifference
{
    //float RGBWeight = 1.0f; // Needs better c++11 support than is provided by VS2010
    float RGBWeight;

public:
    NDPixelDifference(const float rgbWeight) : RGBWeight(rgbWeight) {}

    float Difference(const TPixel& a, const TPixel& b)
    {
        assert(a.GetNumberOfComponents() == b.GetNumberOfComponents());

        unsigned int numberOfComponents = a.GetNumberOfComponents();

        // Compute the Euclidean distance between N dimensional pixels
        float difference = 0;

        if (a.GetNumberOfComponents() > 3)
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                difference += (this->RGBWeight / 3.) * pow(a[i] - b[i], 2);
            }
            for (unsigned int i = 3; i < numberOfComponents; i++)
            {
                difference += (1 - this->RGBWeight) / (numberOfComponents - 3.) * pow(a[i] - b[i], 2);
            }
        }
        else // image is RGB or less (grayscale)
        {
            for (unsigned int i = 0; i < numberOfComponents; i++)
            {
                difference += pow(a[i] - b[i], 2);
            }
        }
        return sqrt(difference);
    }
};

#endif // PixelDifference_h__
