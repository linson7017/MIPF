#ifndef MatrixUtil_h__
#define MatrixUtil_h__
#include <QMatrix4x4>
#include <itkMatrix.h>

class MatrixUtil
{
public:
    static void QMatrixToItkMatrix(const QMatrix4x4& qm, itk::Matrix<double, 4, 4>& im)
    {
        im.SetIdentity();
        for (int i=0;i<4;i++)
        {
            for (int j=0;j<4;j++)
            {
                im[i][j] = qm(i, j);
            }
        }
    }

    static void ItkMatrixToQMatrix(const itk::Matrix<double, 4, 4>& im, QMatrix4x4& qm)
    {
        qm.setToIdentity();
        for (int i = 0;i < 4;i++)
        {
            for (int j = 0;j < 4;j++)
            {
                qm(i, j) = im[i][j] ;
            }
        }
    }
};

#pragma once
#endif // MatrixUtil_h__
