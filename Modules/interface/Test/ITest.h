#pragma once


const char QF_INTERFACE_TEST[] = "QF_INTERFACE_TEST";

class ITest
{
public:
    virtual int Add(int iA, int iB) = 0;

};