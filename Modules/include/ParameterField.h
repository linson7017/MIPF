#ifndef ParameterField_h__
#define ParameterField_h__

#include "Parameter.h"

class ParameterField
{
public:
    ParameterField() {}
    ~ParameterField() {}
    inline Parameter GetParameter(const char* parameterID);

    inline bool AddIntParameter(const char* parameterID, int value);
    inline bool AddDoubleParameter(const char* parameterID, double value);
    inline bool AddBoolParameter(const char* parameterID, bool value);
    inline bool AddStringParameter(const char* parameterID, const char* value);
    inline bool AddPtrParameter(const char* parameterID, void* value);
private:
    ParameterMap _parameters;
};

inline Parameter ParameterField::GetParameter(const char* parameterID)
{
    return Parameter::GetParameter(_parameters, parameterID);
}

inline bool ParameterField::AddIntParameter(const char* parameterID, int value)
{
    Parameter p;
    p.setInt(value);
    _parameters[parameterID] = p;
    return  true;
}

inline bool ParameterField::AddDoubleParameter(const char* parameterID, double value)
{
    Parameter p;
    p.setInt(value);
    _parameters[parameterID] = p;
    return  true;
}

inline bool ParameterField::AddBoolParameter(const char* parameterID, bool value)
{
    Parameter p;
    p.setBool(value);
    _parameters[parameterID] = p;
    return  true;
}

inline bool ParameterField::AddStringParameter(const char* parameterID, const char* value)
{
    Parameter p;
    p.setString(value);
    _parameters[parameterID] = p;
    return  true;
}

inline bool ParameterField::AddPtrParameter(const char* parameterID, void* value)
{
    Parameter p;
    p.setPtr(value);
    _parameters[parameterID] = p;
    return  true;
}

#endif // ParameterField_h__
