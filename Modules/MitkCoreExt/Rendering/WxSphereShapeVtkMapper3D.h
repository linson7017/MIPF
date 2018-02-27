/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#ifndef WxSphereShapeVtkMapper3D_h__
#define WxSphereShapeVtkMapper3D_h__

#include <mitkVtkMapper.h>

#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

#include "qf_config.h"

namespace mitk
{
	class QF_API WxSphereShapeVtkMapper3D : public VtkMapper
	{
	public:
		static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

		mitkClassMacro(WxSphereShapeVtkMapper3D, VtkMapper) itkFactorylessNewMacro(Self) itkCloneMacro(Self)

		void ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *) override;
		vtkProp *GetVtkProp(BaseRenderer *renderer) override;
	protected:
		void GenerateDataForRenderer(BaseRenderer *renderer) override;

	private:
		WxSphereShapeVtkMapper3D();
		~WxSphereShapeVtkMapper3D();

		WxSphereShapeVtkMapper3D(const Self &);
		Self &operator=(const Self &);

		class Impl;
		Impl *m_Impl;
	};
} //namespace mitk

#endif // WxSphereShapeVtkMapper3D_h__
