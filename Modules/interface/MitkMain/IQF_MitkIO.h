#ifndef IQF_MitkIO_h__
#define IQF_MitkIO_h__

#pragma once

const char QF_MitkMain_IO[] = "QF_MitkMain_IO";

class IQF_MitkIO
{
public:
    virtual bool Load(const char* filename) = 0;
    virtual void LoadFiles() = 0;
};

#endif // IQF_MitkIO_h__
