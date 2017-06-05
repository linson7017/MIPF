#ifndef MatrixUtil_h__
#define MatrixUtil_h__
#include <QMatrix4x4>
#include <itkMatrix.h>
#include <vtkMatrix4x4.h>

class MatrixUtil
{
public:
    static void ItkMatrixToQMatrix(const itk::Matrix<double, 4, 4>& im, QMatrix4x4& qm)
    {
        qm.setToIdentity();
        for (int i = 0;i < 4;i++)
        {
            for (int j = 0;j < 4;j++)
            {
                qm(i, j) = im[i][j];
            }
        }
    }

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

    static void VtkMatrixToQMatrix(const vtkMatrix4x4* vm, QMatrix4x4& qm)
    {
        qm.setToIdentity();
        for (int i = 0;i < 4;i++)
        {
            for (int j = 0;j < 4;j++)
            {
                qm(i, j) = vm->GetElement(i,j);
            }
        }
    }

    static void QMatrixToVtkMatrix(const QMatrix4x4& qm, vtkMatrix4x4* vm)
    {
        vm->Identity();
        for (int i = 0;i < 4;i++)
        {
            for (int j = 0;j < 4;j++)
            {
                vm->SetElement(i,j,qm(i, j));
            }
        }
    }


    static void ItkMatrixToVtkMatrix(const itk::Matrix<double, 4, 4>& im, vtkMatrix4x4* vm)
    {
        vm->Identity();
        for (int i = 0;i < 4;i++)
        {
            for (int j = 0;j < 4;j++)
            {
                vm->SetElement(i, j, im[i][j]);
            }
        }
    }

    static void VtkMatrixToItkMatrix(const vtkMatrix4x4* vm, itk::Matrix<double, 4, 4>& im)
    {
        im.SetIdentity();
        for (int i = 0;i < 4;i++)
        {
            for (int j = 0;j < 4;j++)
            {
                im[i][j] = vm->GetElement(i,j);
            }
        }
    }

    static void VnlMatrixToVtkMatrix(const vnl_matrix_fixed<double, 4, 4>& vnlm, vtkMatrix4x4* vm)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                vm->SetElement(i, j, vnlm(i, j));
            }
        }
    }
};

#pragma once
#endif // MatrixUtil_h__
