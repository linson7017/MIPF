#pragma once


const char QF_INTERFACE_TESTALGORITHM[] = "QF_INTERFACE_TESTALGORITHM";

class IQF_TestAlgorithm
{
public:
     /*********************************************************************************
       *Function:    Add
       * Description£ºadd
       *Input:  
       *Output:  
       *Return:  
       *Update Record:
     **********************************************************************************/
    virtual int Add(int iX, int iY) = 0;
};