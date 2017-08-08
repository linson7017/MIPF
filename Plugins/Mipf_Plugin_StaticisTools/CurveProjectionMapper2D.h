#ifndef CurveProjectionMapper2D_h__
#define CurveProjectionMapper2D_h__

#include "mitkCommon.h"
#include "mitkGLMapper.h"

#pragma once

namespace mitk
{
    class CurveProjectionMapper2D :public mitk::GLMapper
    {
    public:
        mitkClassMacro(CurveProjectionMapper2D, GLMapper);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);

        virtual void Paint(BaseRenderer *renderer) override;
        void InitGLSL();
    protected:

        CurveProjectionMapper2D();
        ~CurveProjectionMapper2D();


        
    };
}



#endif // CurveProjectionMapper2D_h__
