#include <vtkMath.h>
#include <math.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkSmartPointer.h>
#ifndef mathutil_h
#define mathutil_h
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) ((a)>(b)?(a):(b))
void vfp(double [3], const double [3], const double [3]); 
void ve(double [3], const double [3]);
void vn(double [3]);
void gf(double [3], double [3], double [3], const double [3], const double [3]);
void fc(double [3],double [3],double [3], const double [3], const double [3], const double [3]);
void ae(vtkSmartPointer<vtkPoints> , 	const vtkSmartPointer<vtkPoints> , 	const vtkSmartPointer<vtkPoints> , 	const vtkSmartPointer<vtkDoubleArray>  ,	const double );
void cnea( const vtkSmartPointer<vtkPoints> , vtkSmartPointer<vtkPoints> , vtkSmartPointer<vtkDoubleArray> );
void vt(double [3], const double [3], const double );
void va(double [3], const double [3]);
void vt(double [3], const double s);
void nfn(double [3][3], const double [3], double );
void aer(
	const double [3],
	const double [3],
	const double [3],
	double [3], 
	double [3], 
	double [3], 
	const double [3],
	double );
double cne(
	double [3], 
	const double [3], 
	const double [3], 
	const double [3],
	const double [3]);
void id2sub(int, int[3], int[3]);
int sub2id(int, int, int, int[3]);
double sqrtSquare(double[3], double[3]);
double norm(int[3]);
double norm(double[3]);
double normDist(int[3], int[3]);
double normDist(double[3], double[3]);
void getPerpDirection(double [3], double [3], double [3], double [3]);
void cross(double [3], double [3], double [3]);
double angle(double [3], double [3], double[3]);
double projectDist(double [3], double [3], double [3]);
void extropolate(double[3], double[3], double, double[3]);
void eig3volume(float, float, float, float, float, float, float, double[3]);
void tred2(double V[3][3], double d[3], double e[3]);
double hypot2(double x, double y);
void tql2(double V[3][3], double d[3], double e[3]);
double absd(double val);
void eigen_decomposition(double A[3][3], double V[3][3], double d[3]);
void threePoints2volPoints(int origDims[3], int firstSub[3], int lastSub[3], int seedSub[3], int newOrigSub[3], int finalSub[3]);
void onePoint2volPoints(int origDims[3], int seedpointSub[3], int origPointSub[3], int finalPointSub[3]);
void endPointCompute(double pA[3], double pB[3], double pC[3], double pAbbar[3]);
void endPointCompute0(double pA[3], double pB[3], double pC[3], double pCbbar[3]);

#endif
