/********************************************************************
	FileName:    MitkProjectIO.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef MitkProjectIO_h__
#define MitkProjectIO_h__


#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"

#include <Poco/Zip/ZipLocalFileHeader.h>
class TiXmlElement;

namespace mitk
{
    class BaseData;
    class PropertyList;
}

class MitkProjectIO : public itk::Object
{
public:
    mitkClassMacroItkParent(MitkProjectIO, itk::Object);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
        typedef mitk::DataStorage::SetOfObjects FailedBaseDataListType;

    virtual mitk::DataStorage::Pointer LoadScene(const std::string &filename,
        mitk::DataStorage *storage = NULL,
        bool clearStorageFirst = false);

    virtual bool SaveScene(mitk::DataStorage::SetOfObjects::ConstPointer sceneNodes,
        const mitk::DataStorage *storage,
        const std::string &filename);

    const FailedBaseDataListType *GetFailedNodes();

    const mitk::PropertyList *GetFailedProperties();

    typedef mitk::DataStorage::SetOfObjects FailedBaseDataListType;

protected:
    MitkProjectIO();
    virtual ~MitkProjectIO();

    std::string CreateEmptyTempDirectory();

    TiXmlElement *SaveBaseData(mitk::BaseData *data, const std::string &filenamehint, bool &error);
    TiXmlElement *SavePropertyList(mitk::PropertyList *propertyList, const std::string &filenamehint);

    void OnUnzipError(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> &info);
    void OnUnzipOk(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> &info);

    FailedBaseDataListType::Pointer m_FailedNodes;
    mitk::PropertyList::Pointer m_FailedProperties;

    std::string m_WorkingDirectory;
    unsigned int m_UnzipErrors;
};



#endif // MitkProjectIO_h__