#ifndef Mask_h__
#define Mask_h__

#pragma once
#include <itkImage.h>

class Mask :public itk::Image<unsigned char, 3>
{
public:
    typedef Mask                       Self;
    typedef itk::Image< unsigned char, 3> Superclass;
    typedef itk::SmartPointer< Self >              Pointer;
    typedef itk::SmartPointer< const Self >        ConstPointer;
    typedef itk::WeakPointer< const Self >         ConstWeakPointer;

    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(Mask, Image);

    /** Dimension of the image. */
    itkStaticConstMacro(ImageDimension, unsigned int,
        Superclass::ImageDimension);
    typedef Superclass::IndexType IndexType;

    typedef Superclass::IOPixelType IOPixelType;

private:

    Mask(const Self &);    //purposely not implemented
    void operator=(const Self &); //purposely not implemented

                                  /** Constructor. */
    Mask() {}

};

#endif // Mask_h__
