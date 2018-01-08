#ifndef IQF_FastGrowCutSegmentation_h__
#define IQF_FastGrowCutSegmentation_h__

#include "IQF_Object.h"
#include "iconfig.h"

class IQF_FastGrowCutSegmentation:public IQF_Object
{
    BUILD_INTERFACE(IQF_FastGrowCutSegmentation)
public:
	//设置原图像
	virtual void SetSourceImage(vtkImageData* vtkSourceImage)=0;
	//设置种子图像
	virtual void SetSeedlImage(vtkImageData* vtkSeedImage)=0;
	//设置工作模式，创建新的分割实例时请设置为false
	virtual void SetWorkMode(bool bSegInitialized)=0;
	//virtual void SetImageSize(const std::vector<long>& imSize)=0;

	//Init之前请务必设置好原图像和种子图像
	virtual void Init() = 0;
	//设置好以上4步，运行分割算法
	virtual void DoSegmentation()=0;
	//将结果图像存放于形参图像中
	virtual void GetForegroundmage(vtkImageData* vtkResultImage)=0;

};

#endif // IQF_AirwaySegmentation_h__