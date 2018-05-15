#include "ImageReslice.h"
  

//mitk
#include "mitkDataNode.h"
#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkBaseRenderer.h"
#include "mitkExtractSliceFilter.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "vtkMitkLevelWindowFilter.h"
#include "vtkMitkThickSlicesFilter.h"
#include "mitkResliceMethodProperty.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkPlaneClipping.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkRenderingModeProperty.h"

//vtk
#include "vtkImageData.h"
#include "vtkSmartPointer.h" 
#include "vtkImageExtractComponents.h"

#include "qf_log.h"

void ApplyLevelWindow(mitk::DataNode* imageNode, mitk::BaseRenderer* renderer, vtkMitkLevelWindowFilter* levelWindowFilter)
{
    mitk::LevelWindow levelWindow;
    imageNode->GetLevelWindow(levelWindow, renderer, "levelwindow");
    levelWindowFilter->GetLookupTable()->SetRange(levelWindow.GetLowerWindowBound(),
        levelWindow.GetUpperWindowBound());

    mitk::LevelWindow opacLevelWindow;
    if (imageNode->GetLevelWindow(opacLevelWindow, renderer, "opaclevelwindow"))
    {
        // pass the opaque level window to the filter
        levelWindowFilter->SetMinOpacity(opacLevelWindow.GetLowerWindowBound());
        levelWindowFilter->SetMaxOpacity(opacLevelWindow.GetUpperWindowBound());
    }
    else
    {
        // no opaque level window
        levelWindowFilter->SetMinOpacity(0.0);
        levelWindowFilter->SetMaxOpacity(255.0);
    }
}

void ApplyColorTransferFunction(mitk::DataNode* imageNode, mitk::BaseRenderer* renderer, vtkMitkLevelWindowFilter* levelWindowFilter)
{
    mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty *>(
        imageNode->GetProperty("Image Rendering.Transfer Function", renderer));

    if (transferFunctionProp.IsNull())
    {
        QF_ERROR << "'Image Rendering.Mode'' was set to use a color transfer function but there is no property 'Image "
            "Rendering.Transfer Function'. Nothing will be done.";
        return;
    }
    // pass the transfer function to our level window filter
    levelWindowFilter->SetLookupTable(transferFunctionProp->GetValue()->GetColorTransferFunction());
    levelWindowFilter->SetOpacityPiecewiseFunction(
        transferFunctionProp->GetValue()->GetScalarOpacityFunction());
}


void ApplyLookuptable(mitk::DataNode* imageNode, mitk::BaseRenderer* renderer, vtkMitkLevelWindowFilter* levelWindowFilter)
{
    // If lookup table or transferfunction use is requested...
    mitk::LookupTableProperty::Pointer lookupTableProp =
        dynamic_cast<mitk::LookupTableProperty *>(imageNode->GetProperty("LookupTable"));

    vtkLookupTable *usedLookupTable = vtkSmartPointer<vtkLookupTable>::New();
    if (lookupTableProp.IsNotNull()) // is a lookuptable set?
    {
        usedLookupTable = lookupTableProp->GetLookupTable()->GetVtkLookupTable();
    }
    else
    {
        //"Image Rendering.Mode was set to use a lookup table but there is no property 'LookupTable'.
        // A default (rainbow) lookup table will be used.
        // Here have to do nothing. Warning for the user has been removed, due to unwanted console output
        // in every interation of the rendering.
    }
    levelWindowFilter->SetLookupTable(usedLookupTable);
}


ImageReslice::ImageReslice()
{
}


ImageReslice::~ImageReslice()
{
}

bool ImageReslice::GetReslicePlaneImage(mitk::DataNode* imageNode, const mitk::PlaneGeometry* worldGeometry, vtkImageData* output)
{
    mitk::Image * mitkImage = dynamic_cast<mitk::Image*>(imageNode->GetData());
    if (mitkImage)
    {
        return GetReslicePlaneImage(mitkImage->GetVtkImageData(), worldGeometry, output);
    }
    else
    {
        return false;
    }
    
}

bool ImageReslice::GetReslicePlaneImage(vtkImageData* image, const mitk::PlaneGeometry* worldGeometry, vtkImageData* output)
{
    vtkSmartPointer<vtkImageReslice> reslicer = vtkSmartPointer<vtkImageReslice>::New();
    reslicer->SetInputData(image);
    reslicer->SetOutputExtent(0, worldGeometry->GetExtent(0),
        0, worldGeometry->GetExtent(1),
        0, 0);
    reslicer->SetResliceAxes(((mitk::PlaneGeometry*)worldGeometry)->GetVtkMatrix());
    reslicer->Update();
    output->DeepCopy(reslicer->GetOutput());
    return true;
}

void ApplyRenderingMode(mitk::DataNode* imageNode, mitk::BaseRenderer* renderer, vtkMitkLevelWindowFilter* levelWindowFilter)
{
    bool binary = false;
    imageNode->GetBoolProperty("binary", binary, renderer);
    if (binary) // is it a binary image?
    {
        // for binary images, we always use our default LuT and map every value to (0,1)
        // the opacity of 0 will always be 0.0. We never a apply a LuT/TfF nor a level window.

        mitk::LookupTable::Pointer mitkLUT = mitk::LookupTable::New();
        levelWindowFilter->SetLookupTable(mitkLUT->GetVtkLookupTable());
    }
    else
    {
        // all other image types can make use of the rendering mode
        int renderingMode = mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR;
        mitk::RenderingModeProperty::Pointer mode =
            dynamic_cast<mitk::RenderingModeProperty *>(imageNode->GetProperty("Image Rendering.Mode", renderer));
        if (mode.IsNotNull())
        {
            renderingMode = mode->GetRenderingMode();
        }
        switch (renderingMode)
        {
        case mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR:
            MITK_DEBUG << "'Image Rendering.Mode' = LevelWindow_LookupTable_Color";
            ApplyLookuptable(imageNode,renderer,levelWindowFilter);
            ApplyLevelWindow(imageNode, renderer, levelWindowFilter);
            break;
        case mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR:
            MITK_DEBUG << "'Image Rendering.Mode' = LevelWindow_ColorTransferFunction_Color";
            ApplyColorTransferFunction(imageNode, renderer, levelWindowFilter);
            ApplyLevelWindow(imageNode, renderer, levelWindowFilter);
            break;
        case mitk::RenderingModeProperty::LOOKUPTABLE_COLOR:
            MITK_DEBUG << "'Image Rendering.Mode' = LookupTable_Color";
            ApplyLookuptable(imageNode, renderer, levelWindowFilter);
            break;
        case mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR:
            MITK_DEBUG << "'Image Rendering.Mode' = ColorTransferFunction_Color";
            ApplyColorTransferFunction(imageNode, renderer, levelWindowFilter);
            break;
        default:
            QF_ERROR << "No valid 'Image Rendering.Mode' set. Using LOOKUPTABLE_LEVELWINDOW_COLOR instead.";
            ApplyLookuptable(imageNode, renderer, levelWindowFilter);
            ApplyLevelWindow(imageNode, renderer, levelWindowFilter);
            break;
        }
    }
    // we apply color for all images (including binaries).
   // ApplyColor(imageNode, renderer, levelWindowFilter);
}




bool ImageReslice::GetReslicePlaneImageWithLevelWindow(mitk::DataNode* imageNode, const mitk::PlaneGeometry* worldGeometry, mitk::BaseRenderer* renderer, vtkImageData* output)
{

    mitk::Image *image = dynamic_cast<mitk::Image *>(imageNode->GetData());
    mitk::DataNode *datanode = imageNode;
    if (nullptr == image || !image->IsInitialized())
    {
        return false;
    }

    // check if there is a valid worldGeometry
   // const mitk::PlaneGeometry *worldGeometry = renderer->GetCurrentWorldPlaneGeometry();
    if (nullptr == worldGeometry || !worldGeometry->IsValid() || !worldGeometry->HasReferenceGeometry())
    {
        return false;
    }

    image->Update();


    //new 
    auto reslicedImage = vtkSmartPointer<vtkImageData>::New();
    auto levelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
    mitk::ExtractSliceFilter::Pointer  reslicer = mitk::ExtractSliceFilter::New();

    // set main input for ExtractSliceFilter
    reslicer->SetInput(image);
    reslicer->SetWorldGeometry(worldGeometry);
    reslicer->SetTimeStep(renderer->GetTimeStep(image));

    // set the transformation of the image to adapt reslice axis
    reslicer->SetResliceTransformByGeometry(
        image->GetTimeGeometry()->GetGeometryForTimeStep(renderer->GetTimeStep(image)));

    // is the geometry of the slice based on the input image or the worldgeometry?
    bool inPlaneResampleExtentByGeometry = false;
    datanode->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
    reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);

    // Initialize the interpolation mode for resampling; switch to nearest
    // neighbor if the input image is too small.
    if ((image->GetDimension() >= 3) && (image->GetDimension(2) > 1))
    {
        mitk::VtkResliceInterpolationProperty *resliceInterpolationProperty;
        datanode->GetProperty(resliceInterpolationProperty, "reslice interpolation", renderer);

        int interpolationMode = VTK_RESLICE_NEAREST;
        if (resliceInterpolationProperty != NULL)
        {
            interpolationMode = resliceInterpolationProperty->GetInterpolation();
        }

        switch (interpolationMode)
        {
        case VTK_RESLICE_NEAREST:
            reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
            break;
        case VTK_RESLICE_LINEAR:
            reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_LINEAR);
            break;
        case VTK_RESLICE_CUBIC:
            reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_CUBIC);
            break;
        }
    }
    else
    {
        reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
    }

    // set the vtk output property to true, makes sure that no unneeded mitk image convertion
    // is done.
    reslicer->SetVtkOutputRequest(true);

    // Thickslicing
    int thickSlicesMode = 0;
    int thickSlicesNum = 1;
    // Thick slices parameters
    if (image->GetPixelType().GetNumberOfComponents() == 1) // for now only single component are allowed
    {
        mitk::DataNode *dn = renderer->GetCurrentWorldPlaneGeometryNode();
        if (dn)
        {
            mitk::ResliceMethodProperty *resliceMethodEnumProperty = 0;

            if (dn->GetProperty(resliceMethodEnumProperty, "reslice.thickslices", renderer) && resliceMethodEnumProperty)
                thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

            mitk::IntProperty *intProperty = 0;
            if (dn->GetProperty(intProperty, "reslice.thickslices.num", renderer) && intProperty)
            {
                thickSlicesNum = intProperty->GetValue();
                if (thickSlicesNum < 1)
                    thickSlicesNum = 1;
            }
        }
        else
        {
            QF_WARN << "no associated widget plane data tree node found";
        }
    }

    const mitk::PlaneGeometry *planeGeometry = dynamic_cast<const mitk::PlaneGeometry *>(worldGeometry);

    if (thickSlicesMode > 0)
    {
        double dataZSpacing = 1.0;

        mitk::Vector3D normInIndex, normal;

        const mitk::AbstractTransformGeometry *abstractGeometry =
            dynamic_cast<const mitk::AbstractTransformGeometry *>(worldGeometry);
        if (abstractGeometry != NULL)
            normal = abstractGeometry->GetPlane()->GetNormal();
        else
        {
            if (planeGeometry != NULL)
            {
                normal = planeGeometry->GetNormal();
            }
            else
                return false; // no fitting geometry set
        }
        normal.Normalize();

        image->GetTimeGeometry()->GetGeometryForTimeStep(renderer->GetTimeStep(image))->WorldToIndex(normal, normInIndex);

        dataZSpacing = 1.0 / normInIndex.GetNorm();

        reslicer->SetOutputDimensionality(3);
        reslicer->SetOutputSpacingZDirection(dataZSpacing);
        reslicer->SetOutputExtentZDirection(-thickSlicesNum, 0 + thickSlicesNum);

        // Do the reslicing. Modified() is called to make sure that the reslicer is
        // executed even though the input geometry information did not change; this
        // is necessary when the input /em data, but not the /em geometry changes.
        auto pTSFilter = vtkSmartPointer<vtkMitkThickSlicesFilter>::New();
        pTSFilter->SetThickSliceMode(thickSlicesMode - 1);
        pTSFilter->SetInputData(reslicer->GetVtkOutput());

        // vtkFilter=>mitkFilter=>vtkFilter update mechanism will fail without calling manually
        reslicer->Modified();
        reslicer->Update();

        pTSFilter->Modified();
        pTSFilter->Update();
        reslicedImage = pTSFilter->GetOutput();
    }
    else
    {
        // this is needed when thick mode was enable bevore. These variable have to be reset to default values
        reslicer->SetOutputDimensionality(2);
        reslicer->SetOutputSpacingZDirection(1.0);
        reslicer->SetOutputExtentZDirection(0, 0);

        reslicer->Modified();
        // start the pipeline with updating the largest possible, needed if the geometry of the input has changed
        reslicer->UpdateLargestPossibleRegion();
        reslicedImage = reslicer->GetVtkOutput();
    }

    // Bounds information for reslicing (only reuqired if reference geometry
    // is present)
    // this used for generating a vtkPLaneSource with the right size
    double sliceBounds[6];
    for (auto &sliceBound : sliceBounds)
    {
        sliceBound = 0.0;
    }
    reslicer->GetClippedPlaneBounds(sliceBounds);

    // get the spacing of the slice
    double* mmPerPixel = reslicer->GetOutputSpacing();

    // calculate minimum bounding rect of IMAGE in texture
    {
        double textureClippingBounds[6];
        for (auto &textureClippingBound : textureClippingBounds)
        {
            textureClippingBound = 0.0;
        }
        // Calculate the actual bounds of the transformed plane clipped by the
        // dataset bounding box; this is required for drawing the texture at the
        // correct position during 3D mapping.
        mitk::PlaneClipping::CalculateClippedPlaneBounds(image->GetGeometry(), planeGeometry, textureClippingBounds);

        textureClippingBounds[0] = static_cast<int>(textureClippingBounds[0] /mmPerPixel[0] + 0.5);
        textureClippingBounds[1] = static_cast<int>(textureClippingBounds[1] /mmPerPixel[0] + 0.5);
        textureClippingBounds[2] = static_cast<int>(textureClippingBounds[2] /mmPerPixel[1] + 0.5);
        textureClippingBounds[3] = static_cast<int>(textureClippingBounds[3] /mmPerPixel[1] + 0.5);

        // clipping bounds for cutting the image
        levelWindowFilter->SetClippingBounds(textureClippingBounds);
    }

    //// get the number of scalar components to distinguish between different image types
    int numberOfComponents = reslicedImage->GetNumberOfScalarComponents();
    //// get the binary property
    //bool binary = false;
    //bool binaryOutline = false;
    //datanode->GetBoolProperty("binary", binary, renderer);
    //if (binary) // binary image
    //{
    //    datanode->GetBoolProperty("outline binary", binaryOutline, renderer);
    //    if (binaryOutline) // contour rendering
    //    {
    //        // get pixel type of vtk image
    //        itk::ImageIOBase::IOComponentType componentType = static_cast<itk::ImageIOBase::IOComponentType>(image->GetPixelType().GetComponentType());
    //        switch (componentType)
    //        {
    //        case itk::ImageIOBase::UCHAR:
    //            // generate contours/outlines
    //            localStorage->m_OutlinePolyData = CreateOutlinePolyData<unsigned char>(renderer);
    //            break;
    //        case itk::ImageIOBase::USHORT:
    //            // generate contours/outlines
    //            localStorage->m_OutlinePolyData = CreateOutlinePolyData<unsigned short>(renderer);
    //            break;
    //        default:
    //            binaryOutline = false;
    //            this->ApplyLookuptable(renderer);
    //            QF_WARN << "Type of all binary images should be unsigned char or unsigned short. Outline does not work on other pixel types!";
    //        }
    //        if (binaryOutline) // binary outline is still true --> add outline
    //        {
    //            float binaryOutlineWidth = 1.0;
    //            if (datanode->GetFloatProperty("outline width", binaryOutlineWidth, renderer))
    //            {
    //                if (localStorage->m_Actors->GetNumberOfPaths() > 1)
    //                {
    //                    float binaryOutlineShadowWidth = 1.5;
    //                    datanode->GetFloatProperty("outline shadow width", binaryOutlineShadowWidth, renderer);

    //                    dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))
    //                        ->GetProperty()
    //                        ->SetLineWidth(binaryOutlineWidth * binaryOutlineShadowWidth);
    //                }
    //                localStorage->m_Actor->GetProperty()->SetLineWidth(binaryOutlineWidth);
    //            }
    //        }
    //    }
    //    else // standard binary image
    //    {
    //        if (numberOfComponents != 1)
    //        {
    //            QF_ERROR << "Rendering Error: Binary Images with more then 1 component are not supported!";
    //        }
    //    }
    //}

    //ApplyOpacity(imageNode,renderer);
    ApplyRenderingMode(imageNode,renderer, levelWindowFilter);

    // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
   // localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();

    int displayedComponent = 0;

    if (datanode->GetIntProperty("Image.Displayed Component", displayedComponent, renderer) && numberOfComponents > 1)
    {
        auto vectorComponentExtractor = vtkSmartPointer<vtkImageExtractComponents>::New();
        vectorComponentExtractor->SetComponents(displayedComponent);
        vectorComponentExtractor->SetInputData(reslicedImage);
        levelWindowFilter->SetInputConnection(vectorComponentExtractor->GetOutputPort(0));
    }
    else
    {
        // connect the input with the levelwindow filter
        levelWindowFilter->SetInputData(reslicedImage);
    }

    levelWindowFilter->Update();
    
    output->DeepCopy(levelWindowFilter->GetOutput());
    return true;

    // check for texture interpolation property
  //  bool textureInterpolation = false;
   // imageNode->GetBoolProperty("texture interpolation", textureInterpolation, renderer);

    // set the interpolation modus according to the property
    //localStorage->m_Texture->SetInterpolate(textureInterpolation);

    // connect the texture with the output of the levelwindow filter
    //localStorage->m_Texture->SetInputConnection(levelWindowFilter->GetOutputPort());

    //this->TransformActor(renderer);

   // vtkActor *contourShadowActor = dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0));

    //if (binary && binaryOutline) // connect the mapper with the polyData which contains the lines
    //{
    //    // We need the contour for the binary outline property as actor
    //    localStorage->m_Mapper->SetInputData(localStorage->m_OutlinePolyData);
    //    localStorage->m_Actor->SetTexture(nullptr); // no texture for contours

    //    bool binaryOutlineShadow = false;
    //    datanode->GetBoolProperty("outline binary shadow", binaryOutlineShadow, renderer);
    //    if (binaryOutlineShadow)
    //    {
    //        contourShadowActor->SetVisibility(true);
    //    }
    //    else
    //    {
    //        contourShadowActor->SetVisibility(false);
    //    }
    //}
    //else
    //{ // Connect the mapper with the input texture. This is the standard case.
    //    // setup the textured plane
    //    this->GeneratePlane(renderer, sliceBounds);
    //    // set the plane as input for the mapper
    //    localStorage->m_Mapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
    //    // set the texture for the actor

    //    localStorage->m_Actor->SetTexture(localStorage->m_Texture);
    //    contourShadowActor->SetVisibility(false);
    //}

    // We have been modified => save this for next Update()
  //  localStorage->m_LastUpdateTime.Modified();
}
