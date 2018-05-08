#pragma once
#include "Test/ITest.h"

class Test :public ITest
{
public:

    virtual int Add(int iA, int iB) { return iA + iB; }
    Test();
    ~Test();
};

