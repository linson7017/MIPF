#include "util.h"





unsigned int getKernelSize(double sigma, unsigned int order)
{

  // A size factor that makes the filter kernel longer when
  // derivatives of the Gaussian are wanted. Suitable numbers are:
  // order 0 (no derivative) : sizeFactor = 2.5
  // order 1 (1st derivative): sizeFactor = 3.25
  // order 2 (2nd derivative): sizeFactor = 4
  const double sizeFactor = 2.5 + static_cast<double>(order);

  // Return suitable size
  return (0 >= sigma) ? 1 :  2*static_cast<unsigned int>(ceil(sizeFactor*sigma))+1;
}

void get1DGaussKernel(double sigma, std::valarray<double>& kernel, unsigned int order, double voxelSize, bool multiScaleNormalize)
{
  // Negative or zero sigma not allowed, set kernel = 1.
  if(sigma<=0) {
    kernel.resize(1);
    kernel[0] = 1.0;
    return;
  }

  // Recalculate sigma in voxels. voxelSize should be 1 if a filter in voxel units is wanted.
  const double sigmaVoxel = sigma/voxelSize;

  // Get kernel size
  const int N = getKernelSize(sigmaVoxel, order);
  const int Ndiv2 = (N-1)/2;

  // Resize vector to fit kernel
  kernel.resize(N);

  // Calculate Gaussian and normalization constant
  kernel[Ndiv2] = 1.0;    // Middle coefficient is always 1 ...
  double s = 1.0;         // ... therefore, normalization factor starts at 1.0.
  for(int x1Idx=1; x1Idx<=Ndiv2; ++x1Idx) {
    kernel[Ndiv2+x1Idx] = exp(-x1Idx*x1Idx/(2.0*sigmaVoxel*sigmaVoxel)); // Sigma cannot be zero due to check above
    kernel[Ndiv2-x1Idx] = kernel[Ndiv2+x1Idx];
    s += 2.0*kernel[Ndiv2+x1Idx];
  }

  // Normalize kernel so that sum becomes 1.
  for(int x2Idx=0; x2Idx<N; ++x2Idx) {
    kernel[x2Idx] /= s;
  }

  // Multiplicative factor to be used when the multi-scale normalization a'la
  // T. Lindeberg is applied, see comments above for more info. If multiScaleNormalize
  // is true the scale factor equals sigma, otherwise no normalization is performed and
  // the factor is set to 1.
  // NOTE that it is scaled with sigma and not sigmaVoxel here - this has experimentally
  // been found the right thing to do. Otherwise will the gradient be twice as large
  // when the voxel size in one dimension is twice the size of another dimension.
  const double multiScaleFactor = multiScaleNormalize ? sigma : 1.0;

  // Differentiate kernel
  if(0 == order) {
    return;
  }
  else if(1 == order) {
    // If g(x) = exp(-x^2/(2*sigma^2)) we have g'(x) = -x/sigma^2*g(x).
    // Multiply the Gaussian kernel g(x) with -x/sigma^2 to get derivative.
    // To get everything right we also have to scale with the voxel size so
    // that we get (intensity change)/mm instead of (intensity change)/voxel.
    for(int x3Idx=0; x3Idx<=Ndiv2; ++x3Idx) {
      const double polyfactor = multiScaleFactor * x3Idx/(sigmaVoxel*sigmaVoxel) / voxelSize;
      kernel[Ndiv2+x3Idx] *= polyfactor;
      kernel[Ndiv2-x3Idx] = -kernel[Ndiv2+x3Idx];
    }
  }
  else if (2 == order) {
    // If g(x) = exp(-x^2/(2*sigma^2)) we have g''(x) = (x^2/sigma^2-1)/sigma^2 * g(x).
    // Multiply the Gaussian kernel g(x) with (x^2/sigma^2-1)/sigma^2 to get 2nd derivative.
    // To get everything right we also have to scale with the voxel size so
    // that we get (intensity change)/mm^2 instead of (intensity change)/voxel^2.
    const double sigmaVoxel2 = sigmaVoxel*sigmaVoxel;
    for(int x4Idx=0; x4Idx<=Ndiv2; ++x4Idx) {
      const double polyfactor = multiScaleFactor * multiScaleFactor * (x4Idx*x4Idx/sigmaVoxel2 - 1.0)/sigmaVoxel2 / (voxelSize*voxelSize);
      kernel[Ndiv2+x4Idx] *= polyfactor;
      kernel[Ndiv2-x4Idx] = kernel[Ndiv2+x4Idx];
    }
  }
}
