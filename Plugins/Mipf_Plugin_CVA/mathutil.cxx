#include "mathutil.h"

void vfp(double x[3], const double y[3], const double z[3])
{
	x[0] = y[0] - z[0];
	x[1] = y[1] - z[1];
	x[2] = y[2] - z[2];
}

void ve(double a[3], const double b[3])
{
	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
}

void vn(double v[3])
{
	double nv = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);

	v[0] = v[0] / nv;
	v[1] = v[1] / nv;
	v[2] = v[2] / nv;
}

void vt(double w[3], const double v[3], const double s)
{
	w[0] = v[0] * s;
	w[1] = v[1] * s;
	w[2] = v[2] * s;
}

void va(double v[3], const double dv[3])
{
	v[0] = v[0] + dv[0];
	v[1] = v[1] + dv[1];
	v[2] = v[2] + dv[2];
}

void vt(double v[3], const double s)
{
	v[0] = v[0] * s;
	v[1] = v[1] * s;
	v[2] = v[2] * s;
}

void gf(double x[3], double y[3], double z[3], const double a[3], const double b[3])
{
	ve(x, a);
	vn(x);
	vtkMath::Cross(a, b, z);
	vn(z);
	vtkMath::Cross(z, x, y);
	vn(y);
}

void fc(double a[3],double b[3],double c[3], const double d[3], const double e[3], const double f[3])
{
	double u = 1.0;

	if ( vtkMath::Dot( c, f) < 0)
	{
		u = -1.0;
	}
	vt(b,u);
	vt(c,u);
}

void mem(double fn[3][3], const double fa[3], double fb)
{
	double u = fa[0];
	double v = fa[1];
	double w = fa[2];
	double a = (1-cos(fb));
	double b = sin(fb);
	double c = cos(fb);
	fn[0][0] = u*u+(v*v+w*w)*c; fn[0][1] = u*v*a-w*b; fn[0][2] = u*w*a + v*b; 
	fn[1][0] = u*v*a+w*b; fn[1][1] = v*v+(u*u+w*w)*c; fn[1][2] = v*w*a - u*b; 
	fn[2][0] = u*w*a -v*b; fn[2][1] = v*w*a+u*b; fn[2][2] = w*w+(u*u+v*v)*c; 
}

void aer(
	const double a[3],
	const double b[3],
	const double c[3],
	double d[3], 
	double e[3], 
	double f[3], 
	const double fa[3],
	double fb)
{
	double fn[3][3];
	double aea[3];
	double XT[3][3];

	XT[0][0] = a[0]; XT[0][1] = b[0]; XT[0][2] = c[0]; 
	XT[1][0] = a[1]; XT[1][1] = b[1]; XT[1][2] = c[1];
	XT[2][0] = a[2]; XT[2][1] = b[2]; XT[2][2] = c[2]; 
	vtkMath::Multiply3x3(XT,fa, aea);

	mem(fn, aea, fb);
	vtkMath::Multiply3x3(fn, a, d);
	vtkMath::Multiply3x3(fn, b, e);
	vtkMath::Multiply3x3(fn, c, f);
}

void ae(vtkSmartPointer<vtkPoints> jp,
	const vtkSmartPointer<vtkPoints> sp, 
	const vtkSmartPointer<vtkPoints> ea, 
	const vtkSmartPointer<vtkDoubleArray>  eb,
	const double af)
{
	double ov1[3];
	double ov2[3];
	double ov3[3];

	double op0[3];
	double op1[3];
	double op2[3];

	double a[3];
	double b[3];
	double c[3];

	double d[3];
	double e[3];
	double f[3];

	double nv;
	double op[3];
	double np[3];

	double fa[3];
	double fb;
	sp->GetPoint(0, op0);
	jp->InsertNextPoint(op0);
	sp->GetPoint(1, op1);
	jp->InsertNextPoint(op1);
	sp->GetPoint(2, op2);
	vfp(ov1, op1, op0);
	vn(ov1);
	vfp(ov2, op2, op1);
	nv = vtkMath::Norm(ov2);
	vn(ov2);
	gf(a, b, c, ov1, ov2);
	double cp[3];
	ve(cp, op2);
	double dv[3];
	for (int i = 0; i < sp->GetNumberOfPoints()-3; i++)
	{
		sp->GetPoint(i+1, op1);
		sp->GetPoint(i+2, op2);
		vfp(ov2, op2, op1);
		nv = vtkMath::Norm(ov2);
		ea->GetPoint(i, fa);
		fb = eb->GetValue(i);
		aer(a, b, c, d, e, f, fa, fb * af);
		vt(dv, d, nv);
		va(cp, dv);
		jp->InsertNextPoint(cp);
		ve(a, d);
		ve(b, e);
		ve(c, f);
		vn(a);
		vn(b);
		vn(c);
	}
}

double cne(
	double ea[3], 
	const double a[3], 
	const double b[3], 
	const double c[3],
	const double v[3])
{
	double e[3];
	vtkMath::Cross(a, v, e);
	vn(e);
	double fb = acos(vtkMath::Dot(a, v));

	double XT[3][3];

	XT[0][0] = a[0]; XT[0][1] = a[1]; XT[0][2] = a[2]; 
	XT[1][0] = b[0]; XT[1][1] = b[1]; XT[1][2] = b[2]; 
	XT[2][0] = c[0]; XT[2][1] = c[1]; XT[2][2] = c[2]; 
	
	vtkMath::Multiply3x3(XT, e, ea);
	return fb;
}

void cnea( const vtkSmartPointer<vtkPoints> x, vtkSmartPointer<vtkPoints> ea, vtkSmartPointer<vtkDoubleArray> eb)
{
	double v1[3];
	double v2[3];
	double v3[3];

	double p0[3];
	double p1[3];
	double p2[3];

	double a[3];
	double b[3];
	double c[3];

	double d[3];
	double e[3];
	double f[3];

	x->GetPoint(0, p0);
	x->GetPoint(1, p1);
	x->GetPoint(2, p2);
	vfp(v1, p1, p0);
	vn(v1);
	vfp(v2, p2, p1);
	vn(v2);
	gf(a, b, c, v1, v2);
	double fa[3];
	double fb;
	fb = acos(vtkMath::Dot(v1, v2));
	fa[0] = 0; fa[1] = 0; fa[2] = 1; 
	ea->InsertNextPoint(fa);
	eb->InsertNextValue(fb);
	for(unsigned int i = 1; i < x->GetNumberOfPoints() - 3; i++)
	{
		x->GetPoint(i, p0);
		x->GetPoint(i + 1, p1);
		x->GetPoint(i + 2, p2);
		vfp(v1, p1, p0);
		vn(v1);
		fb = cne(fa, a, b, c, v1);
		aer(a, b, c, d, e, f, fa, fb);
		vn(d);
		vn(e);
		vn(f);
		ve(a, d);
		ve(b, e);
		ve(c, f);
		ea->InsertNextPoint(fa);
		eb->InsertNextValue(fb);
	}
}

void id2sub(int id, int dims[3], int sub[3])
{
	sub[2] = id / (dims[0] * dims[1]);
	sub[1] = (id - dims[0] * dims[1] * sub[2]) / dims[0];
	sub[0] = id - dims[0] * dims[1] * sub[2] - dims[0] * sub[1];
}

int sub2id(int x, int y, int z, int dims[3])
{
	return x + y * dims[0] + z * dims[0] * dims[1];
}

double sqrtSquare(double x[3], double y[3])
{
	return (x[0] - y[0]) * (x[0] - y[0]) + (x[1] - y[1]) * (x[1] - y[1]) + (x[2] - y[2]) * (x[2] - y[2]);
}

double norm(int x[3])
{
	return sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
}

double norm(double x[3])
{
	return sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
}

double normDist(int x[3], int y[3])
{
	return sqrt((x[0] - y[0]) * (x[0] - y[0]) + (x[1] - y[1]) * (x[1] - y[1]) + (x[2] - y[2]) * (x[2] - y[2]));
}

double normDist(double x[3], double y[3])
{
	return sqrt((x[0] - y[0]) * (x[0] - y[0]) + (x[1] - y[1]) * (x[1] - y[1]) + (x[2] - y[2]) * (x[2] - y[2]));
}

double angle(double x[3], double y[3], double d[3])
{
    double xy[3];
    xy[0] = y[0] - x[0];
    xy[1] = y[1] - x[1];
    xy[2] = y[2] - x[2];
    double normXy = norm(xy);
    if (normXy > 0)
    {
        xy[0] = xy[0] / normXy;
        xy[1] = xy[1] / normXy;
        xy[2] = xy[2] / normXy;
        double normD = norm(d);
        d[0] = d[0] / normD;
        d[1] = d[1] / normD;
        d[2] = d[2] / normD;
        return acos(xy[0] * d[0] + xy[1] * d[1] + xy[2] * d[2]);
    }
    else
    {
        return 3.1415;
    }
}

void getPerpDirection(double a[3], double b[3], double c[3], double o[3])
{
    double ab[3];
    ab[0] = b[0] - a[0];
    ab[1] = b[1] - a[1];
    ab[2] = b[2] - a[2];
    double bc[3];
    bc[0] = c[0] - b[0];
    bc[1] = c[1] - b[1];
    bc[2] = c[2] - b[2];
    double abcrossbc[3];
    cross(ab, bc, abcrossbc);
    cross(abcrossbc, ab, o);
    double no = norm(o);
    o[0] = o[0] / no;
    o[1] = o[1] / no;
    o[2] = o[2] / no;
}

void cross(double a[3], double b[3], double o[3])
{
    o[0] = a[1] * b[2] - a[2] * b[1];
    o[1] = a[2] * b[0] - a[0] * b[2];
    o[2] = a[0] * b[1] - a[1] * b[0];
}

double projectDist(double a[3], double b[3], double direction[3])
{
    double ab[3];
    ab[0] = a[0] - b[0];
    ab[1] = a[1] - b[1];
    ab[2] = a[2] - b[2];
    return ab[0] * direction[0] + ab[1] * direction[1] + ab[2] * direction[2];
}

void extropolate(double x1[3], double x2[3], double dist, double y[3])
{
    // y = x1 + (x1 - x2) / ||x1 - x2|| * dist
    double deltax[3];
    for (int i = 0; i < 3; i++)
    {
        deltax[i] = x1[i] - x2[i];
    }
    double normDeltax = norm(deltax);
    for (int i = 0; i < 3; i++)
    {
        y[i] = x1[i] + dist / normDeltax * deltax[i];
    }
}

void eig3volume(float dxx, float dxy, float dxz, float dyy, float dyz, float dzz, float c, double lambda[3])
{
	double ma[3][3] = { {dxx * c, dxy * c, dxz * c}, {dxy * c, dyy * c, dyz * c}, {dxz * c, dyz * c, dzz * c} };
	double davec[3][3];
	double daeig[3];
	eigen_decomposition(ma, davec, daeig);
	lambda[0] = daeig[0];
	lambda[1] = daeig[1];
	lambda[2] = daeig[2];
}

void tred2(double V[3][3], double d[3], double e[3]) {

	/*  This is derived from the Algol procedures tred2 by */
	/*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
	/*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
	/*  Fortran subroutine in EISPACK. */
	int i, j, k;
	double scale;
	double f, g, h;
	double hh;
	for (j = 0; j < 3; j++) { d[j] = V[3 - 1][j]; }

	/* Householder reduction to tridiagonal form. */

	for (i = 3 - 1; i > 0; i--) {
		/* Scale to avoid under/overflow. */
		scale = 0.0;
		h = 0.0;
		for (k = 0; k < i; k++) { scale = scale + fabs(d[k]); }
		if (scale == 0.0) {
			e[i] = d[i - 1];
			for (j = 0; j < i; j++) { d[j] = V[i - 1][j]; V[i][j] = 0.0;  V[j][i] = 0.0; }
		}
		else {

			/* Generate Householder vector. */

			for (k = 0; k < i; k++) { d[k] /= scale; h += d[k] * d[k]; }
			f = d[i - 1];
			g = sqrt(h);
			if (f > 0) { g = -g; }
			e[i] = scale * g;
			h = h - f * g;
			d[i - 1] = f - g;
			for (j = 0; j < i; j++) { e[j] = 0.0; }

			/* Apply similarity transformation to remaining columns. */

			for (j = 0; j < i; j++) {
				f = d[j];
				V[j][i] = f;
				g = e[j] + V[j][j] * f;
				for (k = j + 1; k <= i - 1; k++) { g += V[k][j] * d[k]; e[k] += V[k][j] * f; }
				e[j] = g;
			}
			f = 0.0;
			for (j = 0; j < i; j++) { e[j] /= h; f += e[j] * d[j]; }
			hh = f / (h + h);
			for (j = 0; j < i; j++) { e[j] -= hh * d[j]; }
			for (j = 0; j < i; j++) {
				f = d[j]; g = e[j];
				for (k = j; k <= i - 1; k++) { V[k][j] -= (f * e[k] + g * d[k]); }
				d[j] = V[i - 1][j];
				V[i][j] = 0.0;
			}
		}
		d[i] = h;
	}

	/* Accumulate transformations. */

	for (i = 0; i < 3 - 1; i++) {
		V[3 - 1][i] = V[i][i];
		V[i][i] = 1.0;
		h = d[i + 1];
		if (h != 0.0) {
			for (k = 0; k <= i; k++) { d[k] = V[k][i + 1] / h; }
			for (j = 0; j <= i; j++) {
				g = 0.0;
				for (k = 0; k <= i; k++) { g += V[k][i + 1] * V[k][j]; }
				for (k = 0; k <= i; k++) { V[k][j] -= g * d[k]; }
			}
		}
		for (k = 0; k <= i; k++) { V[k][i + 1] = 0.0; }
	}
	for (j = 0; j < 3; j++) { d[j] = V[3 - 1][j]; V[3 - 1][j] = 0.0; }
	V[3 - 1][3 - 1] = 1.0;
	e[0] = 0.0;
}

double hypot2(double x, double y) { return sqrt(x*x + y*y); }

/* Symmetric tridiagonal QL algorithm. */
void tql2(double V[3][3], double d[3], double e[3]) {

	/*  This is derived from the Algol procedures tql2, by */
	/*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
	/*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
	/*  Fortran subroutine in EISPACK. */

	int i, j, k, l, m;
	double f;
	double tst1;
	double eps;
	int iter;
	double g, p, r;
	double dl1, h, c, c2, c3, el1, s, s2;

	for (i = 1; i < 3; i++) { e[i - 1] = e[i]; }
	e[3 - 1] = 0.0;

	f = 0.0;
	tst1 = 0.0;
	eps = pow(2.0, -52.0);
	for (l = 0; l < 3; l++) {

		/* Find small subdiagonal element */

		tst1 = MAX(tst1, fabs(d[l]) + fabs(e[l]));
		m = l;
		while (m < 3) {
			if (fabs(e[m]) <= eps*tst1) { break; }
			m++;
		}

		/* If m == l, d[l] is an eigenvalue, */
		/* otherwise, iterate. */

		if (m > l) {
			iter = 0;
			do {
				iter = iter + 1;  /* (Could check iteration count here.) */
								  /* Compute implicit shift */
				g = d[l];
				p = (d[l + 1] - g) / (2.0 * e[l]);
				r = hypot2(p, 1.0);
				if (p < 0) { r = -r; }
				d[l] = e[l] / (p + r);
				d[l + 1] = e[l] * (p + r);
				dl1 = d[l + 1];
				h = g - d[l];
				for (i = l + 2; i < 3; i++) { d[i] -= h; }
				f = f + h;
				/* Implicit QL transformation. */
				p = d[m]; c = 1.0; c2 = c; c3 = c;
				el1 = e[l + 1]; s = 0.0; s2 = 0.0;
				for (i = m - 1; i >= l; i--) {
					c3 = c2;
					c2 = c;
					s2 = s;
					g = c * e[i];
					h = c * p;
					r = hypot2(p, e[i]);
					e[i + 1] = s * r;
					s = e[i] / r;
					c = p / r;
					p = c * d[i] - s * g;
					d[i + 1] = h + s * (c * g + s * d[i]);
					/* Accumulate transformation. */
					for (k = 0; k < 3; k++) {
						h = V[k][i + 1];
						V[k][i + 1] = s * V[k][i] + c * h;
						V[k][i] = c * V[k][i] - s * h;
					}
				}
				p = -s * s2 * c3 * el1 * e[l] / dl1;
				e[l] = s * p;
				d[l] = c * p;

				/* Check for convergence. */
			} while (fabs(e[l]) > eps*tst1);
		}
		d[l] = d[l] + f;
		e[l] = 0.0;
	}

	/* Sort eigenvalues and corresponding vectors. */
	for (i = 0; i < 3 - 1; i++) {
		k = i;
		p = d[i];
		for (j = i + 1; j < 3; j++) {
			if (d[j] < p) {
				k = j;
				p = d[j];
			}
		}
		if (k != i) {
			d[k] = d[i];
			d[i] = p;
			for (j = 0; j < 3; j++) {
				p = V[j][i];
				V[j][i] = V[j][k];
				V[j][k] = p;
			}
		}
	}
}

double absd(double val) { if (val>0) { return val; } else { return -val; } };

void eigen_decomposition(double A[3][3], double V[3][3], double d[3]) {
	double e[3];
	double da[3];
	double dt, dat;
	double vet[3];
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			V[i][j] = A[i][j];
		}
	}
	tred2(V, d, e);
	tql2(V, d, e);

	/* Sort the eigen values and vectors by abs eigen value */
	da[0] = absd(d[0]); da[1] = absd(d[1]); da[2] = absd(d[2]);
	if ((da[0] >= da[1]) && (da[0]>da[2]))
	{
		dt = d[2];   dat = da[2];    vet[0] = V[0][2];    vet[1] = V[1][2];    vet[2] = V[2][2];
		d[2] = d[0]; da[2] = da[0];  V[0][2] = V[0][0]; V[1][2] = V[1][0]; V[2][2] = V[2][0];
		d[0] = dt;   da[0] = dat;    V[0][0] = vet[0];  V[1][0] = vet[1];  V[2][0] = vet[2];
	}
	else if ((da[1] >= da[0]) && (da[1]>da[2]))
	{
		dt = d[2];   dat = da[2];    vet[0] = V[0][2];    vet[1] = V[1][2];    vet[2] = V[2][2];
		d[2] = d[1]; da[2] = da[1];  V[0][2] = V[0][1]; V[1][2] = V[1][1]; V[2][2] = V[2][1];
		d[1] = dt;   da[1] = dat;    V[0][1] = vet[0];  V[1][1] = vet[1];  V[2][1] = vet[2];
	}
	if (da[0]>da[1])
	{
		dt = d[1];   dat = da[1];    vet[0] = V[0][1];    vet[1] = V[1][1];    vet[2] = V[2][1];
		d[1] = d[0]; da[1] = da[0];  V[0][1] = V[0][0]; V[1][1] = V[1][0]; V[2][1] = V[2][0];
		d[0] = dt;   da[0] = dat;    V[0][0] = vet[0];  V[1][0] = vet[1];  V[2][0] = vet[2];
	}


}

void threePoints2volPoints(int origDims[3], int firstSub[3], int lastSub[3], int seedSub[3], int newOrigSub[3], int finalSub[3])
{
	newOrigSub[0] = seedSub[0];
	finalSub[0] = seedSub[0];
	if (firstSub[0] < seedSub[0] && firstSub[0] <= lastSub[0])
	{
		newOrigSub[0] = firstSub[0];
	}
	if (lastSub[0] < seedSub[0] && lastSub[0] <= firstSub[0])
	{
		newOrigSub[0] = lastSub[0];
	}
	if (firstSub[0] > seedSub[0] && firstSub[0] >= lastSub[0])
	{
		finalSub[0] = firstSub[0];
	}
	if (lastSub[0] > seedSub[0] && lastSub[0] >= firstSub[0])
	{
		finalSub[0] = lastSub[0];
	}

	newOrigSub[1] = seedSub[1];
	finalSub[1] = seedSub[1];
	if (firstSub[1] < seedSub[1] && firstSub[1] <= lastSub[1])
	{
		newOrigSub[1] = firstSub[1];
	}
	if (lastSub[1] < seedSub[1] && lastSub[1] <= firstSub[1])
	{
		newOrigSub[1] = lastSub[1];
	}
	if (firstSub[1] > seedSub[1] && firstSub[1] >= lastSub[1])
	{
		finalSub[1] = firstSub[1];
	}
	if (lastSub[1] > seedSub[1] && lastSub[1] >= firstSub[1])
	{
		finalSub[1] = lastSub[1];
	}

	newOrigSub[2] = seedSub[2];
	finalSub[2] = seedSub[2];
	if (firstSub[2] < seedSub[2] && firstSub[2] <= lastSub[2])
	{
		newOrigSub[2] = firstSub[2];
	}
	if (lastSub[2] < seedSub[2] && lastSub[2] <= firstSub[2])
	{
		newOrigSub[2] = lastSub[2];
	}
	if (firstSub[2] > seedSub[2] && firstSub[2] >= lastSub[2])
	{
		finalSub[2] = firstSub[2];
	}
	if (lastSub[2] > seedSub[2] && lastSub[2] >= firstSub[2])
	{
		finalSub[2] = lastSub[2];
	}

	if (finalSub[0] + 20 > origDims[0])
	{
		finalSub[0] = finalSub[0];
	}
	else
	{
		finalSub[0] = finalSub[0] + 20;
	}
	if (finalSub[1] + 20 > origDims[1])
	{
		finalSub[1] = finalSub[1];
	}
	else
	{
		finalSub[1] = finalSub[1] + 20;
	}
	if (finalSub[2] + 20 > origDims[2])
	{
		finalSub[2] = finalSub[2];
	}
	else
	{
		finalSub[2] = finalSub[2] + 20;
	}
	if (newOrigSub[0] - 20 < 0)
	{
		newOrigSub[0] = newOrigSub[0];
	}
	else
	{
		newOrigSub[0] = newOrigSub[0] - 20;
	}
	if (newOrigSub[1] - 20 < 0)
	{
		newOrigSub[1] = newOrigSub[1];
	}
	else
	{
		newOrigSub[1] = newOrigSub[1] - 20;
	}
	if (newOrigSub[2] - 20 < 0)
	{
		newOrigSub[2] = newOrigSub[2];
	}
	else
	{
		newOrigSub[2] = newOrigSub[2] - 20;
	}
}

void onePoint2volPoints(int origDims[3], int seedpointSub[3], int origPointSub[3], int finalPointSub[3])
{
	if (seedpointSub[0] - 30 < 0)
	{
		origPointSub[0] = seedpointSub[0];
	}
	else
	{
		origPointSub[0] = seedpointSub[0] - 30;
	}
	if (seedpointSub[1] - 30 < 0)
	{
		origPointSub[1] = seedpointSub[1];
	}
	else
	{
		origPointSub[1] = seedpointSub[1] - 30;
	}
	if (seedpointSub[2] - 30 < 0)
	{
		origPointSub[2] = seedpointSub[2];
	}
	else
	{
		origPointSub[2] = seedpointSub[2] - 30;
	}

	if (seedpointSub[0] + 30 > origDims[0])
	{
		finalPointSub[0] = seedpointSub[0];
	}
	else
	{
		finalPointSub[0] = seedpointSub[0] + 30;
	}
	if (seedpointSub[1] + 30 > origDims[1])
	{
		finalPointSub[1] = seedpointSub[1];
	}
	else
	{
		finalPointSub[1] = seedpointSub[1] + 30;
	}
	if (seedpointSub[2] + 30 > origDims[2])
	{
		finalPointSub[2] = seedpointSub[2];
	}
	else
	{
		finalPointSub[2] = seedpointSub[2] + 30;
	}
}

void endPointCompute(double pA[3],double pB[3],double pC[3],double pAbbar[3])
{
	double distBC = sqrtSquare(pC, pB);
	double AB[3];
	vfp(AB, pB, pA);
	vn(AB);
	double pAbar[3];
	pAbar[0] = pB[0] + distBC*AB[0];
	pAbar[1] = pB[1] + distBC*AB[1];
	pAbar[2] = pB[2] + distBC*AB[2];

	double pD[3];
	double BC[3], BAbar[3];
	vfp(BC, pC, pB);
	vfp(BAbar, pAbar, pB);
	double dotproduction = vtkMath::Dot(BAbar, BC);
	vn(BC);
	pD[0] = pB[0] + dotproduction*BC[0] / distBC;
	pD[1] = pB[1] + dotproduction*BC[1] / distBC;
	pD[2] = pB[2] + dotproduction*BC[2] / distBC;

	//double pAbbar[3];
	pAbbar[0] = pD[0] * 2 - pAbar[0];
	pAbbar[1] = pD[1] * 2 - pAbar[1];
	pAbbar[2] = pD[2] * 2 - pAbar[2];
}

void endPointCompute0(double pA[3], double pB[3], double pC[3], double pCbbar[3])
{
	double distAB = sqrtSquare(pA,pB);
	double distBC = sqrtSquare(pB, pC);
	double pCbar[3];
	pCbar[0] = pA[0] * distBC / (distBC + distAB) + pC[0] * distAB / (distAB + distBC);
	pCbar[1] = pA[1] * distBC / (distBC + distAB) + pC[1] * distAB / (distAB + distBC);
	pCbar[2] = pA[2] * distBC / (distBC + distAB) + pC[2] * distAB / (distAB + distBC);

	double BCbar[3];
	vfp(BCbar, pCbar, pB);
	vn(BCbar);
	pCbbar[0] = pB[0] + distBC*BCbar[0];
	pCbbar[1] = pB[1] + distBC*BCbar[1];
	pCbbar[2] = pB[2] + distBC*BCbar[2];
}