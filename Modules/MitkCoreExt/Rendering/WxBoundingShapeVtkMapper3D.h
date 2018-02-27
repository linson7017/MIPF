/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#ifndef WxBoundingShapeVtkMapper3D_h
#define WxBoundingShapeVtkMapper3D_h

#include <mitkVtkMapper.h>

#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

#include "qf_config.h"

namespace mitk
{
  class QF_API WxBoundingShapeVtkMapper3D : public VtkMapper
  {
  public:
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    mitkClassMacro(WxBoundingShapeVtkMapper3D, VtkMapper) itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    void ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *) override;
    vtkProp *GetVtkProp(BaseRenderer *renderer) override;

  protected:
    void GenerateDataForRenderer(BaseRenderer *renderer) override;

  private:
    WxBoundingShapeVtkMapper3D();
    ~WxBoundingShapeVtkMapper3D();

    WxBoundingShapeVtkMapper3D(const Self &);
    Self &operator=(const Self &);

    class Impl;
    Impl *m_Impl;
  };
}

#endif
