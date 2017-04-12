#ifndef SphereFit_h__
#define SphereFit_h__

#include <QVector>
#include <QVector3D>
#include <QList>

using namespace std;
#define   ABS(x)   (x)>0?(x):-(x)   
#define   SWAP(a,b)   {temp=(a);(a)=(b);(b)=temp;}   
class SphereFit
{
public:
    SphereFit() {}
    ~SphereFit() {}
    void Clear() { m_Points.clear(); }
    void InsertPoint(const QVector3D& p) { m_Points.push_back(p); }
    void Fit(QVector3D& center, double& radius)
    {
        int   i, n, p = 4;
        double   x0, y0, z0, r;
        double   **x, *y, beta[4];
        n = m_Points.size();
        x = new   double*[n];
        y = new   double[n];
        for (i = 0; i < n; i++)
        {
            x[i] = new double[4];
            x[i][0] = m_Points.at(i).x();
            x[i][1] = m_Points.at(i).y();
            x[i][2] = m_Points.at(i).z();
            x[i][3] = 1;
            y[i] = -x[i][0] * x[i][0] - x[i][1] * x[i][1] - x[i][2] * x[i][2];
        }
        linear(x, y, beta, n, p);
        x0 = -beta[0] / 2;
        y0 = -beta[1] / 2;
        z0 = -beta[2] / 2;
        radius = sqrt(x0*x0 + y0*y0 + z0*z0 - beta[3]);
        center.setX(x0);
        center.setY(y0);
        center.setZ(z0);
    }
    QVector3D GetPoint(int index)
    {
        return m_Points.at(index);
    }
private:
    void linear(double **x, double *y, double *beta, int n, int p)
    {
        double   **a, *b;
        int   i, j, k;
        a = new   double*[p];
        for (i = 0; i < p; i++)
            a[i] = new   double[p];
        for (i = 0; i < p; i++)
            for (j = 0; j < p; j++)
            {
                a[i][j] = 0;
                for (k = 0; k < n; k++)
                    a[i][j] += x[k][i] * x[k][j];
            }
        b = new   double[p];
        for (i = 0; i < p; i++)
        {
            b[i] = 0;
            for (j = 0; j < n; j++)
                b[i] += x[j][i] * y[j];
        }
        solve(a, b, beta, p);
        for (i = 0; i < p; i++)
            delete   a[i];
        delete   a, b;
    }
    void solve(double **a, double *b, double *x, int n)
    {
        int   i, j, k, ik;
        double   mik, temp;
        for (k = 0; k < n; k++)
        {
            mik = -1;
            for (i = k; i < n; i++)
                if (ABS(a[i][k]) > mik)
                {
                    mik = ABS(a[i][k]);
                    ik = i;
                }
            for (j = k; j < n; j++)
                SWAP(a[ik][j], a[k][j]);
            SWAP(b[k], b[ik]);
            b[k] /= a[k][k];
            for (i = n - 1; i >= k; i--)
                a[k][i] /= a[k][k];
            for (i = k + 1; i < n; i++)
            {
                b[i] -= a[i][k] * b[k];
                for (j = n - 1; j >= k; j--)
                    a[i][j] -= a[i][k] * a[k][j];
            }
        }
        for (i = n - 1; i >= 0; i--)
        {
            x[i] = b[i];
            for (j = i + 1; j < n; j++)
                x[i] -= a[i][j] * x[j];
        }
    }
    QList<QVector3D> m_Points;
};
#endif // SphereFit_h__