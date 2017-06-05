#include "TestTool.h"


#include "mitkDrawPaintbrushTool.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
    MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, DrawPaintbrushTool, "Paintbrush drawing tool");
}

mitk::TestTool::TestTool() : PaintbrushTool(1)
{
}

mitk::TestTool::~TestTool()
{
}

const char **mitk::TestTool::GetXPM() const
{
    static const char * mitkDrawPaintbrushTool_xpm[] = { " " };
    return mitkDrawPaintbrushTool_xpm;
    //return mitkDrawPaintbrushTool_xpm;
}

us::ModuleResource mitk::TestTool::GetIconResource() const
{
    us::Module *module = us::GetModuleContext()->GetModule();
    us::ModuleResource resource = module->GetResource("Paint_48x48.png");
    return resource;
}

us::ModuleResource mitk::TestTool::GetCursorIconResource() const
{
    us::Module *module = us::GetModuleContext()->GetModule();
    us::ModuleResource resource = module->GetResource("Paint_Cursor_32x32.png");
    return resource;
}

const char *mitk::TestTool::GetName() const
{
    return "TestTool";
}