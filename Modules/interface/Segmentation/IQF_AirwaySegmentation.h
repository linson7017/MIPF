#ifndef IQF_AirwaySegmentation_h__
#define IQF_AirwaySegmentation_h__

#include "IQF_Object.h"
#include "iconfig.h"
class TInputImageType;
class TOutputImageType;

class IQF_AirwaySegmentation:public IQF_Object
{
    BUILD_INTERFACE(IQF_AirwaySegmentation)
public:
	virtual int DoSegmentation()=0;
	virtual void SetSourceImage(TInputImageType* const sourceimage)=0;
	virtual void SetLabelImage(TOutputImageType* labelimage)=0;
	virtual void SetSeedPoint(const double* seed)=0;
};

#endif // IQF_AirwaySegmentation_h__