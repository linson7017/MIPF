#ifndef CQF_DSATool_h__
#define CQF_DSATool_h__
/********************************************************************
	FileName:    CQF_DSATool
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "CVA/IQF_DSATool.h"

#pragma once
class CQF_DSATool :public IQF_DSATool
{
public:
    CQF_DSATool();
    ~CQF_DSATool();

    //interface 
    virtual mitk::DataNode::Pointer LoadDSADicomFile(const char* szFileName, const char* szNodeName = "");
    virtual bool SaveDSADicomFile(mitk::Image* pImage, const char* szFileName, std::map<std::string, std::string>& dictionary = std::map<std::string, std::string>());
    virtual void Release() { delete this; }

};

#endif // CQF_DSATool_h__
