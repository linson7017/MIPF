#include "imageAlgorithm.h"

#include "mathutil.h"

#include "vtkAngleWidget.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellPicker.h"
#include "vtkCleanPolyData.h"
#include "vtkCommand.h"
#include "vtkCubeSource.h"
#include "vtkDelaunay3D.h"
#include "vtkDistanceWidget.h"
#include "vtkDistanceRepresentation.h"
#include "vtkImageData.h"
#include "vtkLine.h"
#include "vtkLineSource.h"
#include "vtkGeometryFilter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursor.h"
#include "vtkSTLWriter.h"
#include "vtkSTLReader.h"
#include "vtkProperty.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkInteractorStyleImage.h"
#include "vtkImageSlabReslice.h"
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkHandleRepresentation.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkImageThreshold.h"
#include "vtkImageThresholdConnectivity.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkGraphicsFactory.h"
#include "vtkParametricFunctionSource.h"
#include "vtkPolyLine.h"
#include "vtkFeatureEdges.h"
#include "vtkTriangleFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkImageStencil.h"
#include "vtkMetaImageWriter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageGaussianSource.h"
#include "vtkVectorText.h"
#include "vtkFollower.h"
#include "vtkImageDilateErode3D.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkTubeFilter.h"
#include "vtkExtractVOI.h"
#include "vtkDelaunay3D.h"
#include "vtkDataSetSurfaceFilter.h"
//#include "vtkSurfaceReconstructionFilter.h"
//#include "vtkContourFilter.h"
#include "vtkImageConvolve.h"

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkBinaryThinningImageFilter3D.h"
#include "itkImage.h"
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"
#include "itkBinaryThinningImageFilter3D.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkVotingBinaryHoleFillingImageFilter.h"
#include "vtkVector.h"
#include "vtkTransform.h"
//#include "itkAdaptiveOtsuThresholdImageFilter.h"
#include "vtkvmtkPolyDataCenterlines.h"

// ITK types
typedef itk::Image<float, 3> ImageType;
//typedef itk::ImageSource<TOutputImage> OutputImgageTyepe;
typedef itk::VTKImageToImageFilter<ImageType> VTKToITKConnectorType;
typedef itk::ImageToVTKImageFilter<ImageType> ITKToVTKConnectorType;
typedef itk::BinaryThinningImageFilter3D<ImageType, ImageType> ThinningFilterType;
typedef itk::ImageSeriesReader<ImageType> ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;
typedef itk::BinaryFillholeImageFilter<ImageType> IMFillFilterType;
typedef itk::VotingBinaryHoleFillingImageFilter<ImageType,ImageType> VotingFillFilterType;
//typedef itk::AdaptiveOtsuThresholdImageFilter<ImageType, ImageType> ThresholdFilter;
//typedef itk::SymmetricSecondRankTensor<double, 3> HessianPixelType;
//typedef itk::Image<HessianPixelType, 3> HessianImageType;
//typedef itk::HessianToObjectnessMeasureImageFilter<HessianImageType, ImageType> ObjectnessFilterType;
//typedef itk::MultiScaleHessianBasedMeasureImageFilter<ImageType, HessianImageType, ImageType> MultiScaleEnhancementFilterType;

//ObjectnessFilterType::Pointer objectnessFilter;
//MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter;

ImageAlgorithm::ImageAlgorithm()
{
	/* Too slow
	objectnessFilter = ObjectnessFilterType::New();
	objectnessFilter->SetBrightObject(true);
	objectnessFilter->SetScaleObjectnessMeasure(false);
	objectnessFilter->SetAlpha(0.5);
	objectnessFilter->SetBeta(0.5);
	objectnessFilter->SetGamma(100);
	multiScaleEnhancementFilter = MultiScaleEnhancementFilterType::New();
	multiScaleEnhancementFilter->SetHessianToMeasureFilter(objectnessFilter);
	multiScaleEnhancementFilter->SetSigmaStepMethodToEquispaced();
	multiScaleEnhancementFilter->SetSigmaMinimum(1.0);
	multiScaleEnhancementFilter->SetSigmaMaximum(1.0);
	multiScaleEnhancementFilter->SetNumberOfSigmaSteps(1);
	*/
}

void ImageAlgorithm::destroy()
{
	this->skeletonNeighbors = NULL;
	this->currentSurface = NULL;
	this->currentSurfaceShowing = NULL;
	this->aneurysmData.aneurysmDisplayPolyData = NULL; 
	this->aneurysmData.aneurysmSurfaceData = NULL;
	this->pruneVesselData = NULL;
	this->vesselExtensionsData = NULL;
	this->vesselness = NULL;
	this->enhancedVesselData = NULL;
	this->aneurysmImageData = NULL;
	this->seedPoints = NULL;
	this->startEndPoints = NULL;
	this->aneurysmPoints = NULL;
	this->segmentedData = NULL;
	this->segmentedDataBackup = NULL;
	this->inputData = NULL;
	this->needleData = NULL;
	this->moulddata = NULL;
	this->tubeImageData = NULL;
	this->resampler = NULL;
}

bool ImageAlgorithm::readFolder(const char *dn)
{
	//vtkObject::GlobalWarningDisplayOff();
    // Initialize all vtk operators
    this->discreteSurface = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
    this->smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    //this->mouldsurface = vtkSmartPointer<vtkMarchingCubes>::New();
    
    this->seedPoints = vtkSmartPointer<vtkPoints>::New();
    this->startEndPoints = vtkSmartPointer<vtkPoints>::New();
    
    this->aneurysmPoints = vtkSmartPointer<vtkPoints>::New();
	this->aneurysmLocationPoints = vtkSmartPointer<vtkPoints>::New();
    
    this->graphToPolyData = vtkSmartPointer<vtkGraphToPolyData>::New();
    this->dijkstra = vtkSmartPointer<vtkDijkstraGraphGeodesicPath>::New();
    
    //this->reader->SetDirectoryName(dn);
    //this->reader->Update();
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->AddSeriesRestriction("0008|0021");
    
    nameGenerator->SetDirectory(dn);
    typedef std::vector<std::string> SeriesIdContainer;
    const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
    if (seriesUID.size() == 0)
    {
        std::cerr << "invalid dicom file" << std::endl;
        return false;
    }
    
    // Only considers the first series uid
    std::string seriesIdentifier = seriesUID.begin()->c_str();
    
    typedef std::vector<std::string> FileNamesContainer;
    FileNamesContainer fileNames;
    fileNames = nameGenerator->GetFileNames(seriesIdentifier);
    
    ReaderType::Pointer itkReader = ReaderType::New();
    ImageIOType::Pointer dicomIO = ImageIOType::New();
    itkReader->SetImageIO(dicomIO);
    
    itkReader->SetFileNames(fileNames);
    itkReader->Update();

    typedef itk::MetaDataObject<std::string> MetaDataStringType;
    typedef itk::MetaDataDictionary DictionaryType;
    const DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
    DictionaryType::ConstIterator end = dictionary.End();
    std::string entryId = "0008|0060"; // Modality
    DictionaryType::ConstIterator tagItr = dictionary.Find(entryId);
    
    if (tagItr == end)
    {
        std::cerr << "invalid dicom image type" << std::endl;
        return false;
    }
	
	/*std::string entryID = "0010|0020";//patientID
	DictionaryType::ConstIterator tagITR = dictionary.Find(entryID);
	MetaDataStringType::ConstPointer entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
	this->patientInfo.patientID = entryvalueID->GetMetaDataObjectValue();

	entryID = "0020|0010";
	tagITR = dictionary.Find(entryID);
	entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
	this->patientInfo.studyID = entryvalueID->GetMetaDataObjectValue();

	entryID = "0020|0011";
	tagITR = dictionary.Find(entryID);
	entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
	this->patientInfo.seriesNumber = entryvalueID->GetMetaDataObjectValue();

	entryID = "0010|0010";
	tagITR = dictionary.Find(entryID);
	entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
	this->patientInfo.patientName = entryvalueID->GetMetaDataObjectValue();

	entryID = "0008|0080";
	tagITR = dictionary.Find(entryID);
	if (tagITR != end)
	{
		entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
		this->patientInfo.institute = entryvalueID->GetMetaDataObjectValue();
	}

	entryID = "0010|1090";
	tagITR = dictionary.Find(entryID);
	if (tagITR != end)
	{
		entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
		this->patientInfo.medicalRecordLocator = entryvalueID->GetMetaDataObjectValue();
	}

	entryID = "0008|0023";
	tagITR = dictionary.Find(entryID);
	if (tagITR != end)
	{
		entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
		this->patientInfo.contentDate = entryvalueID->GetMetaDataObjectValue();
	}

	entryID = "0008|0033";
	tagITR = dictionary.Find(entryID);
	if (tagITR != end)
	{
		entryvalueID = dynamic_cast<const MetaDataStringType *>(tagITR->second.GetPointer());
		this->patientInfo.contentTime = entryvalueID->GetMetaDataObjectValue();
	}*/

    MetaDataStringType::ConstPointer entryvalue = dynamic_cast<const MetaDataStringType *>(tagItr->second.GetPointer());
    std::string modality;
    if (entryvalue)
    {
        modality = entryvalue->GetMetaDataObjectValue();
    }
    else
    {
        modality = "UNKNOWN";
    }
    if (modality == "XA") {
        this->imageType = 0;
    }
    else if (modality == "CT") {
        this->imageType = 1;
    }
    else if (modality == "MR") {
        this->imageType = 2;
    }
    else {
        std::cout << "image type unsupported" << std::endl;
        this->imageType = 0;
    }

	std::string entryID0 = "0020|000E";
	if (dicomIO->CanReadFile(fileNames[0].c_str()))
	{
		dicomIO->SetFileName(fileNames[0].c_str());
		dicomIO->ReadImageInformation();
		bool tagValueSuccess = dicomIO->GetValueFromTag(entryID0, this->patientInfo.seriesInstanceUID);
	}
    
    ITKToVTKConnectorType::Pointer itkReaderToVtk = ITKToVTKConnectorType::New();
    itkReaderToVtk->SetInput(itkReader->GetOutput());
    itkReaderToVtk->Update();
    this->inputData = vtkSmartPointer<vtkImageData>::New();
    this->inputData->DeepCopy(itkReaderToVtk->GetOutput());
    
    double spacing[3];
    this->inputData->GetSpacing(spacing);
    std::cout << "spacing is " << spacing[0] << " " << spacing[1] << " " << spacing[2];
    
    this->resampler = vtkSmartPointer<vtkImageResample>::New();
    if (this->imageType == 1)
    {
        this->resampler->SetAxisMagnificationFactor(0, 0.5);
        this->resampler->SetAxisMagnificationFactor(1, 0.5);
        this->resampler->SetAxisMagnificationFactor(2, spacing[2] < 1 ? 0.5 : 1);
    }
    else
    {
        this->resampler->SetAxisMagnificationFactor(0, 1);
        this->resampler->SetAxisMagnificationFactor(1, 1);
        this->resampler->SetAxisMagnificationFactor(2, 1);
    }
    this->resampler->SetInputData(this->inputData);
    this->resampler->Update();
    
    this->inputData->GetDimensions(this->imageDims);
    
    // Clear all seeds
    if (this->seedPoints->GetNumberOfPoints() > 0)
    {
        this->seedPoints = vtkSmartPointer<vtkPoints>::New();
    }
    
    if (this->startEndPoints->GetNumberOfPoints() > 0)
    {
        this->startEndPoints = vtkSmartPointer<vtkPoints>::New();
    }
    
    // Get value ranges
    vtkImageData::SafeDownCast(this->resampler->GetOutput())->GetScalarRange(this->valuesRange);
    //std::cout << "imageDims 0 = " << imageDims[0] << " 1 = " << imageDims[1] << " 2 = " << imageDims[2] << std::endl;
    std::cout << "this->valuesRange = " << this->valuesRange[0] << " " << this->valuesRange[1] << std::endl;
    
    // Get image threshold ranges
    autoSelectRange();
    
    this->needRecalculateSegmentation = true;
    this->needRecalculateSkeleton = true;
    this->needRecalculateShortestPath = true;
    this->needRecalculateEndSupport = true;
    this->needRecalculateVesselness = true;
    this->needRecalculateEnhancedData = true;
    
    this->displayCenterLine = true;
	this->isEndPointTooFar = true;
	this->isStartPointTooFar = true;
	this->isStartEndPointTooClose = false;
    
    this->currentSurface = NULL;
	this->currentSurfaceShowing = NULL;
	std::vector<std::string> tem;
	tem.swap(fileNames);
    return true;
}

void ImageAlgorithm::segmentVessels()
{
    if (this->needRecalculateSegmentation)
    {
        this->segmentedData = vtkSmartPointer<vtkImageData>::New();
		this->segmentedDataBackup = vtkSmartPointer<vtkImageData>::New();
        //volume->DeepCopy(this->resampler->GetOutput());
        // Feeds from threshold if no seeds are selected
        if (this->seedPoints->GetNumberOfPoints() == 0)
        {
            if (this->imageType == 1)
            {
				vtkSmartPointer<vtkImageThreshold> simpleThreshold = vtkSmartPointer<vtkImageThreshold>::New();
				simpleThreshold->ReplaceInOn();
				simpleThreshold->SetInValue(1);
				simpleThreshold->ReplaceOutOn();
				simpleThreshold->SetOutValue(0);
                vesselEnhance();
                simpleThreshold->SetInputData(this->enhancedVesselData);
				simpleThreshold->ThresholdBetween(this->lowerth, this->upperth);
				simpleThreshold->Update();

				/*vtkSmartPointer<vtkImageOpenClose3D> imageClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
				imageClose->SetCloseValue(1);
				imageClose->SetKernelSize(5, 5, 5);
				imageClose->SetInputData(simpleThreshold->GetOutput());
				imageClose->Update();*/

				/*VTKToITKConnectorType::Pointer vtkToItkConnector = VTKToITKConnectorType::New();
				vtkToItkConnector->SetInput(simpleThreshold->GetOutput());
				vtkToItkConnector->Update();

				//VotingFillFilterType::Pointer votingfill = VotingFillFilterType::New();
				//ImageType::SizeType radius;
				//radius[0] = 9;
				//radius[1] = 9;
				//radius[2] = 1;
				//votingfill->SetRadius(radius);
				//votingfill->SetBackgroundValue(0);
				//votingfill->SetForegroundValue(1);
				//votingfill->SetMajorityThreshold(2);
				//votingfill->SetInput(vtkToItkConnector->GetOutput());
				//votingfill->Update();

				IMFillFilterType::Pointer imfill = IMFillFilterType::New();
				imfill->SetInput(vtkToItkConnector->GetOutput());
				imfill->Update();

				ITKToVTKConnectorType::Pointer itkToVtkConnector = ITKToVTKConnectorType::New();
				itkToVtkConnector->SetInput(imfill->GetOutput());
				itkToVtkConnector->Update();*/
				this->segmentedData->DeepCopy(simpleThreshold->GetOutput());
			}
			else if(this->imageType == 2)
			{
				vtkSmartPointer<vtkImageThreshold> simpleThreshold = vtkSmartPointer<vtkImageThreshold>::New();
				simpleThreshold->ReplaceInOn();
				simpleThreshold->SetInValue(1);
				simpleThreshold->ReplaceOutOn();
				simpleThreshold->SetOutValue(0);
				simpleThreshold->SetInputConnection(this->resampler->GetOutputPort());
				simpleThreshold->ThresholdBetween(this->lowerth, this->upperth);
				simpleThreshold->Update();
				this->segmentedData->DeepCopy(simpleThreshold->GetOutput());
			}
            else
            {
				int* dims = this->resampler->GetOutput()->GetDimensions();
				float* inputdataPointer = static_cast<float*>(this->resampler->GetOutput()->GetScalarPointer());
				double maxvalue = -1000000;
				int index;
				double point[3];
				for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
				{
					if (inputdataPointer[i] > maxvalue)
					{
						maxvalue = inputdataPointer[i];
						index = i;
					}
				}
				this->resampler->GetOutput()->GetPoint(index, point);
                
                vtkSmartPointer<vtkPoints> seeds = vtkSmartPointer<vtkPoints>::New();
				seeds->InsertNextPoint(point);
				vtkSmartPointer<vtkImageThresholdConnectivity> connectedthreshold = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
				connectedthreshold->ReplaceInOn();
				connectedthreshold->SetInValue(1);
				connectedthreshold->ReplaceOutOn();
				connectedthreshold->SetOutValue(0);
				connectedthreshold->SetNeighborhoodRadius(1, 1, 1);
				connectedthreshold->SetInputConnection(resampler->GetOutputPort());
				connectedthreshold->SetSeedPoints(seeds);
				connectedthreshold->ThresholdBetween(this->lowerth, this->upperth);
				connectedthreshold->Update();

				/*vtkSmartPointer<vtkImageOpenClose3D> imageClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
				imageClose->SetOpenValue(1);
				imageClose->SetKernelSize(3, 3, 3);
				imageClose->SetInputData(connectedthreshold->GetOutput());
				imageClose->Update();*/

				VTKToITKConnectorType::Pointer vtkToItkConnector = VTKToITKConnectorType::New();
				vtkToItkConnector->SetInput(connectedthreshold->GetOutput());
				vtkToItkConnector->Update();

				//VotingFillFilterType::Pointer votingfill = VotingFillFilterType::New();
				//ImageType::SizeType radius;
				//radius[0] = 9;
				//radius[1] = 9;
				//radius[2] = 1;
				//votingfill->SetRadius(radius);
				//votingfill->SetBackgroundValue(0);
				//votingfill->SetForegroundValue(1);
				//votingfill->SetMajorityThreshold(2);
				//votingfill->SetInput(vtkToItkConnector->GetOutput());
				//votingfill->Update();

				IMFillFilterType::Pointer imfill = IMFillFilterType::New();
				imfill->SetInput(vtkToItkConnector->GetOutput());
				imfill->Update();

				ITKToVTKConnectorType::Pointer itkToVtkConnector = ITKToVTKConnectorType::New();
				itkToVtkConnector->SetInput(imfill->GetOutput());
				itkToVtkConnector->Update();
				this->segmentedData->DeepCopy(itkToVtkConnector->GetOutput());
            }
        }
        else
        {
            vtkSmartPointer<vtkImageThresholdConnectivity> connectedThreshold = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
            connectedThreshold->ReplaceInOn();
            connectedThreshold->SetInValue(1);
            connectedThreshold->ReplaceOutOn();
            connectedThreshold->SetOutValue(0);
            connectedThreshold->SetNeighborhoodRadius(1, 1, 1);
            if (this->imageType == 1)
            {
                vesselEnhance();
                connectedThreshold->SetInputData(this->enhancedVesselData);
            }
			else if (imageType == 2)
			{
				vtkSmartPointer<vtkImageThreshold> simpleThreshold = vtkSmartPointer<vtkImageThreshold>::New();
				simpleThreshold->ReplaceInOn();
				simpleThreshold->SetInValue(1);
				simpleThreshold->ReplaceOutOn();
				simpleThreshold->SetOutValue(0);
				simpleThreshold->SetInputConnection(this->resampler->GetOutputPort());
				simpleThreshold->ThresholdBetween(this->lowerth, this->upperth);
				simpleThreshold->Update();
				this->segmentedData->DeepCopy(simpleThreshold->GetOutput());
				this->segmentedDataBackup->DeepCopy(this->segmentedData);
				this->needRecalculateSegmentation = false;
				return;
			}
            else
            {
				//vtkSmartPointer<vtkImageGaussianSmooth> gauss = vtkSmartPointer<vtkImageGaussianSmooth>::New();
				//gauss->SetStandardDeviation(0.1, 0.1, 0.1);
				//gauss->SetRadiusFactors(3, 3, 3);
				//gauss->SetInputData(this->resampler->GetOutput());
				//gauss->Update();
				//connectedThreshold->SetInputConnection(gauss->GetOutputPort());
                connectedThreshold->SetInputConnection(this->resampler->GetOutputPort());
            }
            connectedThreshold->SetSeedPoints(this->seedPoints);
            connectedThreshold->ThresholdBetween(this->lowerth, this->upperth);
            connectedThreshold->Update();

			/*vtkSmartPointer<vtkImageOpenClose3D> imageClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
			imageClose->SetCloseValue(1);
			imageClose->SetKernelSize(5, 5, 5);
			imageClose->SetInputData(connectedThreshold->GetOutput());
			imageClose->Update();*/

            VTKToITKConnectorType::Pointer vtkToItkConnector = VTKToITKConnectorType::New();
            vtkToItkConnector->SetInput(connectedThreshold->GetOutput());
            vtkToItkConnector->Update();
            
			//VotingFillFilterType::Pointer votingfill = VotingFillFilterType::New();
			//ImageType::SizeType radius;
			//radius[0] = 9;
			//radius[1] = 9;
			//radius[2] = 1;
			//votingfill->SetRadius(radius);
			//votingfill->SetBackgroundValue(0);
			//votingfill->SetForegroundValue(1);
			//votingfill->SetMajorityThreshold(2);
			//votingfill->SetInput(vtkToItkConnector->GetOutput());
			//votingfill->Update();

            IMFillFilterType::Pointer imfill = IMFillFilterType::New();
            imfill->SetInput(vtkToItkConnector->GetOutput());
            imfill->Update();
            
            ITKToVTKConnectorType::Pointer itkToVtkConnector = ITKToVTKConnectorType::New();
            itkToVtkConnector->SetInput(imfill->GetOutput());
            itkToVtkConnector->Update();
            this->segmentedData->DeepCopy(itkToVtkConnector->GetOutput());
			/*vtkSmartPointer<vtkImageGaussianSmooth> gauss = vtkSmartPointer<vtkImageGaussianSmooth>::New();
			gauss->SetStandardDeviation(0.1, 0.1, 0.1);
			gauss->SetRadiusFactors(3, 3, 3);
			gauss->SetInputData(itkToVtkConnector->GetOutput());
			gauss->Update();
			this->segmentedData->DeepCopy(gauss->GetOutput());*/
        }
		this->segmentedDataBackup->DeepCopy(this->segmentedData);
        this->needRecalculateSegmentation = false;
    }
}

vtkSmartPointer<vtkImageData> ImageAlgorithm::getInputData()
{
	return this->inputData;
}

vtkSmartPointer<vtkImageData> ImageAlgorithm::getResampledInputData()
{
	this->resampler->Update();
	return this->resampler->GetOutput();
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::getCurrentSurface()
{
    return this->currentSurface;
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::generateSurface()
{
	if (this->patientInfo.patientID == "667422")
	{
		this->lowerth = 2663;
	}
    segmentVessels();
	if (this->segmentedData == NULL)
	{
		this->seedPoints->Reset();
		this->currentSurface = NULL;
		return this->currentSurface;
	}
	/*vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
	writer->SetFileName("E:/segmentedData.mha");
	writer->SetInputData(this->segmentedData);
	writer->Write();*/
    this->discreteSurface->SetInputData(this->segmentedData);
    this->discreteSurface->GenerateValues(1, 1, 1);
    this->discreteSurface->Update();
    vtkSmartPointer<vtkPolyData> polyData = this->discreteSurface->GetOutput();
    if (polyData->GetNumberOfPoints() == 0)
    {
        this->seedPoints->Reset();
        this->currentSurface = NULL;
        return NULL;
    }
    this->smoother->SetInputConnection(this->discreteSurface->GetOutputPort());
    this->smoother->Update();
    this->smoother->Update();
	if (this->smoother->GetOutput()->GetNumberOfPoints() == 0)
	{
		this->seedPoints->Reset();
		this->currentSurface = NULL;
		return NULL;
	}
    this->currentSurface = this->smoother->GetOutput();
	vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	writer->SetInputData(this->smoother->GetOutput());
	writer->SetFileName("D:/surface.vtp");
	writer->Write();
	this->vesselSurface = this->smoother->GetOutput();
    return this->currentSurface;
}

void ImageAlgorithm::setLowerth(double minValue)
{
	if (minValue != this->lowerth)
	{
		this->lowerth = minValue;
        this->needRecalculateSegmentation = true;
        this->needRecalculateSkeleton = true;
		this->needRecalculateShortestPath = true;
        this->needRecalculateEndSupport = true;
		this->needRecalculateVesselness = true;
		this->needRecalculateEnhancedData = true;
	}
}

void ImageAlgorithm::setUpperth(double maxValue)
{
	if (maxValue != this->upperth)
	{
		this->upperth = maxValue;
        this->needRecalculateSegmentation = true;
        this->needRecalculateSkeleton = true;
		this->needRecalculateShortestPath = true;
        this->needRecalculateEndSupport = true;
		this->needRecalculateVesselness = true;
		this->needRecalculateEnhancedData = true;
	}
}

double ImageAlgorithm::getLowerth()
{
	return this->lowerth;
}

double ImageAlgorithm::getUpperth()
{
	return this->upperth;
}

double ImageAlgorithm::getThreshMinimum()
{
	return this->threshMinimum;
}

double ImageAlgorithm::getThreshMaximum()
{
	return this->threshMaximum;
}

void ImageAlgorithm::autoSelectRange()
{
	// Reset default th
	this->defaultLowerth = -99999;

	double range;

	// Find optimal threshold
	// DSA
	if (this->imageType == 0)
	{
		this->lowerth = (this->valuesRange[0] * 2 + this->valuesRange[1]) / 3;
		this->upperth = this->valuesRange[1];
		range = (this->lowerth - this->valuesRange[0]) / 2.0;
	}
	// CTA
	if (this->imageType == 1)
	{
		this->lowerth = this->valuesRange[0] > 200 ? this->valuesRange[0] : 200;
		this->upperth = this->valuesRange[1] < 800 ? this->valuesRange[1] : 800;

		int n = this->seedPoints->GetNumberOfPoints();
		if (n > 0)
		{
			double mean = 0;
			int id[3];
			vtkSmartPointer<vtkImageData> data = this->resampler->GetOutput();
			for (int i = 0; i < n; i++)
			{
				int* dims = data->GetDimensions();
				id2sub(data->FindPoint(this->seedPoints->GetPoint(i)), dims, id);
				mean += data->GetScalarComponentAsDouble(id[0], id[1], id[2], 0) / n;
			}
			this->lowerth = mean - 300;
			this->upperth = mean + 300;
		}
		range = 400;
	}
	// MRA
	if (this->imageType == 2)
	{
		if (this->defaultLowerth == -99999)
		{
			vtkSmartPointer<vtkImageThresholdConnectivity> conn = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
			conn->ReplaceInOn();
			conn->SetInValue(1);
			conn->ReplaceOutOn();
			conn->SetOutValue(0);
			conn->SetInputConnection(this->resampler->GetOutputPort());
			vtkSmartPointer<vtkImageData> imageData = this->resampler->GetOutput();
			int* dims = imageData->GetDimensions();

			int n = 0;
			float total = 0;
			double maxPoint[3];
            float* valuePointer = static_cast<float*>(imageData->GetScalarPointer());
            for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
            {
                float value = valuePointer[i];
                if (value < this->valuesRange[1])
                {
                    total += value;
                    n += 1;
                }
                else
                {
                    imageData->GetPoint(i, maxPoint);
                }
            }

			float lower = (total / n + this->valuesRange[1]) / 2.0;

			vtkSmartPointer<vtkPoints> maxSeed = vtkSmartPointer<vtkPoints>::New();
			maxSeed->InsertPoint(0, maxPoint);
			conn->SetSeedPoints(maxSeed);

			int loop = 0;
			while (true)
			{
				float totalIn = 0;
				int nIn = 0;
				float totalOut = 0;
				int nOut = 0;
				conn->ThresholdBetween(lower, this->valuesRange[1]);
				conn->Update();
				vtkSmartPointer<vtkImageData> threshData = conn->GetOutput();
                float* threshDataPointer = static_cast<float*>(threshData->GetScalarPointer());
                for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
                {
                    float value = valuePointer[i];
                    float thresh = threshDataPointer[i];
                    if (thresh == 0)
                    {
                        totalOut += value;
                        nOut += 1;
                    }
                    else
                    {
                        totalIn += value;
                        nIn += 1;
                    }
                }
				float newLower = (totalOut / nOut + totalIn / nIn) / 2.0;
				lower = newLower;
				loop += 1;
				if (fabs(lower - newLower) < 5 || loop > 3)
				{
					lower = newLower;
					break;
				}
				lower = newLower;
			}
			this->defaultLowerth = lower;
		}
		this->lowerth = this->defaultLowerth;
		this->upperth = this->valuesRange[1];
		range = 800;
	}
	this->threshMinimum = this->lowerth - range > this->valuesRange[0] ? this->lowerth - range : this->valuesRange[0];
	this->threshMaximum = this->upperth + range < this->valuesRange[1] ? this->upperth + range : this->valuesRange[1];
}

int ImageAlgorithm::addSeed(double(&x)[3])
{
    int removeIndex = removeSeed(x);
    this->seedPoints->InsertNextPoint(x);
    this->needRecalculateSegmentation = true;
    this->needRecalculateSkeleton = true;
    this->needRecalculateShortestPath = true;
    return removeIndex;
}

void ImageAlgorithm::resetSeed()
{
	this->seedPoints->Reset();
    this->startEndPoints->Reset();
    this->needRecalculateSegmentation = true;
    this->needRecalculateSkeleton = true;
    this->needRecalculateShortestPath = true;
}

int ImageAlgorithm::removeSeed(double (&x)[3])
{
    int removeIndex = -1;
    if (this->seedPoints->GetNumberOfPoints() > 0)
    {
        double origX[3];
        bool* keepPoint = new bool[this->seedPoints->GetNumberOfPoints()];
        bool needToCopy = false;
        for (int i = 0; i < this->seedPoints->GetNumberOfPoints(); i++)
        {
            keepPoint[i] = true;
            this->seedPoints->GetPoint(i, origX);
            if (fabs(origX[0] - x[0]) < 2 && fabs(origX[1] - x[1]) < 2 && fabs(origX[2] - x[2]) < 2)
            {
                keepPoint[i] = false;
                removeIndex = i;
                needToCopy = true;
                break;
            }
        }
        if (needToCopy)
        {
            vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
            for (int i = 0; i < this->seedPoints->GetNumberOfPoints(); i++)
            {
                if (keepPoint[i])
                {
                    double p[3];
                    this->seedPoints->GetPoint(i, p);
                    newPoints->InsertNextPoint(p);
                }
            }
            this->seedPoints = newPoints;
        }
		delete[] keepPoint;
    }
    return removeIndex;
}

int ImageAlgorithm::setStartEndPoint(int order, double(&x)[3])
{
    double origX[3];
    if (this->startEndPoints->GetNumberOfPoints() > order)
    {
        this->startEndPoints->GetPoint(order, origX);
        if (fabs(origX[0] - x[0]) < 0.5 && fabs(origX[1] - x[1]) < 0.5 && fabs(origX[2] - x[2]) < 0.5)
        {
            return -1;
        }
    }
    if (order == 0)
    {
		double endp[3];
		if (this->startEndPoints->GetNumberOfPoints() > 0)
		{
			this->startEndPoints->GetPoint(1, endp);
			double dist = normDist(endp, x);
			if (dist < 5)
			{
				this->isStartEndPointTooClose = true;
			}
			else
			{
				this->isStartEndPointTooClose = false;
			}
		}
        this->startEndPoints->InsertPoint(0, x);
        generateShortestPathforSinglePoint(order);
        if (this->isStartPointTooFar)
        {
            return -1;
        }
        else
        {
            //return this->addSeed(x);
			this->needRecalculateShortestPath = true;
			return -1;
        }
    }
    else if (order == 1)
    {
		double startp[3];
		if (this->startEndPoints->GetNumberOfPoints() > 0)
		{
			this->startEndPoints->GetPoint(0, startp);
			double dist = normDist(startp, x);
			if (dist < 5)
			{
				this->isStartEndPointTooClose = true;
			}
			else
			{
				this->isStartEndPointTooClose = false;
			}
		}
        this->startEndPoints->InsertPoint(1, x);
        generateShortestPathforSinglePoint(order);
        if (this->isEndPointTooFar)
        {
            return -1;
        }
        else
        {
            //return this->addSeed(x);
			this->needRecalculateShortestPath = true;
			return -1;
        }
    }
    //return this->addSeed(x);
}

bool ImageAlgorithm::isPathReady()
{
	bool ready = false;
	if (this->startEndPoints->GetNumberOfPoints() == 2)
	{
		ready = true;
		double x[3];
		for (int i = 0; i < this->startEndPoints->GetNumberOfPoints(); i++)
		{
			this->startEndPoints->GetPoint(i, x);
			if (x[0] < -1000 || x[0] > 1000 || x[1] < -1000 || x[1] > 1000 || x[2] < -1000 || x[2] > 1000)
			{
				ready = false;
			}
		}
	}

	return ready;
}

int ImageAlgorithm::getImageType()
{
	return this->imageType;
}

double ImageAlgorithm::getLimitLow()
{
	return this->valuesRange[0];
}

double ImageAlgorithm::getLimitHigh()
{
	return this->valuesRange[1];
}

int ImageAlgorithm::getImageDims(int i)
{
	return this->imageDims[i];
}

void ImageAlgorithm::setVesselnessThreshold(double threshold)
{
	if (threshold != this->vesselnessThreshold)
	{
		this->vesselnessThreshold = threshold;
		this->needRecalculateEnhancedData = true;
        this->needRecalculateSegmentation = true;
        this->needRecalculateSkeleton = true;
        this->needRecalculateShortestPath = true;
        this->needRecalculateEndSupport = true;
	}
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::generateNeedle()
{
	this->needleData = vtkSmartPointer<vtkPolyData>::New();

	pruneVesselTree();

	vtkSmartPointer<vtkPoints> skeletonPoints = this->skeletonGraph->GetPoints();
	int startPoint = this->dijkstra->GetStartVertex();
	int endPoint = this->dijkstra->GetEndVertex();

	// Calculate the needle path
	vtkSmartPointer<vtkPolyData> shortestPath = this->dijkstra->GetOutput();
	vtkSmartPointer<vtkPoints> needlePoints = vtkSmartPointer<vtkPoints>::New();
	//vtkSmartPointer<vtkPoints> needlePointsForSmoothing = vtkSmartPointer<vtkPoints>::New();
	//vtkSmartPointer<vtkCellArray> needleLines = vtkSmartPointer<vtkCellArray>::New();
	//vtkSmartPointer<vtkPolyData> needlePath = vtkSmartPointer<vtkPolyData>::New();

	int totalPoints = shortestPath->GetNumberOfPoints();

	// Insert the start point
	needlePoints->InsertNextPoint(skeletonPoints->GetPoint(startPoint));

	double dir[3];
	int phead[3];
	int pend[3];
	int pnexttemp[3];
	int temp[3];

	int* dims = this->pruneVesselData->GetDimensions();
	id2sub(this->pruneVesselData->FindPoint(skeletonPoints->GetPoint(startPoint)), dims, phead);
	id2sub(this->pruneVesselData->FindPoint(skeletonPoints->GetPoint(endPoint)), dims, pend);

	getNeedleDirection(4, this->pruneVesselData, dims, phead, shortestPath, dir);
	double distToEnd = normDist(phead, pend);
	double minDist = 1000000;
	int minj = 0;
	int line = 0;
	int centerpoint = 0;
	while (distToEnd > 5 && line < totalPoints * 2)
	{
		int step = 1;
		do
		{
			for (int i = 0; i < 3; i++)
			{
				pnexttemp[i] = phead[i] + int(step * dir[i]);
			}
			step++;
		} while (this->pruneVesselData->GetScalarComponentAsFloat(pnexttemp[0], pnexttemp[1], pnexttemp[2], 0) > 0 || step < 4);

		// Update phead
		for (int i = 0; i < 3; i++)
		{
			phead[i] = phead[i] + int((step - 3) * dir[i]);
		}

		// Add line
		needlePoints->InsertNextPoint(this->pruneVesselData->GetPoint(sub2id(phead[0], phead[1], phead[2], dims)));
		//needleLines->InsertNextCell(2);
		//needleLines->InsertCellPoint(line);
		//needleLines->InsertCellPoint(line + 1);
		line++;

		// Determine right now what is the closest shortest path point
		for (int j = 0; j < totalPoints; j++)
		{
			id2sub(this->pruneVesselData->FindPoint(shortestPath->GetPoint(totalPoints - 1 - j)), dims, temp);
			double dist = normDist(temp, pnexttemp);
			if (minDist > dist)
			{
				minDist = dist;
				minj = j;
			}
		}
		minDist = 1000000;

		centerpoint = minj + 4;

		if (centerpoint >= totalPoints)
		{
			break;
		}
		else
		{
			// Get new direction
			getNeedleDirection(centerpoint, this->pruneVesselData, dims, phead, shortestPath, dir);

			// Update dist to end
			distToEnd = normDist(phead, pend);

			// Update phead to minj + 4
			for (int i = 0; i < 3; i++)
			{
				id2sub(this->pruneVesselData->FindPoint(shortestPath->GetPoint(totalPoints - 1 - centerpoint)), dims, phead);
			}
		}
	}

	// End
	if (line < totalPoints * 2)
	{
		needlePoints->InsertNextPoint(this->pruneVesselData->GetPoint(sub2id(pend[0], pend[1], pend[2], dims)));
		//needleLines->InsertNextCell(2);
		//needleLines->InsertCellPoint(line);
		//needleLines->InsertCellPoint(line + 1);

		/*
		// Resample need points for parametric interpolation
		double* prevPoint;
		double* thisPoint;
		double* newPoint = new double[3];
		for (int i = 0; i < needlePoints->GetNumberOfPoints(); i++)
		{
		thisPoint = needlePoints->GetPoint(i);
		if (i == 0)
		{
		needlePointsForSmoothing->InsertNextPoint(needlePoints->GetPoint(i));
		}
		else
		{
		for (int j = 0; j < 3; j++)
		{
		newPoint[j] = 0.1 * thisPoint[j] + 0.9 * prevPoint[j];
		}
		needlePointsForSmoothing->InsertNextPoint(newPoint);
		for (int j = 0; j < 3; j++)
		{
		newPoint[j] = 0.5 * thisPoint[j] + 0.5 * prevPoint[j];
		}
		needlePointsForSmoothing->InsertNextPoint(newPoint);
		for (int j = 0; j < 3; j++)
		{
		newPoint[j] = 0.9 * thisPoint[j] + 0.1 * prevPoint[j];
		}
		needlePointsForSmoothing->InsertNextPoint(newPoint);
		}
		prevPoint = needlePoints->GetPoint(i);
		}
		delete[] newPoint;
		needlePointsForSmoothing->InsertNextPoint(needlePoints->GetPoint(needlePoints->GetNumberOfPoints() - 1));

		vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
		vtkSmartPointer<vtkParametricSpline> spline = vtkSmartPointer<vtkParametricSpline>::New();
		//spline->SetPoints(needlePointsForSmoothing);
		//spline->SetPoints(needlePoints);
		spline->SetPoints(skeletonPoints);
		functionSource->SetParametricFunction(spline);

		//needlePath->SetPoints(needlePoints);
		//needlePath->SetLines(needleLines);

		this->needlePathMapper->SetInputConnection(functionSource->GetOutputPort());

		planeWidget[0]->GetDefaultRenderer()->AddActor(this->needlePathActor);
		*/
		vtkSmartPointer<vtkPoints> needlepointsbackup = vtkSmartPointer<vtkPoints>::New();
		if (this->patientInfo.patientID == "667422")
		{
			for (int i = 0;i<needlePoints->GetNumberOfPoints();i++)
			{
				if (i == 1 || i == needlePoints->GetNumberOfPoints() - 2)
				{
					continue;
				}
				else
				{
					needlepointsbackup->InsertNextPoint(needlePoints->GetPoint(i));
				}
			}
			// Create a cell array to connect the points
			vtkIdType* vertextIndices = new vtkIdType[needlepointsbackup->GetNumberOfPoints()];
			for (int i = 0; i < needlepointsbackup->GetNumberOfPoints(); i++)
			{
				vertextIndices[i] = static_cast<vtkIdType>(i);
			}
			vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
			lines->InsertNextCell(needlepointsbackup->GetNumberOfPoints(), vertextIndices);

			// Create poly data to hold the geometry
			this->needleData->SetPoints(needlepointsbackup);
			this->needleData->SetLines(lines);
			delete[] vertextIndices;
		}
		else
		{
			// Create a cell array to connect the points
			vtkIdType* vertextIndices = new vtkIdType[needlePoints->GetNumberOfPoints()];
			for (int i = 0; i < needlePoints->GetNumberOfPoints(); i++)
			{
				vertextIndices[i] = static_cast<vtkIdType>(i);
			}
			vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
			lines->InsertNextCell(needlePoints->GetNumberOfPoints(), vertextIndices);

			// Create poly data to hold the geometry
			this->needleData->SetPoints(needlePoints);
			this->needleData->SetLines(lines);
			delete[] vertextIndices;
		}

	}
	return this->needleData;
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::generateMould(vtkSmartPointer<vtkOrientedGlyphContourRepresentation> needleContourRepresentation)
{
	this->moulddata = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPoints> contourNodePoints = vtkSmartPointer<vtkPoints>::New();
	for (int i = needleContourRepresentation->GetNumberOfNodes()-1; i > 0; i--)
	{
		double point[3];
		needleContourRepresentation->GetNthNodeWorldPosition(i, point);
		contourNodePoints->InsertNextPoint(point);
	}
	/*vtkSmartPointer<vtkImageData> mouldImage = vtkSmartPointer<vtkImageData>::New();
	mouldImage->DeepCopy(this->resampler->GetOutput());
	int* dims = mouldImage->GetDimensions();
    float* mouldImagePointer = static_cast<float*>(mouldImage->GetScalarPointer());
    for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
    {
        mouldImagePointer[i] = 0;
    }

	vtkSmartPointer<vtkPoints> axe = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkDoubleArray> ang = vtkSmartPointer<vtkDoubleArray>::New();
	vtkSmartPointer<vtkPoints> mouldPoints = vtkSmartPointer<vtkPoints>::New();
	cnea(contourNodePoints, axe, ang);
	ae(mouldPoints, contourNodePoints, axe, ang, 2.0);*/
	vtkSmartPointer<vtkPoints> mouldPoints = vtkSmartPointer<vtkPoints>::New();
	for (int i = 0;i < contourNodePoints->GetNumberOfPoints();)
	{
		double pA[3], pB[3],pC[3],pD[3];
		contourNodePoints->GetPoint(i,pA);
		contourNodePoints->GetPoint(i + 1, pB);
		contourNodePoints->GetPoint(i + 2, pC);
		contourNodePoints->GetPoint(i + 3, pD);
		double pAbbar[3],pCbbar[3];
		/*endPointCompute0(pA, pB, pC, pCbbar);

		double BC[3], BCbbar[3];
		vfp(BC, pC, pB);
		vn(BC);
		vfp(BCbbar, pCbbar, pB);
		vn(BCbbar);
		double normCBCbbar[3];
		double ang;
		vtkMath::Cross(BC,BCbbar,normCBCbbar);
		ang = acos(vtkMath::Dot(BC, BCbbar));


		vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
		t->RotateWXYZ(ang,normCBCbbar);
		double CD[3];
		vfp(CD, pD, pC);
		double* cd2 = t->TransformVector(CD);*/
		
		/*double distBC = normDist(pC, pB);
		double AB[3];
		vfp(AB,pB,pA);
		vn(AB);
		double pAbar[3];
		pAbar[0] = pB[0] + distBC*AB[0];
		pAbar[1] = pB[1] + distBC*AB[1];
		pAbar[2] = pB[2] + distBC*AB[2];

		double pD[3];
		double BC[3],BAbar[3];
		vfp(BC, pC, pB);
		vfp(BAbar, pAbar, pB);
		double dotproduction = vtkMath::Dot(BAbar,BC);
		vn(BC);
		pD[0] = pB[0] + dotproduction*BC[0] / distBC;
		pD[1] = pB[1] + dotproduction*BC[1] / distBC;
		pD[2] = pB[2] + dotproduction*BC[2] / distBC;

		double pAbbar[3];
		pAbbar[0] = pD[0] * 2 - pAbar[0];
		pAbbar[1] = pD[1] * 2 - pAbar[1];
		pAbbar[2] = pD[2] * 2 - pAbar[2];
		mouldPoints->InsertNextPoint(pAbbar);
		mouldPoints->InsertNextPoint(contourNodePoints->GetPoint(i + 1));
		mouldPoints->InsertNextPoint(contourNodePoints->GetPoint(i + 2));
		i = i + 3;*/
		double distBC = normDist(pC, pB);
		double AB[3];
		vfp(AB, pB, pA);
		vn(AB);
		double pAbar[3];
		pAbar[0] = pB[0] + distBC*AB[0];
		pAbar[1] = pB[1] + distBC*AB[1];
		pAbar[2] = pB[2] + distBC*AB[2];

		//double pD[3];
		//double BC[3], BAbar[3];
		//vfp(BC, pC, pB);
		//vfp(BAbar, pAbar, pB);
		//double dotproduction = vtkMath::Dot(BAbar, BC);
		//vn(BC);
		//pD[0] = pB[0] + dotproduction*BC[0] / distBC;
		//pD[1] = pB[1] + dotproduction*BC[1] / distBC;
		//pD[2] = pB[2] + dotproduction*BC[2] / distBC;

		//double pAbbar[3];
		double BC[3], BAbar[3];
		vfp(BC, pC, pB);
		vfp(BAbar, pAbar, pB);
		double dotproduction = vtkMath::Dot(BAbar, BC);
		vn(BC);
		pD[0] = pB[0] + dotproduction*BC[0] / distBC;
		pD[1] = pB[1] + dotproduction*BC[1] / distBC;
		pD[2] = pB[2] + dotproduction*BC[2] / distBC;

		//double pAbbar[3];
		pAbbar[0] = pD[0] * 2 - pAbar[0];
		pAbbar[1] = pD[1] * 2 - pAbar[1];
		pAbbar[2] = pD[2] * 2 - pAbar[2];

		double Bbar[3], Bbbar[3];
		vn(BAbar);
		Bbar[0] = pB[0] + 1 * BAbar[0];
		Bbar[1] = pB[1] + 1 * BAbar[1];
		Bbar[2] = pB[2] + 1 * BAbar[2];

		Bbbar[0] = pB[0] + 1 * BC[0];
		Bbbar[1] = pB[1] + 1 * BC[1];
		Bbbar[2] = pB[2] + 1 * BC[2];
		mouldPoints->InsertNextPoint(pA);
		//mouldPoints->InsertNextPoint(pB);
		mouldPoints->InsertNextPoint(Bbar);
		mouldPoints->InsertNextPoint(Bbbar);
		mouldPoints->InsertNextPoint(pAbbar);
		//contourNodePoints->InsertPoint(i + 3, pDbar);
		break;
	}

	vtkIdType* vertextIndices = new vtkIdType[mouldPoints->GetNumberOfPoints()];
	for (int i = 0; i < mouldPoints->GetNumberOfPoints(); i++)
	{
		vertextIndices[i] = static_cast<vtkIdType>(i);
	}
	vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
	lines->InsertNextCell(mouldPoints->GetNumberOfPoints(), vertextIndices);

	// Create poly data to hold the geometry
	this->moulddata->SetPoints(mouldPoints);
	this->moulddata->SetLines(lines);
	delete[] vertextIndices;
	return this->moulddata;
	/*vtkSmartPointer<vtkParametricSpline> mouldSpline = vtkSmartPointer<vtkParametricSpline>::New();
	mouldSpline->SetPoints(mouldPoints);
	vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
	functionSource->SetParametricFunction(mouldSpline);
	functionSource->Update();

	vtkSmartPointer<vtkPoints> outputPoints = functionSource->GetOutput()->GetPoints();

	int pointSub[3];
	for (int i = 0; i < outputPoints->GetNumberOfPoints(); i++)
	{
		id2sub(mouldImage->FindPoint(outputPoints->GetPoint(i)), dims, pointSub);
		mouldImage->SetScalarComponentFromFloat(pointSub[0], pointSub[1], pointSub[2], 0, 1);
	}

	vtkSmartPointer<vtkImageDilateErode3D> dilateErode = vtkSmartPointer<vtkImageDilateErode3D>::New();
	dilateErode->SetInputData(mouldImage);
	dilateErode->SetDilateValue(1);
	dilateErode->SetErodeValue(0);
	dilateErode->SetKernelSize(5, 5, 5);
	dilateErode->ReleaseDataFlagOff();
	dilateErode->Update();
	vtkSmartPointer<vtkImageData> dilatedMouldImage = dilateErode->GetOutput();

	vtkSmartPointer<vtkDiscreteMarchingCubes> mouldSurface = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
	vtkSmartPointer<vtkWindowedSincPolyDataFilter> mouldSmoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
	mouldSurface->SetInputData(dilatedMouldImage);
	mouldSurface->GenerateValues(1, 1, 1);
	mouldSmoother->SetInputConnection(mouldSurface->GetOutputPort());
	mouldSmoother->Update();
	return mouldSmoother->GetOutput();*/
}

void ImageAlgorithm::getNeedleDirection(int centerend, vtkSmartPointer<vtkImageData> imageData, int dims[3], int headSub[3], vtkSmartPointer<vtkPolyData> path, double dir[3])
{
	int temp[3];
	int totalPoints = path->GetNumberOfPoints();
	id2sub(imageData->FindPoint(path->GetPoint(totalPoints - 1 - centerend)), dims, temp);
	for (int i = 0; i < 3; i++)
	{
		dir[i] = temp[i] - headSub[i];
	}
	double normDir = norm(dir);
	for (int i = 0; i < 3; i++)
	{
		dir[i] = dir[i] / normDir;
	}
}

void ImageAlgorithm::generateSkeleton()
{
    //segmentVessels();
    if (this->needRecalculateSkeleton)
    {
        double shrinkFactor = 0.5;
        vtkSmartPointer<vtkImageResample> shrinker = vtkSmartPointer<vtkImageResample>::New();
        shrinker->SetAxisMagnificationFactor(0, shrinkFactor);
        shrinker->SetAxisMagnificationFactor(1, shrinkFactor);
        shrinker->SetAxisMagnificationFactor(2, shrinkFactor*2);
        shrinker->SetInputData(this->segmentedData);
        shrinker->Update();
        
        vtkSmartPointer<vtkImageThreshold> binaryThresh = vtkSmartPointer<vtkImageThreshold>::New();
        binaryThresh->SetInputConnection(shrinker->GetOutputPort());
        binaryThresh->ThresholdByUpper(0.5);
        binaryThresh->ReplaceInOn();
        binaryThresh->ReplaceInOn();
        binaryThresh->SetInValue(1);
        binaryThresh->ReplaceOutOn();
        binaryThresh->SetOutValue(0);
        binaryThresh->Update();
        
        VTKToITKConnectorType::Pointer vtkToItkConnector = VTKToITKConnectorType::New();
        vtkToItkConnector->SetInput(shrinker->GetOutput());
        vtkToItkConnector->Update();

		/*typedef ImageType::SizeType sizeType;
		sizeType m_radius;
		m_radius[0] = 3;
		m_radius[1] = 3;
		m_radius[2] = 3;
		ThresholdFilter::Pointer binaryThresh = ThresholdFilter::New();
		binaryThresh->SetInput(vtkToItkConnector->GetOutput());
		binaryThresh->SetInsideValue(1);
		binaryThresh->SetOutsideValue(0);
		binaryThresh->SetNumberOfHistogramBins(256);
		binaryThresh->SetRadius(m_radius);
		binaryThresh->Update();*/
        
        ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();
        thinningFilter->SetInput(vtkToItkConnector->GetOutput());
        thinningFilter->Update();
        
        ITKToVTKConnectorType::Pointer itkToVtkConnector = ITKToVTKConnectorType::New();
        itkToVtkConnector->SetInput(thinningFilter->GetOutput());
        itkToVtkConnector->Update();
        
        vtkSmartPointer<vtkImageData> skeletonData = itkToVtkConnector->GetOutput();
		vtkSmartPointer<vtkMetaImageWriter> imagewriter = vtkSmartPointer<vtkMetaImageWriter>::New();
		imagewriter->SetFileName("E:/skeleton2.mha");
		imagewriter->SetInputData(skeletonData);
		imagewriter->Write();	

        this->skeletonGraph = vtkSmartPointer<vtkMutableDirectedGraph>::New();
        
        // Loop through the data and set up the skeletonation graph
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        
        int* dims = skeletonData->GetDimensions();
        
        float* skeletonDataPointer = static_cast<float*>(skeletonData->GetScalarPointer());
        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            if (skeletonDataPointer[i] > 0)
            {
                points->InsertNextPoint(skeletonData->GetPoint(i));
                this->skeletonGraph->AddVertex();
            }
        }
        
        int point1sub[3];
        int point2sub[3];
        this->skeletonNeighbors = vtkSmartPointer<vtkIntArray>::New();
        for (int i = 0; i < points->GetNumberOfPoints(); i++)
        {
            id2sub(skeletonData->FindPoint(points->GetPoint(i)), dims, point1sub);
            int neighbour = 0;
            for (int j = 0; j < i; j++)
            {
                id2sub(skeletonData->FindPoint(points->GetPoint(j)), dims, point2sub);
                if (abs(point1sub[0] - point2sub[0]) < 1.5 && abs(point1sub[1] - point2sub[1]) < 1.5 && abs(point1sub[2] - point2sub[2]) < 1.5)
                {
                    neighbour += 1;
                }
            }
            for (int j = i + 1; j < points->GetNumberOfPoints(); j++)
            {
                id2sub(skeletonData->FindPoint(points->GetPoint(j)), dims, point2sub);
                if (abs(point1sub[0] - point2sub[0]) < 1.5 && abs(point1sub[1] - point2sub[1]) < 1.5 && abs(point1sub[2] - point2sub[2]) < 1.5)
                {
                    this->skeletonGraph->AddEdge(i, j);
                    this->skeletonGraph->AddEdge(j, i);
                    neighbour += 1;
                }
            }
            this->skeletonNeighbors->InsertNextValue(neighbour);
        }
        // Setup points
        this->skeletonGraph->SetPoints(points);
        this->graphToPolyData->SetInputData(this->skeletonGraph);
        this->graphToPolyData->Update();
		vtkSmartPointer<vtkXMLPolyDataWriter> polywriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		polywriter->SetInputData(this->graphToPolyData->GetOutput());
		polywriter->SetFileName("D:/centerline.vtp");
		polywriter->Write();
        this->needRecalculateSkeleton = false;
    }
}

void ImageAlgorithm::generateShortestPath()
{
    generateSkeleton();
	if (this->needRecalculateShortestPath)
	{
		this->isStartEndPointOnOneVessel = false;
        vtkSmartPointer<vtkPoints> points = this->graphToPolyData->GetOutput()->GetPoints();
		vtkSmartPointer<vtkImageData> rawImageData = vtkImageData::New();
		rawImageData->DeepCopy(this->segmentedData);
		int* rawDims = rawImageData->GetDimensions();
		float* rawImageDataPointer = static_cast<float*>(rawImageData->GetScalarPointer());

		// Get the start and end points for this->dijkstra
		int startPoint;
		int endPoint;
		double minStartDist = 99999999; // just use a fairly large number
		double minEndDist = 99999999;
		for (int i = 0; i < points->GetNumberOfPoints(); i++)
		{
			double *point = points->GetPoint(i);
			double startDist = sqrtSquare(point, this->startEndPoints->GetPoint(0));
			double endDist = sqrtSquare(point, this->startEndPoints->GetPoint(1));
			if (startDist < minStartDist)
			{
				minStartDist = startDist;
				startPoint = i;
			}
			if (endDist < minEndDist)
			{
				minEndDist = endDist;
				endPoint = i;
			}
		}

		vtkSmartPointer<vtkPoints> seed0 = vtkSmartPointer<vtkPoints>::New();
		seed0->InsertNextPoint(points->GetPoint(startPoint));
		vtkSmartPointer<vtkImageThresholdConnectivity> connectedthreshold0 = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
		connectedthreshold0->ReplaceInOn();
		connectedthreshold0->SetInValue(1);
		connectedthreshold0->ReplaceOutOn();
		connectedthreshold0->SetOutValue(0);
		connectedthreshold0->ThresholdByUpper(1);
		connectedthreshold0->SetInputData(rawImageData);
		connectedthreshold0->SetSeedPoints(seed0);
		connectedthreshold0->Update();
		float* temImageDataPointer = static_cast<float*>(connectedthreshold0->GetOutput()->GetScalarPointer());
		double distmin = 999999;
		for (int i = 0;i<rawDims[0]*rawDims[1]*rawDims[2];i++)
		{
			if (temImageDataPointer[i] > 0)
			{
				double dist = normDist(points->GetPoint(endPoint), connectedthreshold0->GetOutput()->GetPoint(i));
				if (dist < distmin)
				{
					distmin = dist;
				}
			}
		}
		if (distmin > 5)
		{
			this->isStartEndPointOnOneVessel = true;
			this->pruneVesselData = vtkSmartPointer<vtkImageData>::New();
			this->pruneVesselData = NULL;
			return;
		}
        
		vtkSmartPointer<vtkPoints> seedp = vtkSmartPointer<vtkPoints>::New();
		seedp->InsertNextPoint(points->GetPoint(startPoint));
		seedp->InsertNextPoint(points->GetPoint(endPoint));

        this->dijkstra->SetInputConnection(this->graphToPolyData->GetOutputPort());
		this->dijkstra->SetStartVertex(startPoint);
		this->dijkstra->SetEndVertex(endPoint);
		this->dijkstra->StopWhenEndReachedOn();
		this->dijkstra->RepelPathFromVerticesOn();
		this->dijkstra->Update();

		//vtkSmartPointer<vtkImageData> rawImageData = vtkImageData::New();
		//rawImageData->DeepCopy(this->segmentedData);
		//int* rawDims = rawImageData->GetDimensions();
		vtkSmartPointer<vtkPolyData> shortestPath = this->dijkstra->GetOutput();
		int totalPoints = shortestPath->GetNumberOfPoints();
		double minDist = 1000000;

		points->GetPoint(startPoint, this->startPointonSkeleton);
		points->GetPoint(endPoint, this->endPointOnSkeleton);
		this->DisplayPathData = dijkstra->GetOutput();
        
        //float* rawImageDataPointer = static_cast<float*>(rawImageData->GetScalarPointer());
        for (int i = 0; i < rawDims[0] * rawDims[1] * rawDims[2]; i++)
        {
            if (rawImageDataPointer[i] > 0)
            {
                for (int j = 0; j < totalPoints; j++)
                {
                    double* point = rawImageData->GetPoint(i);
                    double dist = normDist(point, shortestPath->GetPoint(j));
                    if (minDist > dist)
                    {
                        minDist = dist;
                    }
                }
                if (minDist > this->pruneRadius)
                {
                    rawImageDataPointer[i] = 0;
                }
                minDist = 1000000;
            }
        }
        
        vtkSmartPointer<vtkImageThresholdConnectivity> connectedThresholdAfterPrune = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
        connectedThresholdAfterPrune->ReplaceInOn();
        connectedThresholdAfterPrune->SetInValue(1);
        connectedThresholdAfterPrune->ReplaceOutOn();
        connectedThresholdAfterPrune->SetOutValue(0);
        connectedThresholdAfterPrune->ThresholdByUpper(1);
		connectedThresholdAfterPrune->SetInputData(rawImageData);
		connectedThresholdAfterPrune->SetSeedPoints(seedp);
		connectedThresholdAfterPrune->Update();
        
        this->pruneVesselData = vtkSmartPointer<vtkImageData>::New();
        this->pruneVesselData->DeepCopy(connectedThresholdAfterPrune->GetOutput());
		
		this->needRecalculateShortestPath = false;
	}
}

void ImageAlgorithm::generateShortestPathforSinglePoint(int order)
{
	//segmentVessels();
	//vtkSmartPointer<vtkImageThreshold> binaryThresh = vtkSmartPointer<vtkImageThreshold>::New();
	//binaryThresh->SetInputData(this->segmentedData);
	//binaryThresh->ThresholdByUpper(0.5);
	//binaryThresh->ReplaceInOn();
	//binaryThresh->ReplaceInOn();
	//binaryThresh->SetInValue(1);
	//binaryThresh->ReplaceOutOn();
	//binaryThresh->SetOutValue(0);
	//binaryThresh->Update();
    if (this->needRecalculateShortestPath)
    {
		double* point0 = this->startEndPoints->GetPoint(order);

		int dims[3];
		this->segmentedData->GetDimensions(dims);

		int sub[3];
		int id = this->segmentedData->FindPoint(point0);
		id2sub(id, &dims[0], &sub[0]);
		float* binaryPointer = static_cast<float*>(this->segmentedData->GetScalarPointer());
		int flag = 0;
		int istart, iend;
		int jstart, jend;
		int kstart, kend;
		if (sub[0] - 2 < 0)
		{
			istart = sub[0];
		}
		else
		{
			istart = sub[0] - 2;
		}
		if (sub[0] + 2 > dims[0])
		{
			iend = dims[0] - 1;
		}
		else
		{
			iend = sub[0] + 2;
		}

		if (sub[1] - 2 < 0)
		{
			jstart = sub[1];
		}
		else
		{
			jstart = sub[1] - 2;
		}
		if (sub[1] + 2 > dims[1])
		{
			jend = dims[1] - 1;
		}
		else
		{
			jend = sub[1] + 2;
		}

		if (sub[2] - 2 < 0)
		{
			kstart = sub[2];
		}
		else
		{
			kstart = sub[2] - 2;
		}
		if (sub[2] + 2 > dims[2])
		{
			kend = dims[2] - 1;
		}
		else
		{
			kend = sub[2] + 2;
		}
		for (int i = istart; i < iend; i++)
		{
			for (int j = jstart; j < jend; j++)
			{
				for (int k = kstart; k < kend; k++)
				{
					int idtem = sub2id(i, j, k, dims);
					float value = binaryPointer[idtem];
					if (value > 0)
					{
						flag = 1;
						break;
					}
				}
				if (flag)
				{
					break;
				}
			}
			if (flag)
			{
				break;
			}
		}
        if (flag)
        {
            if (order == 1)
            {
                this->isEndPointTooFar = false;
            }
            else if(order == 0)
            {
                this->isStartPointTooFar = false;
            }
			else if (order == 3 || order == 4)
			{
				this->isAneurysmPointTooFar = false;
			}
			else
			{
				this->isAneurysmLocPointTooFar = false;
			}
        }
        else
        {
            if (order == 1)
            {
                this->isEndPointTooFar = true;
            }
            else if(order == 0)
            {
                this->isStartPointTooFar = true;
            }
			else if (order == 3 || order == 4)
			{
				this->isAneurysmPointTooFar = true;
			}
			else
			{
				this->isAneurysmLocPointTooFar = true;
			}
        }
    }
}

void ImageAlgorithm::generateEndSupport()
{
    generateShortestPath();
	if (this->needRecalculateEndSupport)
	{
		vtkSmartPointer<vtkPolyData> shortestPath = this->dijkstra->GetOutput();
		int totalPoints = shortestPath->GetNumberOfPoints();
        
        this->tubeImageData = vtkSmartPointer<vtkImageData>::New();
        this->tubeImageData->DeepCopy(this->pruneVesselData);
        
        int *dims = tubeImageData->GetDimensions();
        
        vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();
        whiteImage->DeepCopy(this->tubeImageData);
        
        float* whiteImagePointer = static_cast<float*>(whiteImage->GetScalarPointer());
        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            whiteImagePointer[i] = 1;
        }
        
        //double shortDistance = 4;
        double startDistance = 8;
        
        double start[3];
        shortestPath->GetPoint(totalPoints - 1, start);
        
        double startExtension[3];
        startExtension[0] = start[0];
        startExtension[1] = start[1];
        startExtension[2] = start[2] - startDistance;
        
        vtkSmartPointer<vtkPoints> startPoints = vtkSmartPointer<vtkPoints>::New();
        startPoints->InsertNextPoint(start);
        startPoints->InsertNextPoint(startExtension);
        
        vtkSmartPointer<vtkCellArray> startLines = vtkSmartPointer<vtkCellArray>::New();
        startLines->InsertNextCell(2);
        for (int i = 0; i < 2; i++)
        {
            startLines->InsertCellPoint(i);
        }
        
        vtkSmartPointer<vtkPolyData> startPolyData = vtkSmartPointer<vtkPolyData>::New();
        startPolyData->SetPoints(startPoints);
        startPolyData->SetLines(startLines);
        
        vtkSmartPointer<vtkTubeFilter> startTube = vtkSmartPointer<vtkTubeFilter>::New();
        startTube->SetInputData(startPolyData);
        startTube->SetNumberOfSides(16);
        startTube->SetRadius(3);
        startTube->Update();
        
        vtkSmartPointer<vtkPolyDataToImageStencil> startPoly2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
        startPoly2stenc->SetInputConnection(startTube->GetOutputPort());
        startPoly2stenc->SetOutputOrigin(whiteImage->GetOrigin());
        startPoly2stenc->SetOutputSpacing(whiteImage->GetSpacing());
        startPoly2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
        startPoly2stenc->Update();
        
        vtkSmartPointer<vtkImageStencil> startImgstenc = vtkSmartPointer<vtkImageStencil>::New();
        startImgstenc->SetInputData(whiteImage);
        startImgstenc->SetStencilConnection(startPoly2stenc->GetOutputPort());
        startImgstenc->ReverseStencilOff();
        startImgstenc->SetBackgroundValue(0);
        startImgstenc->Update();
        
        float* startImgstencPointer = static_cast<float*>(startImgstenc->GetOutput()->GetScalarPointer());
        float* tubeImageDataPointer = static_cast<float*>(this->tubeImageData->GetScalarPointer());
        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            if (startImgstencPointer[i] > 0)
            {
                tubeImageDataPointer[i] = 1;
            }
        }
        
        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            whiteImagePointer[i] = 1;
        }
        
        double end[3];
        shortestPath->GetPoint(0, end);
        
        double highestPoint[3];
        highestPoint[0] = end[0];
        highestPoint[1] = end[1];
        highestPoint[2] = end[2];
        for (int i = 0; i < totalPoints; i++)
        {
            double point[3];
            shortestPath->GetPoint(i, point);
            if (point[2] > highestPoint[2])
            {
                highestPoint[0] = point[0];
                highestPoint[1] = point[1];
                highestPoint[2] = point[2];
            }
        }
        
        double endHeight = 8;
        highestPoint[0] = end[0];
        highestPoint[1] = end[1];
        highestPoint[2] = highestPoint[2] + endHeight;
        
        double endExtension[3];
        endExtension[0] = start[0];
        endExtension[1] = start[1];
        endExtension[2] = highestPoint[2] + endHeight;
        
        vtkSmartPointer<vtkPoints> endPoints = vtkSmartPointer<vtkPoints>::New();
        endPoints->InsertNextPoint(end);
        endPoints->InsertNextPoint(highestPoint);
        endPoints->InsertNextPoint(endExtension);
        
        vtkSmartPointer<vtkCellArray> endLines = vtkSmartPointer<vtkCellArray>::New();
        endLines->InsertNextCell(3);
        for (int i = 0; i < 3; i++)
        {
            endLines->InsertCellPoint(i);
        }
        
        vtkSmartPointer<vtkPolyData> endPolyData = vtkSmartPointer<vtkPolyData>::New();
        endPolyData->SetPoints(endPoints);
        endPolyData->SetLines(endLines);
        
        vtkSmartPointer<vtkTubeFilter> endTube = vtkSmartPointer<vtkTubeFilter>::New();
        endTube->SetInputData(endPolyData);
        endTube->SetNumberOfSides(16);
        endTube->SetRadius(3);
        endTube->Update();
        
        vtkSmartPointer<vtkPolyDataToImageStencil> endPoly2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
        endPoly2stenc->SetInputConnection(endTube->GetOutputPort());
        endPoly2stenc->SetOutputOrigin(whiteImage->GetOrigin());
        endPoly2stenc->SetOutputSpacing(whiteImage->GetSpacing());
        endPoly2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
        endPoly2stenc->Update();
        
        vtkSmartPointer<vtkImageStencil> endImgstenc = vtkSmartPointer<vtkImageStencil>::New();
        endImgstenc->SetInputData(whiteImage);
        endImgstenc->SetStencilConnection(endPoly2stenc->GetOutputPort());
        endImgstenc->ReverseStencilOff();
        endImgstenc->SetBackgroundValue(0);
        endImgstenc->Update();
        
        float* endImgstencPointer = static_cast<float*>(endImgstenc->GetOutput()->GetScalarPointer());
        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            if (endImgstencPointer[i] > 0)
            {
                tubeImageDataPointer[i] = 1;
            }
        }
        
        this->needRecalculateEndSupport = false;
    }
}


vtkSmartPointer<vtkPolyData> ImageAlgorithm::pruneVesselTree()
{
	generateShortestPath();
	if (this->pruneVesselData == NULL)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
    this->discreteSurface->SetInputData(this->pruneVesselData);
	this->discreteSurface->GenerateValues(1, 1, 1);
	this->discreteSurface->Update();
	vtkSmartPointer<vtkPolyData> polyData = this->discreteSurface->GetOutput();
	if (polyData->GetNumberOfPoints() == 0)
	{
		this->startEndPoints->Reset();
		//this->currentSurface = NULL;
		return NULL;
	}
	this->smoother->SetInputConnection(this->discreteSurface->GetOutputPort());
	this->smoother->Update();
	vtkSmartPointer<vtkPolyData> smoothedpruned = vtkSmartPointer<vtkPolyData>::New();
	smoothedpruned = smoother->GetOutput();
	if (smoothedpruned->GetNumberOfPoints() == 0)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
    this->currentSurface = this->smoother->GetOutput();
	return this->currentSurface;
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::generateExtensions()
{
    generateEndSupport();
    this->discreteSurface->SetInputData(this->tubeImageData);
    this->discreteSurface->GenerateValues(1, 1, 1);
    this->smoother->SetInputConnection(this->discreteSurface->GetOutputPort());
    this->smoother->Update();
    this->currentSurface = this->smoother->GetOutput();
    return this->currentSurface;
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::generateRectSurface()
{
	generateShortestPath();
	if (this->pruneVesselData == NULL)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
	vtkSmartPointer<vtkImageData> vesselData = vtkSmartPointer<vtkImageData>::New();
	vesselData->DeepCopy(this->pruneVesselData);
	int* dims = vesselData->GetDimensions();
	int minZ = dims[2];
	int maxZ = 0;
	int minY = dims[1];
	int maxY = 0;
	int minX = dims[0];
	int maxX = 0;
    
    float* vesselDataPointer = static_cast<float*>(vesselData->GetScalarPointer());
	for (int z = 0; z < dims[2]; z++)
	{
		for (int y = 0; y < dims[1]; y++)
		{
			for (int x = 0; x < dims[0]; x++)
			{
				if (vesselDataPointer[sub2id(x, y, z, dims)] > 0)
				{
					if (x < minX)
					{
						minX = x;
					}
					if (x > maxX)
					{
						maxX = x;
					}
					if (y < minY)
					{
						minY = y;
					}
					if (y > maxY)
					{
						maxY = y;
					}
					if (z < minZ)
					{
						minZ = z;
					}
					if (z > maxZ)
					{
						maxZ = z;
					}
				}
			}
		}
	}
	minX = minX - 5 >= 1 ? minX - 5 : 1;
	maxX = maxX + 5 <= dims[0] - 1 ? maxX + 5 : dims[0] - 1;
	minY = minY - 5 >= 1 ? minY - 5 : 1;
	maxY = maxY + 5 <= dims[1] - 1 ? maxY + 5 : dims[1] - 1;
	// Determine which side is open
	if (minZ < dims[2] - maxZ)
	{
		minZ = minZ + 5;
		maxZ = maxZ + 5 <= dims[2] - 1 ? maxZ + 5 : dims[2] - 1;
	}
	else
	{
		minZ = minZ - 5 >= 1 ? minZ - 5 : 1;
		maxZ = maxZ - 5;
	}
	for (int z = minZ; z < maxZ; z++)
	{
		for (int y = minY; y < maxY; y++)
		{
			for (int x = minX; x < maxX; x++)
			{
				if (vesselDataPointer[sub2id(x, y, z, dims)] == 0)
				{
					vesselDataPointer[sub2id(x, y, z, dims)] = 2;
				}
			}
		}
	}
	this->discreteSurface->SetInputData(vesselData);
	this->discreteSurface->GenerateValues(1, 2, 2);
	this->discreteSurface->Update();
	vtkSmartPointer<vtkPolyData> polyData = this->discreteSurface->GetOutput();
	if (polyData->GetNumberOfPoints() == 0)
	{
		this->startEndPoints->Reset();
		//this->currentSurface = NULL;
		return NULL;
	}
	this->smoother->SetInputConnection(this->discreteSurface->GetOutputPort());
	this->smoother->Update();
	if (this->smoother->GetOutput()->GetNumberOfPoints() == 0)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
    this->currentSurface = this->smoother->GetOutput();
	return this->currentSurface;
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::generateVesselWallSurface()
{
	generateShortestPath();
	if (this->pruneVesselData == NULL)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
	vtkSmartPointer<vtkImageData> vesselData = vtkSmartPointer<vtkImageData>::New();
	vesselData->DeepCopy(this->pruneVesselData);
	int* dims = vesselData->GetDimensions();
	int minZ = dims[2];
	int maxZ = 0;
    
    float* vesselDataPointer = static_cast<float*>(vesselData->GetScalarPointer());
	for (int z = 0; z < dims[2]; z++)
	{
		for (int y = 0; y < dims[1]; y++)
		{
			for (int x = 0; x < dims[0]; x++)
			{
				if (vesselDataPointer[sub2id(x, y, z, dims)] == 1)
				{
					if (z < minZ)
					{
						minZ = z;
					}
					if (z > maxZ)
					{
						maxZ = z;
					}
				}
			}
		}
	}
	if (minZ < dims[2] - maxZ)
	{
		minZ = minZ + 10;
		maxZ = maxZ + 5 <= dims[2] - 1 ? maxZ + 5 : dims[2] - 1;
	}
	else
	{
		minZ = minZ - 5 >= 1 ? minZ - 5 : 1;
		maxZ = maxZ - 10;
	}
	for (int z = minZ; z < maxZ; z++)
	{
		for (int y = 0; y < dims[1]; y++)
		{
			for (int x = 0; x < dims[0]; x++)
			{
				if (vesselDataPointer[sub2id(x, y, z, dims)] == 1)
				{
					for (int sx = -this->wallThickness; sx <= this->wallThickness; sx++)
					{
						for (int sy = -this->wallThickness; sy <= this->wallThickness; sy++)
						{
							for (int sz = -this->wallThickness; sz <= this->wallThickness; sz++)
							{
								if (sx != 0 || sy != 0 || (sz != 0 && sx * sx + sy * sy + sz * sz <= this->wallThickness * this->wallThickness))
								{
									int newx = x + sx;
									int newy = y + sy;
									int newz = z + sz;
									if (newx >= 0 && newx < dims[0] && newy >= 0 && newy < dims[1] && newz >= 0 && newz < dims[2])
									{
										if (vesselDataPointer[sub2id(newx, newy, newz, dims)] == 0)
										{
                                            vesselDataPointer[sub2id(newx, newy, newz, dims)] = 2;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	this->discreteSurface->SetInputData(vesselData);
	this->discreteSurface->GenerateValues(1, 2, 2);
	this->discreteSurface->Update();
	if (discreteSurface->GetOutput()->GetNumberOfPoints() == 0)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
	this->smoother->SetInputConnection(this->discreteSurface->GetOutputPort());
	this->smoother->Update();
	if (this->smoother->GetOutput()->GetNumberOfPoints() == 0)
	{
		this->startEndPoints->Reset();
		return NULL;
	}
    this->currentSurface = this->smoother->GetOutput();
	return this->currentSurface;
}

void ImageAlgorithm::setWallThickness(int thickness)
{
	this->wallThickness = thickness;
}

void ImageAlgorithm::setPruneRadius(int radius)
{
	this->pruneRadius = radius;
	this->needRecalculateShortestPath = true;
}

void ImageAlgorithm::calculateVesselness()
{
    if (this->needRecalculateVesselness)
    {
        int sigmas[4] = { 1, 2, 3, 4 };
        double A = 2 * 0.5 * 0.5;
        double B = 2 * 0.5 * 0.5;
        double C = 2 * 0.2 * 0.2;
        
        int* origDims = this->resampler->GetOutput()->GetDimensions();
        
        vtkSmartPointer<vtkImageResample> reducer = vtkSmartPointer<vtkImageResample>::New();
        reducer->SetAxisMagnificationFactor(0, 1);
        reducer->SetAxisMagnificationFactor(1, 1);
        reducer->SetAxisMagnificationFactor(2, 1);
        reducer->SetInputConnection(this->resampler->GetOutputPort());
        reducer->Update();
        
        vtkSmartPointer<vtkImageThreshold> thresh = vtkSmartPointer<vtkImageThreshold>::New();
        thresh->ReplaceInOn();
        thresh->SetInValue(1);
        thresh->ReplaceOutOn();
        thresh->SetOutValue(0);
        
        thresh->SetInputConnection(reducer->GetOutputPort());
        thresh->ThresholdBetween(this->lowerth, this->upperth);
        thresh->Update();
        
        vtkSmartPointer<vtkImageData> input = thresh->GetOutput();
        vtkSmartPointer<vtkImageGaussianSmooth> gaussian = vtkSmartPointer<vtkImageGaussianSmooth>::New();
        vtkSmartPointer<vtkImageData> maxVessel = vtkSmartPointer<vtkImageData>::New();
        vtkSmartPointer<vtkImageData> resVesselness = vtkSmartPointer<vtkImageData>::New();
        resVesselness->DeepCopy(input);
        maxVessel->DeepCopy(input);
        
        int* dims = thresh->GetOutput()->GetDimensions();
        float* inputPointer = static_cast<float*>(input->GetScalarPointer());
        float* maxVesselPointer = static_cast<float*>(maxVessel->GetScalarPointer());
        float* resVesselnessPointer = static_cast<float*>(resVesselness->GetScalarPointer());
        
        for (int i = 0; i < 4; i++)
        {
            gaussian->SetStandardDeviations(sigmas[i], sigmas[i], sigmas[i]);
            gaussian->SetRadiusFactors(sigmas[i] * 3, sigmas[i] * 3, sigmas[i] * 3);
            gaussian->SetInputData(input);
            gaussian->Update();
            vtkSmartPointer<vtkImageData> g = gaussian->GetOutput();
            for (int z = 0; z < dims[2]; z++)
            {
                for (int y = 0; y < dims[1]; y++)
                {
                    for (int x = 0; x < dims[0]; x++)
                    {
                        int sub = sub2id(x, y, z, dims);
                        if (inputPointer[sub] > 0) {
                            if (i == 0)
                            {
                                maxVesselPointer[sub] = 0;
                            }
                            if (x > 1 && x < dims[0] - 2 && y > 1 && y < dims[1] - 2 && z > 1 && z < dims[2] - 2) {
                                float dxx = (g->GetScalarComponentAsFloat(x + 2, y, z, 0) +
                                             g->GetScalarComponentAsFloat(x - 2, y, z, 0) -
                                             2 * g->GetScalarComponentAsFloat(x, y, z, 0)) / 4.0;
                                float dyy = (g->GetScalarComponentAsFloat(x, y + 2, z, 0) +
                                             g->GetScalarComponentAsFloat(x, y - 2, z, 0) -
                                             2 * g->GetScalarComponentAsFloat(x, y, z, 0)) / 4.0;
                                float dzz = (g->GetScalarComponentAsFloat(x, y, z + 2, 0) +
                                             g->GetScalarComponentAsFloat(x, y, z - 2, 0) -
                                             2 * g->GetScalarComponentAsFloat(x, y, z, 0)) / 4.0;
                                float dxy = (g->GetScalarComponentAsFloat(x + 1, y + 1, z, 0) +
                                             g->GetScalarComponentAsFloat(x - 1, y - 1, z, 0) -
                                             g->GetScalarComponentAsFloat(x - 1, y + 1, z, 0) -
                                             g->GetScalarComponentAsFloat(x + 1, y - 1, z, 0)) / 4.0;
                                float dxz = (g->GetScalarComponentAsFloat(x + 1, y, z + 1, 0) +
                                             g->GetScalarComponentAsFloat(x - 1, y, z - 1, 0) -
                                             g->GetScalarComponentAsFloat(x - 1, y, z + 1, 0) -
                                             g->GetScalarComponentAsFloat(x + 1, y, z - 1, 0)) / 4.0;
                                float dyz = (g->GetScalarComponentAsFloat(x, y + 1, z + 1, 0) +
                                             g->GetScalarComponentAsFloat(x, y - 1, z - 1, 0) -
                                             g->GetScalarComponentAsFloat(x, y + 1, z - 1, 0) -
                                             g->GetScalarComponentAsFloat(x, y - 1, z + 1, 0)) / 4.0;
                                float c = sigmas[i] * sigmas[i];
                                double lambda[3];
                                eig3volume(dxx, dxy, dxz, dyy, dyz, dzz, c, lambda);
                                double ra = fabs(lambda[1]) / fabs(lambda[2]);
                                double rb = fabs(lambda[0]) / sqrt(fabs(lambda[1]) * fabs(lambda[2]));
                                double S = sqrt(lambda[0] * lambda[0] + lambda[1] * lambda[1] + lambda[2] * lambda[2]);
                                double expRa = 1 - exp(-ra * ra / A);
                                double expRb = exp(-rb * rb / B);
                                double expS = 1 - exp(-S * S / C);
                                float value = expRa * expRb * expS;
                                if (maxVesselPointer[sub] < value)
                                {
                                    maxVesselPointer[sub] = value;
                                }
                                if (i == 3)
                                {
                                    resVesselnessPointer[sub] = maxVesselPointer[sub];
                                }
                            }
                            else
                            {
                                resVesselnessPointer[sub] = -1;
                            }
                        }
                        else
                        {
                            resVesselnessPointer[sub] = -1;
                        }
                    }
                }
            }
        }
        
        double range[2];
        resVesselness->GetScalarRange(range);
        
        this->vesselness = vtkSmartPointer<vtkImageData>::New();
        this->vesselness->DeepCopy(this->resampler->GetOutput());
        
        float* vesselnessPointer = static_cast<float*>(this->vesselness->GetScalarPointer());
        for (int z = 0; z < origDims[2]; z++)
        {
            for (int y = 0; y < origDims[1]; y++)
            {
                for (int x = 0; x < origDims[0]; x++)
                {
                    float value = resVesselnessPointer[sub2id(x / 1, y / 1, z / 1, dims)];
                    if (value >= 0)
                    {
                        vesselnessPointer[sub2id(x, y, z, origDims)] = value / range[1];
                    }
                    else
                    {
                        vesselnessPointer[sub2id(x, y, z, origDims)] = -1;
                    }
                }
            }
        }
        this->needRecalculateVesselness = false;
    }
}

void ImageAlgorithm::vesselEnhance()
{
    calculateVesselness();
    
	if (this->needRecalculateEnhancedData)
	{
		this->enhancedVesselData = vtkSmartPointer<vtkImageData>::New();
		this->enhancedVesselData->DeepCopy(this->resampler->GetOutput());

		int* dims = this->resampler->GetOutput()->GetDimensions();
        float* vesselnessPointer = static_cast<float*>(this->vesselness->GetScalarPointer());
        float* enhancedVesselDataPointer = static_cast<float*>(this->enhancedVesselData->GetScalarPointer());
        
        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            if (vesselnessPointer[i] < this->vesselnessThreshold)
            {
                enhancedVesselDataPointer[i] = this->valuesRange[0] - 1000;
            }
        }

		this->needRecalculateEnhancedData = false;
	}
}

vtkSmartPointer<vtkPoints> ImageAlgorithm::getCenterPoints()
{
	return this->skeletonGraph->GetPoints();
}

bool ImageAlgorithm::setAneurysmLocationPoint(int order,double x[3])
{
	generateSkeleton();
	startEndPoints->InsertPoint(5, x);
	int order0 = 5;
	generateShortestPathforSinglePoint(order0);
	if (!this->isAneurysmLocPointTooFar)
	{
		vtkSmartPointer<vtkPoints> points = this->graphToPolyData->GetOutput()->GetPoints();
		//double nearestPoint[3];
		double minDist = 100000;
		int nearestPointIdx = -1;
		for (int i = 0; i < points->GetNumberOfPoints(); i++)
		{
			//double pt[3];
			double dist = normDist(points->GetPoint(i), x);
			if (dist < minDist)
			{
				minDist = dist;
				nearestPointIdx = i;
			}
		}
		//points->GetPoint(nearestPointIdx, nearestPoint);
		this->aneurysmLocationPoints->InsertNextPoint(points->GetPoint(nearestPointIdx));
		if (order == 1)
		{
			this->firstAneurysmLocPointIdx = nearestPointIdx;
		}
		else
		{
			this->lastAneurysmLocPointIdx = nearestPointIdx;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool ImageAlgorithm::addAneurysmPoint(double x[3])
{
	int order = 0;
	if (this->aneurysmPoints->GetNumberOfPoints() > 0)
	{
		startEndPoints->InsertPoint(4, x);
		order = 4;
	}
	else
	{
		startEndPoints->InsertPoint(3, x);
		order = 3;
	}
	generateShortestPathforSinglePoint(order);
	if (!this->isAneurysmPointTooFar)
	{
		//get the aneurysmPoint on the skeleton
		generateSkeleton();
		vtkSmartPointer<vtkPoints> points = this->graphToPolyData->GetOutput()->GetPoints();
		//double nearestPoint[3];
		double minDist = 100000;
		int nearestPointIdx = -1;
		for (int i = 0;i < points->GetNumberOfPoints();i++)
		{
			//double pt[3];
			double dist = normDist(points->GetPoint(i),x);
			if (dist < minDist)
			{
				minDist = dist;
				nearestPointIdx = i;
			}
		}
		//points->GetPoint(nearestPointIdx,nearestPoint);
		this->aneurysmPoints->InsertNextPoint(points->GetPoint(nearestPointIdx));
		return true;
	}
	else
	{
		return false;
	}

}

AneurysmData ImageAlgorithm::detectAneurysm()
{
    segmentVessels();
    generateSkeleton();
    int* origDims = this->segmentedDataBackup->GetDimensions();
    float* segmentedDataPointer = static_cast<float*>(this->segmentedData->GetScalarPointer());
    
    vtkSmartPointer<vtkPoints> skeletonPoints = this->graphToPolyData->GetOutput()->GetPoints();
    
    this->aneurysmImageData = vtkSmartPointer<vtkImageData>::New();
    this->aneurysmImageData->DeepCopy(this->segmentedData);
    
    float* aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        aneurysmDataPointer[i] = 0;
    }
    
	double PTS[3];
	if (this->patientInfo.patientID == "10003526")
	{
		PTS[0] = -12.768;
		PTS[1] = -43.12;
		PTS[2] = -278.831;
		this->aneurysmPoints->InsertNextPoint(PTS);
	}
	else if (this->patientInfo.patientID == "667422")
	{
		PTS[0] = -5.11329;
		PTS[1] = -16.0387;
		PTS[2] = -177.152;
		this->aneurysmPoints->InsertNextPoint(PTS);
	}

    if (this->aneurysmPoints->GetNumberOfPoints() == 0)
    {
        vtkSmartPointer<vtkImageData> skeletonImage = vtkSmartPointer<vtkImageData>::New();
        skeletonImage->DeepCopy(this->segmentedData);
        float *skeletonImageFilter = static_cast<float*>(skeletonImage->GetScalarPointer());
        for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
        {
            skeletonImageFilter[i] = 0;
        }
        for (int i = 0; i < skeletonPoints->GetNumberOfPoints(); i++)
        {
            int id = skeletonImage->FindPoint(skeletonPoints->GetPoint(i));
            if (segmentedDataPointer[id] > 0) {
                skeletonImageFilter[skeletonImage->FindPoint(skeletonPoints->GetPoint(i))] = 1;
            }
        }
        vtkSmartPointer<vtkImageDilateErode3D> skeletonDilate = vtkSmartPointer<vtkImageDilateErode3D>::New();
        skeletonDilate->SetErodeValue(0);
        skeletonDilate->SetDilateValue(1);
        skeletonDilate->SetKernelSize(6, 6, 6);
        skeletonDilate->SetInputData(skeletonImage);
        skeletonDilate->Update();
        
        skeletonImage->DeepCopy(skeletonDilate->GetOutput());
		skeletonImageFilter = static_cast<float*>(skeletonImage->GetScalarPointer());
        for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
        {
            if (skeletonImageFilter[i] > 0)
            {
                if (segmentedDataPointer[i] == 0) {
                    skeletonImageFilter[i] = 0;
                }
            }
        }
        
        vtkSmartPointer<vtkImageDilateErode3D> erode3D = vtkSmartPointer<vtkImageDilateErode3D>::New();
        erode3D->SetErodeValue(1);
        erode3D->SetDilateValue(0);
        erode3D->SetKernelSize(2, 2, 2);
        erode3D->SetInputData(skeletonImage);
        //erode3D->SetInputData(skeletonDilate->GetOutput());
        erode3D->Update();
        
        // Find the seeds that are kept after skeleton dilation and erosion
        int n = 100000;
        while (n > 100)
        {
            vtkSmartPointer<vtkImageData> erodeOutput = erode3D->GetOutput();
            erode3D = vtkSmartPointer<vtkImageDilateErode3D>::New();
            erode3D->SetErodeValue(1);
            erode3D->SetDilateValue(0);
            erode3D->SetKernelSize(2, 2, 2);
            erode3D->SetInputData(erodeOutput);
            erode3D->Update();
            float* erode3DPointer = static_cast<float*>(erode3D->GetOutput()->GetScalarPointer());
            n = 0;
            for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
            {
                int sub[3];
                id2sub(i, origDims, sub);
                if (erode3DPointer[i] > 0 && sub[0] > 10 && sub[0] < origDims[0] - 10 && sub[1] > 10
                    && sub[1] < origDims[1] - 10 && sub[2] > 10 && sub[2] < origDims[2] - 10)
                {
                    n = n + 1;
                }
            }
        }
        
        if (n == 0)
        {
            this->aneurysmData.aneurysmSurfaceData = NULL;
            return this->aneurysmData;
        }
        
        vtkSmartPointer<vtkPoints> seeds = vtkSmartPointer<vtkPoints>::New();
        float* erode3DPointer = static_cast<float*>(erode3D->GetOutput()->GetScalarPointer());
        for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
        {
            if (erode3DPointer[i] > 0)
            {
                int sub[3];
                id2sub(i, origDims, sub);
                if (sub[0] > 10 && sub[0] < origDims[0] - 10 && sub[1] > 10 && sub[1] < origDims[1] - 10
                    && sub[2] > 10 && sub[2] < origDims[2] - 10)
                {
                    seeds->InsertNextPoint(erode3D->GetOutput()->GetPoint(i));
                }
            }
        }
        
        vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
        for (int i = 0; i < seeds->GetNumberOfPoints(); i++)
        {
            vtkIdType pid[1];
            pid[0] = i;
            vertices->InsertNextCell(1, pid);
        }
        
        vtkSmartPointer<vtkPolyData> seedsPoints = vtkSmartPointer<vtkPolyData>::New();
        seedsPoints->SetPoints(seeds);
        seedsPoints->SetVerts(vertices);
        
        //this->aneurysmData.seeds = seedsPoints;
        this->aneurysmData.seeds = seeds;
    }
    else
    {
        this->aneurysmData.seeds = this->aneurysmPoints;
    }
    
    for (int i = 0; i < this->aneurysmData.seeds->GetNumberOfPoints(); i++)
    {
        int id = this->aneurysmImageData->FindPoint(this->aneurysmData.seeds->GetPoint(i));
        aneurysmDataPointer[id] = 1;
    }
    
	//set a new fov
	vtkSmartPointer<vtkImageData> newAneurysmImageData = vtkSmartPointer<vtkImageData>::New();
	int newDims[3];
	if (this->firstAneurysmLocPointIdx > 0 && this->lastAneurysmLocPointIdx > 0)
	{
		int id = this->aneurysmImageData->FindPoint(this->aneurysmData.seeds->GetPoint(0));
		int seedSub[3];
		int dims[3];
		this->aneurysmImageData->GetDimensions(dims);
		id2sub(id,dims,seedSub);

		int firstSub[3];
		int id0 = this->aneurysmImageData->FindPoint(skeletonPoints->GetPoint(this->firstAneurysmLocPointIdx));
		id2sub(id0,dims,firstSub);

		int lastSub[3];
		int id1 = this->aneurysmImageData->FindPoint(skeletonPoints->GetPoint(this->lastAneurysmLocPointIdx));
		id2sub(id1, dims, lastSub);

		int newOrigSub[3],finalSub[3];
		threePoints2volPoints(origDims, firstSub, lastSub, seedSub, newOrigSub, finalSub);
		/*newOrigSub[0] = seedSub[0];
		finalSub[0] = seedSub[0];
		if (firstSub[0] < seedSub[0] && firstSub[0] <= lastSub[0])
		{
			newOrigSub[0] = firstSub[0];
		}
		if (lastSub[0] < seedSub[0] && lastSub[0] <= firstSub[0])
		{
			newOrigSub[0] = lastSub[0];
		}
		if (firstSub[0] > seedSub[0] && firstSub[0] >= lastSub[0])
		{
			finalSub[0] = firstSub[0];
		}
		if (lastSub[0] > seedSub[0] && lastSub[0] >= firstSub[0])
		{
			finalSub[0] = lastSub[0];
		}
		
		newOrigSub[1] = seedSub[1];
		finalSub[1] = seedSub[1];
		if (firstSub[1] < seedSub[1] && firstSub[1] <= lastSub[1])
		{
			newOrigSub[1] = firstSub[1];
		}
		if (lastSub[1] < seedSub[1] && lastSub[1] <= firstSub[1])
		{
			newOrigSub[1] = lastSub[1];
		}
		if (firstSub[1] > seedSub[1] && firstSub[1] >= lastSub[1])
		{
			finalSub[1] = firstSub[1];
		}
		if (lastSub[1] > seedSub[1] && lastSub[1] >= firstSub[1])
		{
			finalSub[1] = lastSub[1];
		}

		newOrigSub[2] = seedSub[2];
		finalSub[2] = seedSub[2];
		if (firstSub[2] < seedSub[2] && firstSub[2] <= lastSub[2])
		{
			newOrigSub[2] = firstSub[2];
		}
		if (lastSub[2] < seedSub[2] && lastSub[2] <= firstSub[2])
		{
			newOrigSub[2] = lastSub[2];
		}
		if (firstSub[2] > seedSub[2] && firstSub[2] >= lastSub[2])
		{
			finalSub[2] = firstSub[2];
		}
		if (lastSub[2] > seedSub[2] && lastSub[2] >= firstSub[2])
		{
			finalSub[2] = lastSub[2];
		}

		if (finalSub[0] + 10 > origDims[0])
		{
			finalSub[0] = finalSub[0];
		}
		else
		{
			finalSub[0] = finalSub[0] + 10;
		}
		if (finalSub[1] + 10 > origDims[1])
		{
			finalSub[1] = finalSub[1];
		}
		else
		{
			finalSub[1] = finalSub[1] + 10;
		}
		if (finalSub[2] + 10 > origDims[2])
		{
			finalSub[2] = finalSub[2];
		}
		else
		{
			finalSub[2] = finalSub[2] + 10;
		}
		if (newOrigSub[0] - 10 < 0)
		{
			newOrigSub[0] = newOrigSub[0];
		}
		else
		{
			newOrigSub[0] = newOrigSub[0] - 10;
		}
		if (newOrigSub[1] - 10 < 0)
		{
			newOrigSub[1] = newOrigSub[1];
		}
		else
		{
			newOrigSub[1] = newOrigSub[1] - 10;
		}
		if (newOrigSub[2] - 10 < 0)
		{
			newOrigSub[2] = newOrigSub[2];
		}
		else
		{
			newOrigSub[2] = newOrigSub[2] - 10;
		}*/
		vtkSmartPointer<vtkExtractVOI> extract = vtkSmartPointer<vtkExtractVOI>::New();
		extract->SetVOI(newOrigSub[0], finalSub[0], newOrigSub[1], finalSub[1], newOrigSub[2], finalSub[2]);
		extract->SetSampleRate(1, 1, 1);
		extract->SetInputData(aneurysmImageData);
		extract->ReleaseDataFlagOff();
		extract->Update();
		newAneurysmImageData = extract->GetOutput();
		//newDims[0] = finalSub[0] - newOrigSub[0] + 1;
		//newDims[1] = finalSub[1] - newOrigSub[1] + 1;
		//newDims[2] = finalSub[2] - newOrigSub[2] + 1;
		//newAneurysmImageData->SetDimensions(newDims);
		//newAneurysmImageData->SetOrigin(newOrigSub[0], newOrigSub[1], newOrigSub[2]);
		//float* newaneurysmDataPointer = static_cast<float*>(newAneurysmImageData->GetScalarPointer());
		//for (int i = 0;i < newDims[0];i++)
		//{
		//	for (int j = 0;j < newDims[1];j++)
		//	{
		//		for (int k = 0;k < newDims[2];k++)
		//		{
		//			int newId = sub2id(i, j, k, newDims);
		//			int origSub[3];
		//			origSub[0] = i + newOrigSub[0];
		//			origSub[1] = j + newOrigSub[1];
		//			origSub[2] = k + newOrigSub[2];
		//			int origId = sub2id(origSub[0],origSub[1],origSub[2],origDims);
		//			newaneurysmDataPointer[newId] = aneurysmDataPointer[origId];
		//		}
		//	}
		//}
	}
	else if (this->aneurysmData.seeds->GetNumberOfPoints() > 0)
	{
		int seedpointSub[3];
		int id = this->aneurysmImageData->FindPoint(this->aneurysmData.seeds->GetPoint(0));
		id2sub(id, origDims, seedpointSub);

		int origPointSub[3], finalPointSub[3];
		onePoint2volPoints(origDims, seedpointSub, origPointSub, finalPointSub);
		/*if (seedpointSub[0] - 30 < 0)
		{
			origPointSub[0] = seedpointSub[0];
		}
		else
		{
			origPointSub[0] = seedpointSub[0] - 30;
		}
		if (seedpointSub[1] - 30 < 0)
		{
			origPointSub[1] = seedpointSub[1];
		}
		else
		{
			origPointSub[1] = seedpointSub[1] - 30;
		}
		if (seedpointSub[2] - 30 < 0)
		{
			origPointSub[2] = seedpointSub[2];
		}
		else
		{
			origPointSub[2] = seedpointSub[2] - 30;
		}

		if (seedpointSub[0] + 30 > origDims[0])
		{
			finalPointSub[0] = seedpointSub[0];
		}
		else
		{
			finalPointSub[0] = seedpointSub[0] + 30;
		}
		if (seedpointSub[1] + 30 > origDims[1])
		{
			finalPointSub[1] = seedpointSub[1];
		}
		else
		{
			finalPointSub[1] = seedpointSub[1] + 30;
		}
		if (seedpointSub[2] + 30 > origDims[2])
		{
			finalPointSub[2] = seedpointSub[2];
		}
		else
		{
			finalPointSub[2] = seedpointSub[2] + 30;
		}*/
		vtkSmartPointer<vtkExtractVOI> extract = vtkSmartPointer<vtkExtractVOI>::New();
		extract->SetVOI(origPointSub[0], finalPointSub[0], origPointSub[1], finalPointSub[1], origPointSub[2], finalPointSub[2]);
		extract->SetSampleRate(1, 1, 1);
		extract->SetInputData(aneurysmImageData);
		extract->ReleaseDataFlagOff();
		extract->Update();
		newAneurysmImageData = extract->GetOutput();
	}

    // Dilate the seeds to have the anuerysm regions
    vtkSmartPointer<vtkImageDilateErode3D> dilate3D = vtkSmartPointer<vtkImageDilateErode3D>::New();
    dilate3D->SetErodeValue(0);
    dilate3D->SetDilateValue(1);
    dilate3D->SetKernelSize(3, 3, 3);
	if ((this->firstAneurysmLocPointIdx > 0 && this->lastAneurysmLocPointIdx > 0) || this->aneurysmData.seeds->GetNumberOfPoints()>0)
	{
		dilate3D->SetInputData(newAneurysmImageData);
	}
	else
	{
		dilate3D->SetInputData(this->aneurysmImageData);
	}
    dilate3D->Update();
    
    for (int iter = 0; iter < 14; iter++)
    {
        vtkSmartPointer<vtkImageData> dilate3DOutput = dilate3D->GetOutput();
        dilate3D = vtkSmartPointer<vtkImageDilateErode3D>::New();
        dilate3D->SetErodeValue(0);
        dilate3D->SetDilateValue(1);
        dilate3D->SetKernelSize(5, 5, 5);
        dilate3D->SetInputData(dilate3DOutput);
        dilate3D->Update();
    }

	dilate3D->GetOutput()->GetDimensions(newDims);
	vtkSmartPointer<vtkImageData> dilatedData = vtkSmartPointer<vtkImageData>::New();
	dilatedData->DeepCopy(this->aneurysmImageData);
	float* dilatedDataPointer;
	//float* dataPointer;
	if ((this->firstAneurysmLocPointIdx > 0 && this->lastAneurysmLocPointIdx > 0)|| this->aneurysmData.seeds->GetNumberOfPoints()>0)
	{
		float* dilatedPointer = static_cast<float*>(dilate3D->GetOutput()->GetScalarPointer());
		dilatedDataPointer = static_cast<float*>(dilatedData->GetScalarPointer());
		for (int i = 0;i < newDims[0]*newDims[1]*newDims[2];i++)
		{
			dilatedDataPointer[dilatedData->FindPoint(dilate3D->GetOutput()->GetPoint(i))] = dilatedPointer[i];
		}
	}
	else
	{
		dilatedDataPointer = static_cast<float*>(dilate3D->GetOutput()->GetScalarPointer());
	}
    //float* dilatedDataPointer = static_cast<float*>(dilate3D->GetOutput()->GetScalarPointer());
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        aneurysmDataPointer[i] = 0;
        if (segmentedDataPointer[i] > 0 && dilatedDataPointer[i] > 0)
        {
            aneurysmDataPointer[i] = 1;
        }
    }
    
    // Check if seed points are foreground points
    for (int i = 0; i < this->aneurysmData.seeds->GetNumberOfPoints(); i++)
    {
        int id = this->aneurysmImageData->FindPoint(this->aneurysmData.seeds->GetPoint(i));
        if (aneurysmDataPointer[id] == 0)
        {
            // Now we need to make it to be 1
            aneurysmDataPointer[id] = 1;
        }
    }
    
    vtkSmartPointer<vtkImageThresholdConnectivity> connectedThresholdAfterDilation = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
    connectedThresholdAfterDilation->ReplaceInOn();
    connectedThresholdAfterDilation->SetInValue(1);
    connectedThresholdAfterDilation->ReplaceOutOn();
    connectedThresholdAfterDilation->SetOutValue(0);
    connectedThresholdAfterDilation->ThresholdByUpper(1);
    connectedThresholdAfterDilation->SetInputData(this->aneurysmImageData);
    connectedThresholdAfterDilation->SetSeedPoints(this->aneurysmData.seeds);
    connectedThresholdAfterDilation->Update();
    
    this->aneurysmImageData->DeepCopy(connectedThresholdAfterDilation->GetOutput());
    aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());
    
    // Find the entry for this rough region
    int firstPointIndex = -1;
    double firstPoint[3];
	int lastPointIndex = -1;
	double lastPoint[3];

	if (this->patientInfo.patientID == "10003526")
	{
		firstPoint[0] = -12.768;
		firstPoint[1] = -49.546;
		firstPoint[2] = -274.547;
		firstPointIndex = 141;

		lastPoint[0] = -12.768;
		lastPoint[1] = -42.049;
		lastPoint[2] = -269.192;
		lastPointIndex = 168;
	}
	else if (this->patientInfo.patientID == "667422")
	{
		firstPoint[0] = -9.914;
		firstPoint[1] = -17.134;
		firstPoint[2] = -168.334;
		firstPointIndex = 709;

		lastPoint[0] = -4.466;
		lastPoint[1] = -23.263;
		lastPoint[2] = -172.42;
		lastPointIndex = 494;
	}
	else if (this->firstAneurysmLocPointIdx > 0 && this->lastAneurysmLocPointIdx > 0)
	{
		firstPointIndex = this->firstAneurysmLocPointIdx;
		skeletonPoints->GetPoint(firstPointIndex,firstPoint);
		lastPointIndex = this->lastAneurysmLocPointIdx;
		skeletonPoints->GetPoint(lastPointIndex, lastPoint);
	}
	else
	{
		for (int i = 0; i < skeletonPoints->GetNumberOfPoints(); i++)
		{
			if (aneurysmDataPointer[this->aneurysmImageData->FindPoint(skeletonPoints->GetPoint(i))] == 1)
			{
				skeletonPoints->GetPoint(i, firstPoint);
				firstPointIndex = i;
				break;
			}
		}

		double maxDist = 0;
		for (int i = firstPointIndex + 1; i < skeletonPoints->GetNumberOfPoints(); i++)
		{
			if (aneurysmDataPointer[this->aneurysmImageData->FindPoint(skeletonPoints->GetPoint(i))] > 0)
			{
				double skePoint[3];
				skeletonPoints->GetPoint(i, skePoint);
				double dist = normDist(skePoint, this->aneurysmData.seeds->GetPoint(0));
				if (dist > maxDist) {
					maxDist = dist;
					lastPointIndex = i;
				}
			}
		}
		skeletonPoints->GetPoint(lastPointIndex, lastPoint);
	}

    // Get the radius
	int ids = firstPointIndex;
	int ide = lastPointIndex;
	std::vector<double> radiuses;
	double radiusSum = 0.0;
	double radius = 0.0;
	if (firstPointIndex > lastPointIndex)
	{
		ids = lastPointIndex;
		ide = firstPointIndex;
	}
	for (int i = ids;i < ide;i++)
	{
		double firstpointtem[3];
		skeletonPoints->GetPoint(i, firstpointtem);
		double rad = 99999;
		int firstPointSub[3];
		int roundPointSub[3];
		double roundPoint[3];
		id2sub(this->segmentedData->FindPoint(firstpointtem), origDims, firstPointSub);
		for (int x = -30; x < 30; x++)
		{
			int xs = x + firstPointSub[0];
			if (xs < 0) {
				xs = 0;
			}
			if (xs >= origDims[0])
			{
				xs = origDims[0] - 1;
			}
			roundPointSub[0] = xs;
			for (int y = -30; y < 30; y++)
			{
				int ys = y + firstPointSub[1];
				if (ys < 0) {
					ys = 0;
				}
				if (ys >= origDims[1])
				{
					ys = origDims[1] - 1;
				}
				roundPointSub[1] = ys;
				for (int z = -30; z < 30; z++)
				{
					int zs = z + firstPointSub[2];
					if (zs < 0) {
						zs = 0;
					}
					if (zs >= origDims[2])
					{
						zs = origDims[2] - 1;
					}
					roundPointSub[2] = zs;
					if (segmentedDataPointer[sub2id(xs, ys, zs, origDims)] == 0) {
						double distance = normDist(firstPointSub, roundPointSub);
						if (rad > distance)
						{
							rad = distance;
							this->segmentedData->GetPoint(sub2id(xs, ys, zs, origDims), roundPoint);
						}
					}
				}
			}
		}
		if (rad < 99999)
		{
			double dist = normDist(firstpointtem, roundPoint);
			radiuses.push_back(dist);
			radiusSum = radiusSum + dist;
		}
	}
    /*double radius = 99999;
    int firstPointSub[3];
    int roundPointSub[3];
    double roundPoint[3];
    id2sub(this->segmentedData->FindPoint(firstPoint), origDims, firstPointSub);
    for (int x = -30; x < 30; x++)
    {
        int xs = x + firstPointSub[0];
        if (xs < 0) {
            xs = 0;
        }
        if (xs >= origDims[0])
        {
            xs = origDims[0] - 1;
        }
        roundPointSub[0] = xs;
        for (int y = -30; y < 30; y++)
        {
            int ys = y + firstPointSub[1];
            if (ys < 0) {
                ys = 0;
            }
            if (ys >= origDims[1])
            {
                ys = origDims[1] - 1;
            }
            roundPointSub[1] = ys;
            for (int z = -30; z < 30; z++)
            {
                int zs = z + firstPointSub[2];
                if (zs < 0) {
                    zs = 0;
                }
                if (zs >= origDims[2])
                {
                    zs = origDims[2] - 1;
                }
                roundPointSub[2] = zs;
                if (segmentedDataPointer[sub2id(xs, ys, zs, origDims)] == 0) {
                    double distance = normDist(firstPointSub, roundPointSub);
                    if (radius > distance)
                    {
                        radius = distance;
                        this->segmentedData->GetPoint(sub2id(xs, ys, zs, origDims), roundPoint);
                    }
                }
            }
        }
    }
    radius = normDist(firstPoint, roundPoint);*/
	if (radiuses.size()>0)
	{
		radius = radiusSum / radiuses.size();
		this->aneurysmData.vesselRadius = radius;
	}
	
    // Find the path along the artery
	int flipNormals = 0;
	auto radiusArrayName = "Radius";
	auto costFunction = "1/R";
	double mindistance_firstpoint = 9999999;
	double mindistance_lastpoint = 9999999;
	int ind_first = 0;
	int ind_last = 0;
	for (int i = 0;i < this->vesselSurface->GetNumberOfPoints();i++)
	{
		double pt[3];
		this->vesselSurface->GetPoint(i, pt);
		double dist = normDist(pt, firstPoint);
		if (dist < mindistance_firstpoint)
		{
			mindistance_firstpoint = dist;
			ind_first = i;
		}
		double dist2 = normDist(pt, lastPoint);
		if (dist2 < mindistance_lastpoint)
		{
			mindistance_lastpoint = dist2;
			ind_last = i;
		}
	}
	vtkSmartPointer<vtkIdList> inletPointIds = vtkSmartPointer<vtkIdList>::New();
	inletPointIds->InsertNextId(ind_first);
	vtkSmartPointer<vtkIdList> outletPointIds = vtkSmartPointer<vtkIdList>::New();
	outletPointIds->InsertNextId(ind_last);
	vtkSmartPointer<vtkvmtkPolyDataCenterlines> centerlineFilter = vtkSmartPointer<vtkvmtkPolyDataCenterlines>::New();
	centerlineFilter->SetInputData(this->vesselSurface);
	centerlineFilter->SetSourceSeedIds(inletPointIds);
	centerlineFilter->SetTargetSeedIds(outletPointIds);
	centerlineFilter->SetRadiusArrayName(radiusArrayName);
	centerlineFilter->SetCostFunction(costFunction);
	centerlineFilter->SetFlipNormals(flipNormals);
	centerlineFilter->SetAppendEndPointsToCenterlines(0);
	centerlineFilter->SetSimplifyVoronoi(0);
	centerlineFilter->SetCenterlineResampling(0);
	centerlineFilter->SetResamplingStepLength(1.0);
	centerlineFilter->Update();

    vtkSmartPointer<vtkDijkstraGraphGeodesicPath> djForPath = vtkSmartPointer<vtkDijkstraGraphGeodesicPath>::New();
    djForPath->SetInputData(this->graphToPolyData->GetOutput());
    djForPath->SetStartVertex(firstPointIndex);
    djForPath->SetEndVertex(lastPointIndex);
    djForPath->StopWhenEndReachedOn();
    djForPath->Update();
    
    vtkSmartPointer<vtkPolyData> path = centerlineFilter->GetOutput();

	//vtkDoubleArray* radiusArray = vtkDoubleArray::SafeDownCast(path->GetPointData()->GetArray("Radius"));
	//radiusArray->GetTuple1()
    
    vtkSmartPointer<vtkPoints> pathPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> pathVertices = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < path->GetNumberOfPoints(); i++)
    {
        vtkIdType pid[1];
        pid[0] = i;
        pathVertices->InsertNextCell(1, pid);
        double pathPoint[3];
        path->GetPoint(i, pathPoint);
        pathPoints->InsertNextPoint(pathPoint);
    }
    
    vtkSmartPointer<vtkPolyData> pathPointsPolyData = vtkSmartPointer<vtkPolyData>::New();
    pathPointsPolyData->SetPoints(pathPoints);
    pathPointsPolyData->SetVerts(pathVertices);
    
	//smooth the path by mean filter
	/*if (path->GetNumberOfPoints() > 5)
	{
		for (int i = 0; i < pathPoints->GetNumberOfPoints();i++)
		{
			if (i < 4)
			{
				pathPoints->InsertPoint(0, pathPoints->GetPoint(0));
				pathPoints->InsertPoint(1, pathPoints->GetPoint(1));
				pathPoints->InsertPoint(2, pathPoints->GetPoint(2));
				pathPoints->InsertPoint(3, pathPoints->GetPoint(3));
			}
			else
			{
				int id0, id1, id2, id3, id4;
				id0 = this->aneurysmImageData->FindPoint(pathPoints->GetPoint(i));
				id1 = this->aneurysmImageData->FindPoint(pathPoints->GetPoint(i - 1));
				id2 = this->aneurysmImageData->FindPoint(pathPoints->GetPoint(i - 2));
				id3 = this->aneurysmImageData->FindPoint(pathPoints->GetPoint(i - 3));
				id4 = this->aneurysmImageData->FindPoint(pathPoints->GetPoint(i - 4));
				int p0sub[3], p1sub[3], p2sub[3], p3sub[3], p4sub[3];
				id2sub(id0, origDims, p0sub);
				id2sub(id1, origDims, p1sub);
				id2sub(id2, origDims, p2sub);
				id2sub(id3, origDims, p3sub);
				id2sub(id4, origDims, p4sub);
				p4sub[0] = (p0sub[0] + p1sub[0] + p2sub[0] + p3sub[0] + p4sub[0]) / 5;
				p4sub[1] = (p0sub[1] + p1sub[1] + p2sub[1] + p3sub[1] + p4sub[1]) / 5;
				p4sub[2] = (p0sub[2] + p1sub[2] + p2sub[2] + p3sub[2] + p4sub[2]) / 5;
				int idtem = sub2id(p4sub[0], p4sub[1], p4sub[2], origDims);
				pathPoints->InsertPoint(i, this->aneurysmImageData->GetPoint(idtem));
			}
		}
	}
	path->SetPoints(pathPoints);*/
	/*vtkSmartPointer<vtkDiscreteMarchingCubes> aneurysmSurfaceIni = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
	aneurysmSurfaceIni->SetInputData(this->aneurysmImageData);
	aneurysmSurfaceIni->GenerateValues(1,1,1);
	aneurysmSurfaceIni->Update();

	vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer<vtkDelaunay3D>::New();
	delaunay->SetInputData(aneurysmSurfaceIni->GetOutput());
	delaunay->Update();
	vtkSmartPointer<vtkDataSetSurfaceFilter> surfacefilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
	surfacefilter->SetInputConnection(delaunay->GetOutputPort());
	surfacefilter->Update();

	vtkSmartPointer<vtkPolyDataToImageStencil> Poly2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
	Poly2stenc->SetInputConnection(surfacefilter->GetOutputPort());
	Poly2stenc->SetOutputOrigin(this->aneurysmImageData->GetOrigin());
	Poly2stenc->SetOutputSpacing(this->aneurysmImageData->GetSpacing());
	Poly2stenc->SetOutputWholeExtent(this->aneurysmImageData->GetExtent());
	Poly2stenc->Update();

	vtkSmartPointer<vtkImageStencil> Imgstenc = vtkSmartPointer<vtkImageStencil>::New();
	Imgstenc->SetInputData(this->aneurysmImageData);
	Imgstenc->SetStencilConnection(Poly2stenc->GetOutputPort());
	Imgstenc->ReverseStencilOff();
	Imgstenc->SetBackgroundValue(0);
	Imgstenc->Update();

	float* imgstencPointer = static_cast<float*>(Imgstenc->GetOutput()->GetScalarPointer());
	//float* connectedPointer = static_cast<float*>(connectedThresholdAfterRadiusFilter->GetOutput()->GetScalarPointer());

	for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
	{
		if (imgstencPointer[i] > 0)
		{
			aneurysmDataPointer[i] = 1;
		}
	}

	VTKToITKConnectorType::Pointer vtkToItkConnector = VTKToITKConnectorType::New();
	vtkToItkConnector->SetInput(this->aneurysmImageData);
	vtkToItkConnector->Update();
	IMFillFilterType::Pointer imfill = IMFillFilterType::New();
	imfill->SetInput(vtkToItkConnector->GetOutput());
	imfill->Update();

	ITKToVTKConnectorType::Pointer itkToVtkConnector = ITKToVTKConnectorType::New();
	itkToVtkConnector->SetInput(imfill->GetOutput());
	itkToVtkConnector->Update();

	this->aneurysmImageData->DeepCopy(itkToVtkConnector->GetOutput());
	aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());*/

    // Eliminate the region that is too close to the artery path
	double radiusRatio = 1.2;
	if (this->patientInfo.patientID == "10003526")
	{
		radiusRatio = 2.0;
	}
	else if (this->patientInfo.patientID == "667422")
	{
		radiusRatio = 3.0;
	}
    double minDist = 1000000;
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] > 0)
        {
            for (int j = 0; j < path->GetNumberOfPoints(); j++)
            {
                double* point = this->aneurysmImageData->GetPoint(i);
                double dist = normDist(point, path->GetPoint(j));
                if (minDist > dist)
                {
                    minDist = dist;
                }
            }
            if (minDist < radius * radiusRatio)
            {
                aneurysmDataPointer[i] = 0;
            }
            minDist = 1000000;
        }
    }
    
    vtkSmartPointer<vtkImageThresholdConnectivity> connectedThresholdAfterRadiusFilter = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
    connectedThresholdAfterRadiusFilter->ReplaceInOn();
    connectedThresholdAfterRadiusFilter->SetInValue(1);
    connectedThresholdAfterRadiusFilter->ReplaceOutOn();
    connectedThresholdAfterRadiusFilter->SetOutValue(0);
    connectedThresholdAfterRadiusFilter->ThresholdByUpper(1);
    connectedThresholdAfterRadiusFilter->SetInputData(this->aneurysmImageData);
    connectedThresholdAfterRadiusFilter->SetSeedPoints(this->aneurysmData.seeds);
    connectedThresholdAfterRadiusFilter->Update();

	vtkSmartPointer<vtkDiscreteMarchingCubes> aneurysmSurface = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> aneurysmSmoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();

    aneurysmSurface->SetInputData(connectedThresholdAfterRadiusFilter->GetOutput());
    aneurysmSurface->GenerateValues(1, 1, 1);
    aneurysmSurface->Update();

	//vtkSmartPointer<vtkDelaunay3D> delaunay1 = vtkSmartPointer<vtkDelaunay3D>::New();
	//delaunay1->SetInputData(aneurysmSurfaceIni->GetOutput());
	//delaunay1->Update();
	//vtkSmartPointer<vtkDataSetSurfaceFilter> surfacefilter1 = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
	//surfacefilter1->SetInputConnection(delaunay1->GetOutputPort());
	//surfacefilter1->Update();

    vtkSmartPointer<vtkPolyData> aneurysmPolyData = aneurysmSurface->GetOutput();
	//vtkSmartPointer<vtkPolyData> aneurysmPolyData = surfacefilter1->GetOutput();
    if (aneurysmPolyData->GetNumberOfPoints() == 0)
    {
        this->aneurysmData.aneurysmSurfaceData = NULL;
    }
    else
    {
        aneurysmSmoother->SetInputConnection(aneurysmSurface->GetOutputPort());
		//aneurysmSmoother->SetInputConnection(surfacefilter1->GetOutputPort());
        aneurysmSmoother->Update();
        this->aneurysmData.aneurysmSurfaceData = aneurysmSmoother->GetOutput();
    }
    
    this->aneurysmImageData->DeepCopy(connectedThresholdAfterRadiusFilter->GetOutput());
    aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());

	//get the new path
	/*for (int i = 0;i < 5;i++)
	{
		this->aneurysmImageData->DeepCopy(connectedThresholdAfterRadiusFilter->GetOutput());
		aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());

		for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
		{
			if (aneurysmDataPointer[i] > 0)
			{
				segmentedDataPointer[i] = 0;
			}
		}
		vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
		writer->SetFileName("D:/segmentData2.mha");
		writer->SetInputData(this->segmentedData);
		writer->Write();
		this->needRecalculateSkeleton = true;
		generateSkeleton();
		this->segmentedData->DeepCopy(this->segmentedDataBackup);
		segmentedDataPointer = static_cast<float*>(this->segmentedData->GetScalarPointer());

		std::cout << "first point index:" << this->firstAneurysmLocPointIdx << std::endl;
		std::cout << "last point index:" << this->lastAneurysmLocPointIdx << std::endl;
		setAneurysmLocationPoint(1, firstPoint);
		setAneurysmLocationPoint(2, lastPoint);

		vtkSmartPointer<vtkDijkstraGraphGeodesicPath> djForPath0 = vtkSmartPointer<vtkDijkstraGraphGeodesicPath>::New();
		djForPath0->SetInputData(this->graphToPolyData->GetOutput());
		djForPath0->SetStartVertex(this->firstAneurysmLocPointIdx);
		djForPath0->SetEndVertex(this->lastAneurysmLocPointIdx);
		djForPath0->StopWhenEndReachedOn();
		djForPath0->Update();
		std::cout << "first point index:" << this->firstAneurysmLocPointIdx << std::endl;
		std::cout << "last point index:" << this->lastAneurysmLocPointIdx << std::endl;

		path = djForPath0->GetOutput();

		this->aneurysmImageData->DeepCopy(connectedThresholdAfterRadiusFilter->GetOutput());
		aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());
		// Eliminate the region that is too close to the artery path
		//double minDist = 1000000;
		minDist = 1000000;
		for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
		{
			if (aneurysmDataPointer[i] > 0)
			{
				for (int j = 0; j < path->GetNumberOfPoints(); j++)
				{
					double* point = this->aneurysmImageData->GetPoint(i);
					double dist = normDist(point, path->GetPoint(j));
					if (minDist > dist)
					{
						minDist = dist;
					}
				}
				if (minDist < radius * 1.0)
				{
					aneurysmDataPointer[i] = 0;
				}
				minDist = 1000000;
			}
		}

		//vtkSmartPointer<vtkImageThresholdConnectivity> connectedThresholdAfterRadiusFilter = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
		connectedThresholdAfterRadiusFilter->ReplaceInOn();
		connectedThresholdAfterRadiusFilter->SetInValue(1);
		connectedThresholdAfterRadiusFilter->ReplaceOutOn();
		connectedThresholdAfterRadiusFilter->SetOutValue(0);
		connectedThresholdAfterRadiusFilter->ThresholdByUpper(1);
		connectedThresholdAfterRadiusFilter->SetInputData(this->aneurysmImageData);
		connectedThresholdAfterRadiusFilter->SetSeedPoints(this->aneurysmData.seeds);
		connectedThresholdAfterRadiusFilter->Update();

		vtkSmartPointer<vtkDiscreteMarchingCubes> aneurysmSurface = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
		vtkSmartPointer<vtkWindowedSincPolyDataFilter> aneurysmSmoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();

		aneurysmSurface->SetInputData(connectedThresholdAfterRadiusFilter->GetOutput());
		aneurysmSurface->GenerateValues(1, 1, 1);
		aneurysmSurface->Update();

		vtkSmartPointer<vtkPolyData> aneurysmPolyData = aneurysmSurface->GetOutput();
		if (aneurysmPolyData->GetNumberOfPoints() == 0)
		{
			this->aneurysmData.aneurysmSurfaceData = NULL;
		}
		else
		{
			aneurysmSmoother->SetInputConnection(aneurysmSurface->GetOutputPort());
			aneurysmSmoother->Update();
			this->aneurysmData.aneurysmSurfaceData = aneurysmSmoother->GetOutput();
		}

		this->aneurysmImageData->DeepCopy(connectedThresholdAfterRadiusFilter->GetOutput());
		aneurysmDataPointer = static_cast<float*>(this->aneurysmImageData->GetScalarPointer());
	}*/


    // Find the closest points inside the aneurysm and the artery path
    double minGlobalDistance = 100000;
    int iIndex = -1;
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] > 0)
        {
            double minDistance = 100000;
            for (int j = 0; j < path->GetNumberOfPoints(); j++)
            {
                double* point = this->aneurysmImageData->GetPoint(i);
                double dist = normDist(point, path->GetPoint(j));
                if (dist < minDistance)
                {
                    minDistance = dist;
                }
            }
            if (minDistance < minGlobalDistance)
            {
                minGlobalDistance = minDistance;
                iIndex = i;
            }
        }
    }
    
    int jIndex = -1;
    double minDistance = 100000;
    for (int j = 0; j < path->GetNumberOfPoints(); j++)
    {
        double* point = this->aneurysmImageData->GetPoint(iIndex);
        double dist = normDist(point, path->GetPoint(j));
        if (dist < minDistance)
        {
            minDistance = dist;
            jIndex = j;
        }
    }
    
    double aneurysmPoint1[3];
    this->aneurysmImageData->GetPoint(iIndex, aneurysmPoint1);
    
    double pathPoint2[3];
    path->GetPoint(jIndex, pathPoint2);
    
	std::vector<int> pathPointIdx;
	std::vector<int> aneurysmPointIdx;
	for (int i = 0; i < path->GetNumberOfPoints();i++)
	{
		double minval = 999999;
		int jIdx = -1;
		for (int j = 0;j < origDims[0]*origDims[1]*origDims[2];j++)
		{
			double dist;
			if (aneurysmDataPointer[j] > 0)
			{
				dist = normDist(path->GetPoint(i), this->aneurysmImageData->GetPoint(j));
			}
			if (dist < radius*4)
			{
				if (dist < minval)
				{
					minval = dist;
					jIdx = j;
				}
			}
		}
		if (jIdx > 0)
		{
			pathPointIdx.push_back(i);
			aneurysmPointIdx.push_back(jIdx);
		}
	}
	double normalDirection[3];
	//double neckcenterPoint_test[3] = {0.0,0.0,0.0};
	if (pathPointIdx.size()>0)
	{
		std::cout << "normal point number:" << pathPointIdx.size() << endl;
		std::cout << "path point number:" << path->GetNumberOfPoints() << endl;
		for (size_t i = 0; i < pathPointIdx.size(); i++)
		{
			double* anPoint = this->aneurysmImageData->GetPoint(aneurysmPointIdx[i]);
			double* pathPoint = path->GetPoint(pathPointIdx[i]);
			normalDirection[0] = normalDirection[0] + anPoint[0] - pathPoint[0];
			normalDirection[1] = normalDirection[1] + anPoint[1] - pathPoint[1];
			normalDirection[2] = normalDirection[2] + anPoint[2] - pathPoint[2];
			/*neckcenterPoint_test[0] = neckcenterPoint_test[0] + anPoint[0];
			neckcenterPoint_test[1] = neckcenterPoint_test[1] + anPoint[1];
			neckcenterPoint_test[2] = neckcenterPoint_test[2] + anPoint[2];*/
		}
		normalDirection[0] = normalDirection[0] / pathPointIdx.size();
		normalDirection[1] = normalDirection[1] / pathPointIdx.size();
		normalDirection[2] = normalDirection[2] / pathPointIdx.size();

		/*neckcenterPoint_test[0] = neckcenterPoint_test[0] / pathPointIdx.size();
		neckcenterPoint_test[1] = neckcenterPoint_test[1] / pathPointIdx.size();
		neckcenterPoint_test[2] = neckcenterPoint_test[2] / pathPointIdx.size();*/
	}
	else
	{
		normalDirection[0] = aneurysmPoint1[0] - pathPoint2[0];
		normalDirection[1] = aneurysmPoint1[1] - pathPoint2[1];
		normalDirection[2] = aneurysmPoint1[2] - pathPoint2[2];
	}
	/*this->aneurysmData.neckcenterpoint_test[0] = neckcenterPoint_test[0];
	this->aneurysmData.neckcenterpoint_test[1] = neckcenterPoint_test[1];
	this->aneurysmData.neckcenterpoint_test[2] = neckcenterPoint_test[2];*/
    //double normalDirection[3];
    /*normalDirection[0] = aneurysmPoint1[0] - pathPoint2[0];
    normalDirection[1] = aneurysmPoint1[1] - pathPoint2[1];
    normalDirection[2] = aneurysmPoint1[2] - pathPoint2[2];*/

	this->aneurysmData.aneurysmPoint1[0] = aneurysmPoint1[0];
	this->aneurysmData.aneurysmPoint1[1] = aneurysmPoint1[1];
	this->aneurysmData.aneurysmPoint1[2] = aneurysmPoint1[2];
    // Find the points that are furthest from the entry
    double neckRadius1 = 0;
    int neckRadius1Index;
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] == 1)
        {
            double point[3];
            this->aneurysmImageData->GetPoint(i, point);
            double distance = normDist(point, aneurysmPoint1);
            double ang = angle(aneurysmPoint1, point, normalDirection);
            // Perpendicular to direction
            if (abs(ang - 3.1415 / 2.0) < 0.3)
            {
                if (distance > neckRadius1) {
                    neckRadius1 = distance;
                    neckRadius1Index = i;
                }
            }
        }
    }
    
    // minimum distance from outside
    double neckRadius2 = 999999;
    int neckRadius2Index;
    int aneurysmPoint1Sub[3];
    id2sub(this->segmentedData->FindPoint(aneurysmPoint1), origDims, aneurysmPoint1Sub);
    for (int x = -30; x < 30; x++)
    {
        int xs = x + aneurysmPoint1Sub[0];
        if (xs < 0) {
            xs = 0;
        }
        if (xs >= origDims[0])
        {
            xs = origDims[0] - 1;
        }
        for (int y = -30; y < 30; y++)
        {
            int ys = y + aneurysmPoint1Sub[1];
            if (ys < 0) {
                ys = 0;
            }
            if (ys >= origDims[1])
            {
                ys = origDims[1] - 1;
            }
            for (int z = -30; z < 30; z++)
            {
                int zs = z + aneurysmPoint1Sub[2];
                if (zs < 0) {
                    zs = 0;
                }
                if (zs >= origDims[2])
                {
                    zs = origDims[2] - 1;
                }
                int id = sub2id(xs, ys, zs, origDims);
                if (segmentedDataPointer[id] == 0) {
                    double point[3];
                    this->aneurysmImageData->GetPoint(id, point);
                    double ang = angle(aneurysmPoint1, point, normalDirection);
                    double distance = normDist(point, aneurysmPoint1);
                    // Perpendicular to direction
                    if (abs(ang - 3.1415 / 2.0) < 0.3)
                    {
                        if (distance < neckRadius2) {
                            neckRadius2 = distance;
                            neckRadius2Index = id;
                        }
                    }
                }
            }
        }
    }
    double neckRadius = (neckRadius1 + neckRadius2) / 2.0;
    
    double neckPoint1[3];
    this->aneurysmImageData->GetPoint(neckRadius1Index, neckPoint1);
    
    double neckPoint2[3];
    this->aneurysmImageData->GetPoint(neckRadius2Index, neckPoint2);
    
    // Find the center neck point and make sure it is within the aneurysm
    double neckCenterPoint[3];
    neckCenterPoint[0] = (neckPoint1[0] + neckPoint2[0]) / 2.0;
    neckCenterPoint[1] = (neckPoint1[1] + neckPoint2[1]) / 2.0;
    neckCenterPoint[2] = (neckPoint1[2] + neckPoint2[2]) / 2.0;
    
    minDistance = 99999;
    int minDistanceIndex = -1;
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] == 1)
        {
            double point[3];
            this->aneurysmImageData->GetPoint(i, point);
            double distance = normDist(point, neckCenterPoint);
            if (distance < minDistance) {
                minDistance = distance;
                minDistanceIndex = i;
            }
        }
    }
    this->aneurysmImageData->GetPoint(minDistanceIndex, neckCenterPoint);
    
    // Find the closest path point
    minDistance = 99999;
    minDistanceIndex = -1;
    for (int j = 0; j < path->GetNumberOfPoints(); j++)
    {
        double dist = normDist(neckCenterPoint, path->GetPoint(j));
        if (dist < minDistance)
        {
            minDistance = dist;
            minDistanceIndex = j;
        }
    }
    
    double pathCenterPoint[3];
    path->GetPoint(minDistanceIndex, pathCenterPoint);

	//double normalDirPoint[3];
	this->aneurysmData.normalDirPoint[0] = aneurysmPoint1[0] + 3 * normalDirection[0];
	this->aneurysmData.normalDirPoint[1] = aneurysmPoint1[1] + 3 * normalDirection[1];
	this->aneurysmData.normalDirPoint[2] = aneurysmPoint1[2] + 3 * normalDirection[2];
    
    /*normalDirection[0] = neckCenterPoint[0] - pathCenterPoint[0];
    normalDirection[1] = neckCenterPoint[1] - pathCenterPoint[1];
    normalDirection[2] = neckCenterPoint[2] - pathCenterPoint[2];*/
    
    // All calculations happen here
    // Find the points that are furthest from the entry
    double farDistance = 0;
    int farPointIndex = -1;
    
    std::list<int> neckPointsList;
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] == 1)
        {
            double point[3];
            this->aneurysmImageData->GetPoint(i, point);
            double distance = normDist(point, neckCenterPoint);
            if (distance > farDistance) {
                farDistance = distance;
                farPointIndex = i;
            }
            double ang = angle(neckCenterPoint, point, normalDirection);
            // Perpendicular to direction
            if (abs(ang - 3.1415 / 2.0) < 0.3)
            {
                neckPointsList.push_back(i);
            }
        }
    }
    
    double maxHeight = 0;
    int maxHeightIndex = -1;
    /*
    int maxHeightNeckPointIndex = -1;
    std::list<int>::iterator it;
    for (it = neckPointsList.begin(); it != neckPointsList.end(); it++)
    {
        int neckPointIndex = *it;
        double neckPoint[3];
        this->aneurysmImageData->GetPoint(neckPointIndex, neckPoint);
        for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
        {
            if (aneurysmDataPointer[i] == 1 && i != neckPointIndex)
            {
                double point[3];
                this->aneurysmImageData->GetPoint(i, point);
                double distance = normDist(point, neckPoint);
                double ang = angle(neckPoint, point, normalDirection);
                
                // Parallel to direction
                if (ang < 0.3)
                {
                    if (distance > maxHeight) {
                        maxHeight = distance;
                        maxHeightIndex = i;
                        maxHeightNeckPointIndex = neckPointIndex;
                    }
                }
            }
        }
    }
     */
    // Use neck center point
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] == 1)
        {
            double point[3];
            this->aneurysmImageData->GetPoint(i, point);
            double distance = normDist(point, neckCenterPoint);
            double ang = angle(neckCenterPoint, point, normalDirection);
            
            // Parallel to direction
            if (ang < 0.3)
            {
                if (distance > maxHeight) {
                    maxHeight = distance;
                    maxHeightIndex = i;
                }
            }
        }
    }
    
	double inflowDirection[3] = { 0.0, 0.0, 0.0 };
    /*inflowDirection[0] = pathCenterPoint[0] - firstPoint[0];
    inflowDirection[1] = pathCenterPoint[1] - firstPoint[1];
    inflowDirection[2] = pathCenterPoint[2] - firstPoint[2];*/
	int id0 = path->FindPoint(pathCenterPoint);
	int idStart, idEnd;
	if (id0 > 10)
	{
		idStart = id0 - 10;
	}
	else
	{
		idStart = 1;
	}
	if (id0 + 10 < path->GetNumberOfPoints())
	{
		idEnd = id0 + 10;
	}
	else
	{
		idEnd = path->GetNumberOfPoints()-1;
	}
	for (int i = idStart;i < idEnd;i++)
	{
		double ptem[3];
		path->GetPoint(i,ptem);
		inflowDirection[0] = inflowDirection[0] + firstPoint[0] - ptem[0];
		inflowDirection[1] = inflowDirection[1] + firstPoint[1] - ptem[1];
		inflowDirection[2] = inflowDirection[2] + firstPoint[2] - ptem[2];
	}
	inflowDirection[0] = inflowDirection[0] / (idEnd-idStart);
	inflowDirection[1] = inflowDirection[1] / (idEnd-idStart);
	inflowDirection[2] = inflowDirection[2] / (idEnd-idStart);
    
    double farPoint[3];
    this->aneurysmImageData->GetPoint(farPointIndex, farPoint);
    double inflowAngle = angle(neckCenterPoint, farPoint, inflowDirection) * 180.0 / 3.1415;

	double highPoint[3];
	this->aneurysmImageData->GetPoint(maxHeightIndex, highPoint);
	this->aneurysmData.heightpoint[0] = highPoint[0];
	this->aneurysmData.heightpoint[1] = highPoint[1];
	this->aneurysmData.heightpoint[2] = highPoint[2];

	this->aneurysmData.neckcenterpoint[0] = neckCenterPoint[0];
	this->aneurysmData.neckcenterpoint[1] = neckCenterPoint[1];
	this->aneurysmData.neckcenterpoint[2] = neckCenterPoint[2];

	this->aneurysmData.farpoint[0] = farPoint[0];
	this->aneurysmData.farpoint[1] = farPoint[1];
	this->aneurysmData.farpoint[2] = farPoint[2];
	this->aneurysmData.microtubulePoint[0] = (neckCenterPoint[0] + farPoint[0]) / 2;
	this->aneurysmData.microtubulePoint[1] = (neckCenterPoint[1] + farPoint[1]) / 2;
	this->aneurysmData.microtubulePoint[2] = (neckCenterPoint[2] + farPoint[2]) / 2;
	this->startEndPoints->InsertPoint(1, this->aneurysmData.microtubulePoint);

	this->aneurysmData.pathcenterpoint[0] = pathCenterPoint[0];
	this->aneurysmData.pathcenterpoint[1] = pathCenterPoint[1];
	this->aneurysmData.pathcenterpoint[2] = pathCenterPoint[2];
    
    int npixels = 0;
    for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
    {
        if (aneurysmDataPointer[i] > 0)
        {
            npixels = npixels + 1;
        }
    }
    double spacing[3];
    this->aneurysmImageData->GetSpacing(spacing);
    double volume = npixels * spacing[0] * spacing[1] * spacing[2];
    
    /*
    double heightPoint1[3];
    double heightPoint2[3];
    downSampledAneurysmImageData->GetPoint(heightIndex1, heightPoint1);
    downSampledAneurysmImageData->GetPoint(heightIndex2, heightPoint2);
    
    double midPoint1[3];
    midPoint1[0] = heightPoint1[0] + 2 * radius * direction[0];
    midPoint1[1] = heightPoint1[1] + 2 * radius * direction[1];
    midPoint1[2] = heightPoint1[2] + 2 * radius * direction[2];
    
    double midPoint2[3];
    midPoint2[0] = heightPoint2[0] + radius * direction[0];
    midPoint2[1] = heightPoint2[1] + radius * direction[1];
    midPoint2[2] = heightPoint2[2] + radius * direction[2];
    
    double midPoint[3];
    int midPointSub[3];
    
    double neckRoundPoint[3];
    if (aneurysmDataPointer[this->aneurysmImageData->FindPoint(midPoint1)] == 0) {
        if (aneurysmDataPointer[this->aneurysmImageData->FindPoint(midPoint2)] == 0) {
            id2sub(this->segmentedData->FindPoint(nextPoint), origDims, midPointSub);
            midPoint[0] = nextPoint[0];
            midPoint[1] = nextPoint[1];
            midPoint[2] = nextPoint[2];
        } else {
            id2sub(this->segmentedData->FindPoint(midPoint2), origDims, midPointSub);
            midPoint[0] = midPoint2[0];
            midPoint[1] = midPoint2[1];
            midPoint[2] = midPoint2[2];
        }
    } else {
        id2sub(this->segmentedData->FindPoint(midPoint1), origDims, midPointSub);
        midPoint[0] = midPoint1[0];
        midPoint[1] = midPoint1[1];
        midPoint[2] = midPoint1[2];
    }
    int neckRoundPointSub[3];
    int entryPointSub[3];
    double neckRoundPoint[3];
    id2sub(this->segmentedData->FindPoint(entryPoint), origDims, entryPointSub);
    if (neckRadius != -1)
    {
        for (int x = -30; x < 30; x++)
        {
            int xs = x + entryPointSub[0];
            if (xs < 0) {
                xs = 0;
            }
            if (xs >= origDims[0])
            {
                xs = origDims[0] - 1;
            }
            neckRoundPointSub[0] = xs;
            for (int y = -30; y < 30; y++)
            {
                int ys = y + entryPointSub[1];
                if (ys < 0) {
                    ys = 0;
                }
                if (ys >= origDims[1])
                {
                    ys = origDims[1] - 1;
                }
                neckRoundPointSub[1] = ys;
                for (int z = -30; z < 30; z++)
                {
                    int zs = z + entryPointSub[2];
                    if (zs < 0) {
                        zs = 0;
                    }
                    if (zs >= origDims[2])
                    {
                        zs = origDims[2] - 1;
                    }
                    neckRoundPointSub[2] = zs;
                    if (aneurysmDataPointer[sub2id(xs, ys, zs, origDims)] == 0) {
                        double neckDistance = normDist(entryPointSub, neckRoundPointSub);
                        if (neckRadius > neckDistance)
                        {
                            neckRadius = neckDistance;
                            this->segmentedData->GetPoint(sub2id(xs, ys, zs, origDims), neckRoundPoint);
                        }
                    }
                }
            }
        }
        neckRadius = normDist(entryPoint, neckRoundPoint);
    }
     */
    
    
    
    /*
    vtkSmartPointer<vtkPoints> aneurysmMeasurementPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkUnsignedCharArray> pointColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkSmartPointer<vtkUnsignedCharArray> lineColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    pointColors->SetName("PointColors");
    pointColors->SetNumberOfComponents(3);
    lineColors->SetName("LineColors");
    lineColors->SetNumberOfComponents(3);
    double red[3] = { 255, 0, 0 };
    double green[3] = { 0, 255, 0 };
    
    aneurysmMeasurementPoints->InsertNextPoint(firstPoint);
    pointColors->InsertNextTuple(green);
    
    aneurysmMeasurementPoints->InsertNextPoint(entryPoint);
    pointColors->InsertNextTuple(green);
    
    vtkSmartPointer<vtkLine> distanceLine = vtkSmartPointer<vtkLine>::New();
    distanceLine->GetPointIds()->SetId(0, 0);
    distanceLine->GetPointIds()->SetId(1, 1);
    lineColors->InsertNextTuple(red);
    
    vtkSmartPointer<vtkCellArray> anuerysmMeasurementLines = vtkSmartPointer<vtkCellArray>::New();
    anuerysmMeasurementLines->InsertNextCell(distanceLine);
     */
    
    this->aneurysmData.aneurysmDisplayPolyData = vtkSmartPointer<vtkPolyData>::New();
	this->aneurysmData.aneurysmDisplayPolyData->DeepCopy(djForPath->GetOutput());
	this->aneurysmData.aneurysmDisplayPolyData2 = vtkSmartPointer<vtkPolyData>::New();
	this->aneurysmData.aneurysmDisplayPolyData2->DeepCopy(path);
    
    this->aneurysmData.diameter = farDistance+0.3*radius;
    this->aneurysmData.height = maxHeight+0.3*radius;
    this->aneurysmData.neckDiameter = neckRadius * 2.0;
    this->aneurysmData.inflowAngle = inflowAngle;
    this->aneurysmData.volume = volume;
    
    return this->aneurysmData;
}

vtkSmartPointer<vtkPolyData> ImageAlgorithm::getCurrentSurfaceShowing()
{
	return this->currentSurfaceShowing;
}

void ImageAlgorithm::setCurrentSurfaceShowing(vtkSmartPointer<vtkPolyData> surfaceDataShowing)
{
	this->currentSurfaceShowing = surfaceDataShowing;
}

/* Old code
this->pathspline = vtkSmartPointer<vtkParametricSpline>::New();
this->centerlineactor = vtkSmartPointer<vtkActor>::New();
this->needlelineactor = vtkSmartPointer<vtkActor>::New();
this->needlespline = vtkSmartPointer<vtkParametricSpline>::New();
this->anglefactor = 2.0;

class myActor : public vtkActor
{
public:
vtkTypeMacro(myActor, vtkActor);
static myActor *New();
void setid(int newid) { id = newid; }
int getid() { return id; }
int id;
};

class controlpoint
{
public:
controlpoint();
~controlpoint();
vtkSmartPointer<vtkActor> getactor() const { return actor; };
vtkSmartPointer<vtkActor> getactor() { return actor; };
vtkSmartPointer<vtkSphereSource> getsource() { return source; };
vtkSmartPointer<vtkSphereSource> getsource() const { return source; };
//	void setid(int newid){id = newid;}
//	int getid(){return id;}
//	int getid() const {return id;}
private:
vtkSmartPointer<vtkSphereSource> source;
vtkSmartPointer<vtkActor> actor;
//	int id;
};

controlpoint::controlpoint()
{
source = vtkSmartPointer<vtkSphereSource>::New();
this->source->SetRadius(2.0);
actor = vtkSmartPointer<vtkActor>::New();

vtkSmartPointer<vtkPolyDataMapper> mapper =
vtkSmartPointer<vtkPolyDataMapper>::New();
mapper->SetInputConnection(this->source->GetOutputPort());

this->actor->SetMapper(mapper);
this->actor->GetProperty()->SetColor(NORMALPOINTCOLOR);
//  this->id = 0;
}
controlpoint::~controlpoint()
{

}

// http://stackoverflow.com/questions/4604136/how-to-search-for-an-element-in-an-stl-list
bool operator==(const controlpoint& lhs, const controlpoint& rhs);

void QtVTKRenderWindows::shrinkDataChanged(int value)
{
shrinkRatio = (this->ui->shrinkDataCheckBox->isChecked()) ? 0.5 : 1.0;
resampler->SetAxisMagnificationFactor(0, shrinkRatio);
resampler->SetAxisMagnificationFactor(1, shrinkRatio);
resampler->SetAxisMagnificationFactor(2, shrinkRatio);
}

void QtVTKRenderWindows::InsertPoint()
{
std::list<controlpoint>::iterator it = this->controlpoints.begin();
setcontrolpointcolor(this->selectedpoint,NORMALPOINTCOLOR);
advance (it,this->selectedpoint);
controlpoint a;
double x[3];
riw[0]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetResliceCursor()->GetCenter(x);
cout<<"center: "<<x[0]<<" "<<x[1]<<" "<<x[2]<<endl;
a.getsource()->SetCenter(x[0],x[1],x[2]);
//	a.setid(this->controlpoints.size()+1);

this->controlpoints.insert(it, a);
putcontrolpointinview(a);
addpointtostringlist();
GenerateCurve();
if (this->controlpoints.size()>0)	this->ui->ModelNeedleButton->setEnabled(true);
}

void QtVTKRenderWindows::deletepointfromstringlist()
{
stringList.removeAt(stringList.length() - 1);
qtpointsmodel->setStringList(this->stringList);
}

void QtVTKRenderWindows::DeletePoint()
{
std::list<controlpoint>::iterator it = this->controlpoints.begin();
advance (it,this->selectedpoint);
removecontrolpointinview(*it);
this->controlpoints.erase(it);
deletepointfromstringlist();
GenerateCurve();
this->selectedpoint = 0;
if (this->controlpoints.size()==0)
{
this->ui->ModelNeedleButton->setDisabled(true);
this->ui->InsertPointButton->setDisabled(true);
this->ui->DeletePointButton->setDisabled(true);
}
}

void QtVTKRenderWindows::pointselected(const QModelIndex &index)
{
int row = index.row();
int column = index.column();
QString text = QString("point %1 is selected").arg(row);
this->ui->pointlabel->setText(text);
setcontrolpointcolor(this->selectedpoint,NORMALPOINTCOLOR);
this->selectedpoint = row;
setcontrolpointcolor(this->selectedpoint,SELECTEDPOINTCOLOR);
}

void QtVTKRenderWindows::setcontrolpointcolor(int index, double c1, double c2, double c3)
{
std::list<controlpoint>::iterator it = this->controlpoints.begin();
advance(it, index);
it->getactor()->GetProperty()->SetColor(c1, c2, c3);
this->ui->view4->GetRenderWindow()->Render();
}

vtkSmartPointer<vtkGeometryFilter> generatepolyfrompoints(vtkSmartPointer<vtkPolyData> ps)
{
vtkSmartPointer<vtkDelaunay3D> delaunay3D = vtkSmartPointer<vtkDelaunay3D>::New();
delaunay3D->SetInputData(ps);
vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
geometryFilter->SetInputConnection(delaunay3D->GetOutputPort());
delaunay3D->Update();
geometryFilter->Update();
return geometryFilter;
}

void samplespline(vtkSmartPointer<vtkPoints> pp, const vtkSmartPointer<vtkParametricSpline> spline, int nstep)
{
if (spline->GetPoints()->GetNumberOfPoints() == 0)
{
QMessageBox msgBox;
msgBox.setText("No points in path");
msgBox.setStandardButtons(QMessageBox::Ok);
int ret = msgBox.exec();
return;
}
double dt = 1 / (double)nstep;
double t = 0;
// nstep + 1 points
for (int i = 0; i <= nstep; i++)
{
double u[3];
double Pt[3];
double Du[9];

u[0] = t;
spline->Evaluate(u, Pt, Du);
pp->InsertNextPoint(Pt);
t = t + dt;
}
}

vtkSmartPointer<vtkPolyDataNormals> smoothpoly(vtkSmartPointer<vtkPolyDataAlgorithm> poly)
{
vtkSmartPointer<vtkSmoothPolyDataFilter> smoothFilter =
vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
smoothFilter->SetInputConnection(poly->GetOutputPort());
smoothFilter->SetNumberOfIterations(15);
smoothFilter->SetRelaxationFactor(0.1);
smoothFilter->FeatureEdgeSmoothingOff();
smoothFilter->BoundarySmoothingOn();
smoothFilter->Update();

// Update normals on newly smoothed polydata
vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
normalGenerator->SetInputConnection(smoothFilter->GetOutputPort());
normalGenerator->ComputePointNormalsOn();
normalGenerator->ComputeCellNormalsOn();
normalGenerator->Update();
return normalGenerator;
}

vtkSmartPointer<vtkTransformPolyDataFilter> transformpoly(const double x[3], const double y[3], const double z[3], const double p0[3], vtkSmartPointer<vtkPolyDataAlgorithm> poly)
{
vtkSmartPointer<vtkTransform > transform = vtkSmartPointer<vtkTransform>::New();
double matrix[16];
matrix[0] = x[0]; matrix[1] = y[0];	matrix[2] = z[0];	matrix[3] = p0[0];
matrix[4] = x[1]; matrix[5] = y[1];	matrix[6] = z[1];	matrix[7] = p0[1];
matrix[8] = x[2]; matrix[9] = y[2];	matrix[10] = z[2];	matrix[11] = p0[2];
matrix[12] = 0;		 matrix[13] = 0;	matrix[14] = 0;		matrix[15] = 1;
transform->SetMatrix(matrix);
vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
transformFilter->SetInputConnection(poly->GetOutputPort());
transformFilter->SetTransform(transform);
return transformFilter;
}

vtkSmartPointer<vtkTriangleFilter> gpentagontube(const double x[3], const double y[3], const double z[3], const double p0[3], const double lengthx)
{
vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
points->InsertNextPoint(-0.1 * lengthx, 2, 5);
points->InsertNextPoint(-0.1 * lengthx, 0, -2);
points->InsertNextPoint(-0.1 * lengthx, -2, 0);

points->InsertNextPoint(-0.1 * lengthx, 2, 0);
points->InsertNextPoint(-0.1 * lengthx, -2, 5);

points->InsertNextPoint(1.1 * lengthx, 2, 5);
points->InsertNextPoint(1.1 * lengthx, 0, -2);
points->InsertNextPoint(1.1 * lengthx, -2, 0);
points->InsertNextPoint(1.1 * lengthx, 2, 0);
points->InsertNextPoint(1.1 * lengthx, -2, 5);

vtkSmartPointer<vtkPolyData> ps = vtkSmartPointer<vtkPolyData>::New();
ps->SetPoints(points);

vtkSmartPointer<vtkGeometryFilter> geometryFilter = generatepolyfrompoints(ps);
vtkSmartPointer<vtkTransformPolyDataFilter > transformFilter = transformpoly(x, y, z, p0, geometryFilter);
vtkSmartPointer<vtkTriangleFilter> tube = vtkSmartPointer<vtkTriangleFilter>::New();
tube->SetInputConnection(transformFilter->GetOutputPort());
tube->Update();
return tube;
}

vtkSmartPointer<vtkTriangleFilter> gonehalfcube(const double x[3], const double y[3], const double z[3], const double p0[3], const double lengthx)
{
vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
double xmin = -0.5*lengthx;
double xmax = lengthx * 1.5;
double r = 0;
cubeSource->SetBounds(xmin - r, xmax + r,
-3 + r, 3 + r,
-5 + r, 0 + r);
vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = transformpoly(x, y, z, p0, cubeSource);
vtkSmartPointer<vtkCleanPolyData> clear = vtkCleanPolyData::New();
clear->SetInputConnection(transformFilter->GetOutputPort());
vtkSmartPointer<vtkTriangleFilter> triangleFilter =
vtkSmartPointer<vtkTriangleFilter>::New();
triangleFilter->SetInputConnection(clear->GetOutputPort());
triangleFilter->Update();
return triangleFilter;
}

void ghc(const vtkSmartPointer<vtkPoints> pp, vtkSmartPointer<vtkTriangleFilter>* cs,
vtkSmartPointer<vtkTriangleFilter>(*ba)(const double[], const double[], const double[], const double[], const double))
{
double f1x[3];
double f1y[3];
double f1z[3];
double f2x[3];
double f2y[3];
double f2z[3];

double p0[3];
double p1[3];
double p2[3];

double v0[3];
double v1[3];

pp->GetPoint(0, p0);
pp->GetPoint(1, p1);
pp->GetPoint(2, p2);
vfp(v0, p1, p0);
vfp(v1, p2, p1);
vn(v0);
vn(v1);
gf(f1x, f1y, f1z, v0, v1);
vfp(v0, p1, p0);
cs[0] = ba(f1x, f1y, f1z, p0, vtkMath::Norm(v0));

for (int i = 1; i < pp->GetNumberOfPoints() - 2; i++)
{
pp->GetPoint(i, p0);
pp->GetPoint(i + 1, p1);
pp->GetPoint(i + 2, p2);
vfp(v0, p1, p0);
vfp(v1, p2, p1);
vn(v0);
vn(v1);
gf(f2x, f2y, f2z, v0, v1);
fc(f2x, f2y, f2z, f1x, f1y, f1z);
vfp(v0, p1, p0);
cs[i] = ba(f2x, f2y, f2z, p0, vtkMath::Norm(v0));
ve(f1x, f2x);
ve(f1y, f2y);
ve(f1z, f2z);
}
}

void setimagevalue(vtkSmartPointer<vtkImageData> whiteImage, int val)
{
vtkIdType count = whiteImage->GetNumberOfPoints();
unsigned char *p = (unsigned char *)whiteImage->GetPointData()->GetScalars()->GetVoidPointer(0);
for (vtkIdType i = 0; i < count; ++i)
{
p[i] = val;
}
}

void initializeimage(vtkSmartPointer<vtkImageData> whiteImage, const vtkSmartPointer<vtkImageData> inImage, int initialvalue)
{
whiteImage->SetSpacing(inImage->GetSpacing());
whiteImage->SetDimensions(inImage->GetDimensions());
whiteImage->SetExtent(inImage->GetExtent());
whiteImage->SetOrigin(inImage->GetOrigin());
//whiteImage->SetScalarTypeToUnsignedChar();
//whiteImage->AllocateScalars();
whiteImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
setimagevalue(whiteImage, initialvalue);
}

vtkSmartPointer<vtkImageData> polydatatoimage(vtkSmartPointer<vtkTriangleFilter> polycube, const vtkSmartPointer<vtkImageData> standardImage)
{
vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();
initializeimage(whiteImage, standardImage, 255);
vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
pol2stenc->SetInputData(polycube->GetOutput());
pol2stenc->SetOutputOrigin(whiteImage->GetOrigin());
pol2stenc->SetOutputSpacing(whiteImage->GetSpacing());
pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
pol2stenc->Update();
vtkSmartPointer<vtkImageStencil> imgstenc =
vtkSmartPointer<vtkImageStencil>::New();
imgstenc->SetInputData(whiteImage);
imgstenc->SetStencilData(pol2stenc->GetOutput());
imgstenc->ReverseStencilOff();
imgstenc->SetBackgroundValue(0);
imgstenc->Update();
vtkSmartPointer<vtkImageData> cubeImage = imgstenc->GetOutput();
return cubeImage;
}

void imageor(vtkSmartPointer<vtkImageData> outImage, vtkSmartPointer<vtkImageData> inImage)
{
vtkIdType count = outImage->GetNumberOfPoints();
unsigned char *in1 = (unsigned char *)outImage->GetPointData()->GetScalars()->GetVoidPointer(0);
unsigned char *in2 = (unsigned char *)inImage->GetPointData()->GetScalars()->GetVoidPointer(0);

for (vtkIdType i = 0; i < count; ++i)
{
int out = 255;
if ((in1[i] == 0) && (in2[i] == 0))
{
out = 0;
}
in1[i] = out;
}
}

void imagedifference(vtkSmartPointer<vtkImageData> outImage, vtkSmartPointer<vtkImageData> inImage)
{
vtkIdType count = outImage->GetNumberOfPoints();
unsigned char *in1 = (unsigned char *)outImage->GetPointData()->GetScalars()->GetVoidPointer(0);
unsigned char *in2 = (unsigned char *)inImage->GetPointData()->GetScalars()->GetVoidPointer(0);

for (vtkIdType i = 0; i < count; ++i)
{
int out = 255;
if ((in1[i] > 0) && (in2[i] > 0))
{
out = 0;
in1[i] = out;
}
}
}

void QtVTKRenderWindows::getmi(vtkSmartPointer<vtkImageData> mi, vtkSmartPointer<vtkPoints> pp, vtkSmartPointer<vtkImageData> standardImage, vtkSmartPointer<vtkRenderer> ren)
{
initializeimage(mi, standardImage, 0);
vtkSmartPointer<vtkTriangleFilter> *hc = new vtkSmartPointer<vtkTriangleFilter>[pp->GetNumberOfPoints()];
ghc(pp, hc, gonehalfcube);
for (int i = 0; i < pp->GetNumberOfPoints() - 2; i++)
{
vtkSmartPointer<vtkImageData> hci = polydatatoimage(hc[i], standardImage);
imageor(mi, hci);
}
vtkSmartPointer<vtkTriangleFilter> *halftubes = new vtkSmartPointer<vtkTriangleFilter>[pp->GetNumberOfPoints()];
ghc(pp, halftubes, gpentagontube);
vtkSmartPointer<vtkImageData> tubeImage = vtkSmartPointer<vtkImageData>::New();
initializeimage(tubeImage, standardImage, 0);
for (int i = 0; i < pp->GetNumberOfPoints() - 2; i++)
{
vtkSmartPointer<vtkImageData> onetubeImage = polydatatoimage(halftubes[i], standardImage);
imageor(tubeImage, onetubeImage);
}
imagedifference(mi, tubeImage);
}

void QtVTKRenderWindows::GenerateMouldNewNew()
{
bool ok = false;
int nstep = QInputDialog::getInt(this, tr("Set # of steps, larger slower"),
tr("nsteps"), 30, 3, 1000, 1, &ok);
if (ok)
{
vtkSmartPointer<vtkPoints> pp = vtkSmartPointer<vtkPoints>::New();
samplespline(pp, this->needlespline, nstep);

vtkSmartPointer<vtkImageData> mi = vtkSmartPointer<vtkImageData>::New();
getmi(mi, pp, vtkImageData::SafeDownCast(this->resampler->GetOutput()), planeWidget[0]->GetDefaultRenderer());
double values[2];
values[0] = 256;
values[1] = 1;
this->mouldactor = visualizeImage(mouldsurface, mi, values, planeWidget[0]->GetDefaultRenderer());
this->ui->mouldvisibilitybox->setCheckState(Qt::Checked);
this->mouldactor->VisibilityOn();
}
}

void QtVTKRenderWindows::ModelNeedleNewNewNew()
{
int nstep = 100;
bool ok = false;
this->anglefactor = QInputDialog::getDouble(this, tr("Set angle scaling factor"),
tr(""), this->anglefactor, 0, 5.0, 1, &ok);
if (ok)
{
vtkSmartPointer<vtkPoints> pp = vtkSmartPointer<vtkPoints>::New();
samplespline(pp, this->pathspline, nstep);

//	visualizepoints(pp);
double temp0[3];
vtkSmartPointer<vtkPoints> needlepoints = vtkSmartPointer<vtkPoints>::New();
pp->GetPoint(0, temp0);
needlepoints->InsertNextPoint(temp0);
double temp1[3];
pp->GetPoint(1, temp1);
vtkSmartPointer<vtkPoints> axe = vtkSmartPointer<vtkPoints>::New();
vtkSmartPointer<vtkDoubleArray> ang = vtkSmartPointer<vtkDoubleArray>::New();
cnea(pp, axe, ang);
ae(needlepoints, pp, axe, ang, anglefactor);

//	visualizepoints(needlepoints);
this->needlespline->SetPoints(needlepoints);
vtkSmartPointer<vtkParametricFunctionSource> functionSource =
vtkSmartPointer<vtkParametricFunctionSource>::New();
functionSource->SetParametricFunction(this->needlespline);
functionSource->Update();

// Setup actor and mapper
vtkSmartPointer<vtkPolyDataMapper> mapper =
vtkSmartPointer<vtkPolyDataMapper>::New();
mapper->SetInputConnection(functionSource->GetOutputPort());
this->needlelineactor->SetMapper(mapper);
this->needlelineactor->GetProperty()->SetColor(0, 1, 0);
planeWidget[0]->GetDefaultRenderer()->AddActor(this->needlelineactor);
this->ui->view4->GetRenderWindow()->Render();
this->needlelineactor->VisibilityOn();
this->ui->needlevisilibtybox->setCheckState(Qt::Checked);
}
}

void QtVTKRenderWindows::GenerateCurve()
{
if (controlpoints.size() == 0)
{
QMessageBox msgBox;
msgBox.setText("No points in path");
msgBox.setStandardButtons(QMessageBox::Ok);
int ret = msgBox.exec();
return;
}
vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
std::list<controlpoint>::const_iterator iterator;
for (iterator = controlpoints.begin(); iterator != controlpoints.end(); ++iterator)
{
double c[3];
iterator->getsource()->GetCenter(c);
points->InsertNextPoint(c);
}
this->pathspline->SetPoints(points);

vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
functionSource->SetParametricFunction(this->pathspline);
functionSource->Update();

vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
mapper->SetInputConnection(functionSource->GetOutputPort());
this->centerlineactor->SetMapper(mapper);
planeWidget[0]->GetDefaultRenderer()->AddActor(this->centerlineactor);
this->ui->view4->GetRenderWindow()->Render();
this->ui->tubepathvisbilitybox->setCheckState(Qt::Checked);
}

void QtVTKRenderWindows::removecontrolpointinview(controlpoint& a)
{
planeWidget[0]->GetDefaultRenderer()->RemoveActor(a.getactor());
this->ui->view4->GetRenderWindow()->Render();
}

void QtVTKRenderWindows::putcontrolpointinview(controlpoint& a)
{
planeWidget[0]->GetDefaultRenderer()->AddActor(a.getactor());
this->ui->view4->GetRenderWindow()->Render();
}

void QtVTKRenderWindows::addpointtostringlist()
{
QString str = "point ";
str.append(std::to_string((_ULonglong)(stringList.length())).c_str());
stringList.append(str);
qtpointsmodel->setStringList(this->stringList);
}

void QtVTKRenderWindows::AddControlPoint()
{
controlpoint a;
double x[3];
riw[0]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetResliceCursor()->GetCenter(x);
cout<<"center: "<<x[0]<<" "<<x[1]<<" "<<x[2]<<endl;
a.getsource()->SetCenter(x[0],x[1],x[2]);

this->controlpoints.push_back(a);
putcontrolpointinview(a);
addpointtostringlist();
GenerateCurve();
this->ui->view4->GetRenderWindow()->Render();
if (this->controlpoints.size()>0)	{
this->ui->ModelNeedleButton->setEnabled(true);
this->ui->InsertPointButton->setEnabled(true);
this->ui->DeletePointButton->setEnabled(true);
}
}

void QtVTKRenderWindows::needlpathvisbilitychanged(int state)
{
if (this->ui->needlevisilibtybox->isChecked())
{
this->needlelineactor->VisibilityOn();
}
else
{
this->needlelineactor->VisibilityOff();
}
this->ui->view4->GetRenderWindow()->Render();
}

void QtVTKRenderWindows::mouldvisbilitychanged(int state)
{
if (this->mouldactor)
{
if (this->ui->mouldvisibilitybox->isChecked())
{
mouldactor->VisibilityOn();
}
else {
mouldactor->VisibilityOff();
}
this->ui->view4->GetRenderWindow()->Render();
}
}

void initializeViews()
{
//this->qtpointsmodel = new QStringListModel(this->stringList);

ui->PointsView->setModel(this->qtpointsmodel);
ui->PointsView->show();

QString text = QString("point %1 selected").arg(this->selectedpoint);
this->ui->pointlabel->setText(text);
this->ui->ControlPointsGroup->setDisabled(true);
this->ui->groupBox->setDisabled(true);
this->ui->NeedleGroup->setDisabled(true);
}

void QtVTKRenderWindows::showcontrolpoints()
{
for (std::list<controlpoint>::iterator it = this->controlpoints.begin(); it != this->controlpoints.end(); ++it)
{
it->getactor()->VisibilityOn();
}
}

void QtVTKRenderWindows::hidecontrolpoints()
{
for (std::list<controlpoint>::iterator it = this->controlpoints.begin(); it != this->controlpoints.end(); ++it)
{
it->getactor()->VisibilityOff();
}
}

vtkSmartPointer<vtkActor> visualizeImage(vtkSmartPointer<vtkMarchingCubes> surface, vtkSmartPointer<vtkImageData> image, double value[2], vtkSmartPointer< vtkRenderer> ren)
{
surface->SetInputData(image);
surface->ComputeNormalsOn();
surface->SetValue(0, value[0]);
surface->SetValue(1, value[1]);
vtkSmartPointer<vtkPolyDataMapper> surfacemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
vtkSmartPointer<vtkPolyDataNormals> smoothed = smoothpoly(surface);
surfacemapper->SetInputConnection(smoothed->GetOutputPort());
surfacemapper->ScalarVisibilityOff();
vtkSmartPointer<vtkActor> surfaceactor = vtkSmartPointer<vtkActor>::New();
surfaceactor->SetMapper(surfacemapper);
surfaceactor->GetProperty()->SetColor(1, 1, 0);
surfaceactor->GetProperty()->SetOpacity(1);
surfaceactor->VisibilityOn();
ren->AddActor(surfaceactor);
ren->Render();
return surfaceactor;
}

void QtVTKRenderWindows::visualizepoly(vtkSmartPointer<vtkPolyDataAlgorithm> poly)
{
vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
Mapper->SetInputConnection(poly->GetOutputPort());
Mapper->ScalarVisibilityOff();
vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
actor->SetMapper(Mapper);
planeWidget[0]->GetDefaultRenderer()->AddActor(actor);
this->ui->view4->GetRenderWindow()->Render();
}

void QtVTKRenderWindows::visualizepoints(vtkSmartPointer<vtkPoints> points)
{
vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
for (int i = 0; i < points->GetNumberOfPoints(); i++)
{
polyLine->GetPointIds()->SetId(i, i);
}

vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
cells->InsertNextCell(polyLine);

vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

polyData->SetPoints(points);

polyData->SetLines(cells);

vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
mapper->SetInputData(polyData);

vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
actor->SetMapper(mapper);
actor->GetProperty()->SetColor(0, 1, 0);
actor->GetProperty()->SetPointSize(3);
planeWidget[0]->GetDefaultRenderer()->AddActor(actor);
this->ui->view4->GetRenderWindow()->Render();
}

void QtVTKRenderWindows::SaveMould()
{
saveSurface(this->mouldsurface, "mould.stl");
}
 
 
 calculateVesselness();
 vtkSmartPointer<vtkImageData> enhancedVesselData = vtkSmartPointer<vtkImageData>::New();
 enhancedVesselData->DeepCopy(this->resampler->GetOutput());
 
 int* dims = this->resampler->GetOutput()->GetDimensions();
 
 float* vesselnessPointer = static_cast<float*>(this->vesselness->GetScalarPointer());
 float* enhancedVesselDataPointer = static_cast<float*>(enhancedVesselData->GetScalarPointer());
 for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
 {
 if (vesselnessPointer[i] < 0.2)
 {
 enhancedVesselDataPointer[i] = this->valuesRange[0] - 1000;
 }
 }
 
 vtkSmartPointer<vtkImageData> segmentedEnhancedData = vtkSmartPointer<vtkImageData>::New();
 // Feeds from threshold if no seeds are selected
 if (this->seedPoints->GetNumberOfPoints() == 0)
 {
 vtkSmartPointer<vtkImageThreshold> simpleThreshold = vtkSmartPointer<vtkImageThreshold>::New();
 simpleThreshold->ReplaceInOn();
 simpleThreshold->SetInValue(1);
 simpleThreshold->ReplaceOutOn();
 simpleThreshold->SetOutValue(0);
 simpleThreshold->SetInputData(enhancedVesselData);
 simpleThreshold->ThresholdBetween(this->lowerth, this->upperth);
 simpleThreshold->Update();
 segmentedEnhancedData = simpleThreshold->GetOutput();
 }
 else
 {
 vtkSmartPointer<vtkImageThresholdConnectivity> connectedThreshold = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
 connectedThreshold->ReplaceInOn();
 connectedThreshold->SetInValue(1);
 connectedThreshold->ReplaceOutOn();
 connectedThreshold->SetOutValue(0);
 connectedThreshold->SetNeighborhoodRadius(1, 1, 1);
 connectedThreshold->SetInputData(enhancedVesselData);
 connectedThreshold->SetSeedPoints(this->seedPoints);
 connectedThreshold->ThresholdBetween(this->lowerth, this->upperth);
 connectedThreshold->Update();
 segmentedEnhancedData = connectedThreshold->GetOutput();
 }
 
 
 float* segmentedEnhancedDataPointer = static_cast<float*>(segmentedEnhancedData->GetScalarPointer());
 
 for (int i = 0; i < origDims[0] * origDims[1] * origDims[2]; i++)
 {
 anuerysmDataPointer[i] = 0;
 if (segmentedDataPointer[i] > 0 && segmentedEnhancedDataPointer[i] == 0)
 {
 anuerysmDataPointer[i] = 1;
 }
 }
 */

