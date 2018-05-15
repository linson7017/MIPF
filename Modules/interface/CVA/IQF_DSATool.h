#ifndef IQF_DSATool_h__
#define IQF_DSATool_h__
/********************************************************************
	FileName:    IQF_DSATool
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "IQF_Object.h"
#include "mitkDataNode.h"

#define  QF_INTERFACE_DSA_TOOL  "QF_INTERFACE_DSA_TOOL"
class IQF_DSATool :public IQF_Object
{
public:
    virtual mitk::DataNode::Pointer LoadDSADicomFile(const char* szFileName, const char* szNodeName="") = 0;
    virtual bool SaveDSADicomFile(mitk::Image* pImage, const char* szFileName,std::map<std::string,std::string>& dictionary= std::map<std::string, std::string>()) = 0;
};

#endif // IQF_DSATool_h__
