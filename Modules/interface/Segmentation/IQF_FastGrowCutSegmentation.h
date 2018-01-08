#ifndef IQF_FastGrowCutSegmentation_h__
#define IQF_FastGrowCutSegmentation_h__

#include "IQF_Object.h"
#include "iconfig.h"

class IQF_FastGrowCutSegmentation:public IQF_Object
{
    BUILD_INTERFACE(IQF_FastGrowCutSegmentation)
public:
	//����ԭͼ��
	virtual void SetSourceImage(vtkImageData* vtkSourceImage)=0;
	//��������ͼ��
	virtual void SetSeedlImage(vtkImageData* vtkSeedImage)=0;
	//���ù���ģʽ�������µķָ�ʵ��ʱ������Ϊfalse
	virtual void SetWorkMode(bool bSegInitialized)=0;
	//virtual void SetImageSize(const std::vector<long>& imSize)=0;

	//Init֮ǰ��������ú�ԭͼ�������ͼ��
	virtual void Init() = 0;
	//���ú�����4�������зָ��㷨
	virtual void DoSegmentation()=0;
	//�����ͼ�������β�ͼ����
	virtual void GetForegroundmage(vtkImageData* vtkResultImage)=0;

};

#endif // IQF_AirwaySegmentation_h__