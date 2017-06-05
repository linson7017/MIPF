#ifndef TestTool_h__
#define TestTool_h__

#include "mitkPaintbrushTool.h"

#pragma once

namespace mitk
{

class TestTool : public PaintbrushTool
{
public:
    mitkClassMacro(TestTool, PaintbrushTool);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

        virtual const char **GetXPM() const override;
    virtual us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    virtual const char *GetName() const override;

protected:
    TestTool(); // purposely hidden
    virtual ~TestTool();
};

} // namespace


#endif // TestTool_h__
