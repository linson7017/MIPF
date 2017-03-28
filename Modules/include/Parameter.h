/*!
 * \file Parameter.h
 *
 * \author 宋凌
 * \date 五月 2016
 *
 * 
 */
#ifndef Parameter_H
#define Parameter_H

#pragma once
#include <string>
#include <map>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

class Parameter;

template<typename T> inline T Parameter_cast(const Parameter&);
typedef std::map<std::string, Parameter> ParameterMap;
/*!
 * \class Parameter
 *
 * \brief 
 *  参照QParameter的一个轻量级的变量类,当前主要用在config文件中属性信息的存储
 *  暂时只支持int，bool，string，double，指针类型变量，例：
 *  Parameter v(3.14);
 *  double val = v.value<double>();或者double val = v.getDouble();
 *
 * \author 宋凌
 * \date 五月 2016
 */
class Parameter{
public:
	//变量类型
	enum Type{
		Int,
		Bool,
		String,
		Double,
		Ptr,
		UnKnown
	};
	//构造函数
	//参数：无
	//返回值：无
    Parameter() {}
	//构造函数
	//参数：
	//	val: int型输入值
	//返回值：无
	Parameter(int val)
	{
		setInt(val);
	}
	//构造函数
	//参数：
	//	val: bool型输入值
	//返回值：无
	Parameter(bool val)
	{
		setBool(val);
	}
	//构造函数
	//参数：
	//	val: 字符型输入值
	//返回值：无
	Parameter(const char* val)
	{
		setString(val);
	}
	//构造函数
	//参数：
	//	val: double型输入值
	//返回值：无
	Parameter(double val)
	{
		setDouble(val);
	}
	//构造函数
	//参数：
	//	val: 指针型输入值
	//返回值：无
	Parameter(void* val)
	{
		setPtr(val);
	}
	//析构函数
	//参数：无
	//返回值：无
    ~Parameter() {}
	//获得变量类型
	//参数：无
	//返回值：变量类型
	Type getType() const
	{
		return _type;
	}
	//设置变量类型
	//参数：
	//	type: 变量类型
	//返回值：无
	void setType(Type type){_type = type;}
	//设置变量值
	//参数：
	//	val: int型变量
	//返回值：无
	void setInt(int val)
	{
		_data.iValue=val;
		char temp[50];
		sprintf(temp,"%d",val);
		_content = temp;
		_type=Int;
	}
	//设置变量值
	//参数：
	//	val: bool型变量
	//返回值：无
	void setBool(bool val)
	{
		_data.bValue=val;
		if (val)
		{
			_content = "true";
		}else
		{
			_content = "false";
		}
		_type = Bool;
	}
	//设置变量值
	//参数：
	//	val: 字符型变量
	//返回值：无
	void setString(const char* val)
	{
		_sdata = val;
		_content = _sdata;
		_type = String;
	}
	//设置变量值
	//参数：
	//	val: double型变量
	//返回值：无
	void setDouble(double val)
	{
		_data.dValue=val;
		char temp[50];
		sprintf(temp,"%lf",val);
		_content = temp;
		_type = Double;
	}
	//设置变量值
	//参数：
	//	val: 指针型变量
	//返回值：无
	void setPtr(void* val)
	{
		_data.pValue=val;
		_type = Ptr;
	}
	//获得变量值
	//参数：无
	//返回值：int型变量值
	int getInt() const
	{
		if (_type==Int)
		{
			return _data.iValue;
		}
		else
			return -1111111;
	}
	//获得变量值
	//参数：无
	//返回值：bool型变量值
	bool getBool() const
	{
		if (_type==Bool)
		{
			return _data.bValue;
		}
		else
			return false;
	}
	//获得变量值
	//参数：无
	//返回值：double型变量值
	double getDouble() const
	{
		if (_type==Double)
		{
			return _data.dValue;
		}
		else
			return -11111111;
	}
	//获得变量值
	//参数：无
	//返回值：字符型变量值
	const char* getString() const
	{
		if (_type==String)
		{
			return _sdata.c_str();
		}
		else
			return "";
	}
	//获得变量内容
	//参数：无
	//返回值：变量内容
	const char* getContent() const
	{
		return _content.c_str();
	}
	//设置变量内容
	//参数：
	//		conten：变量内容
	//返回值：无
	void setContent(const char* content)
	{
		_content = content;
	}
	//获得变量值
	//参数：无
	//返回值：指针型变量值
	void* getPtr() const
	{
		if (_type==Ptr)
		{
			return _data.pValue;
		}
		else
			return NULL;
	}
	//返回模板参数
	//C++要求模板函数的声明和实现对引用者必须都可见，所以需要放在同一个文件里面或者包含实现的文件
    template<typename T> T value() const
    {
        return Parameter_cast<T>(*this);
    }
    inline static Parameter& GetParameter(ParameterMap& map, const char* id)
    {
        ParameterMap::iterator it = map.find(id);
        if (it != map.end())
        {
            return it->second;
        }
        else
        {
            return Parameter(NULL);
        }
    }
private:
    friend inline bool Parameter_cast_helper(const Parameter&, Parameter::Type, void *);
	union{
		int iValue;
		bool bValue;
		double dValue;
		void* pValue;
	}_data;
	string _sdata;
	string _content;
	Type _type;
};


inline bool Parameter_cast_helper(const Parameter &v, Parameter::Type tp, void *ptr)  //类型转换
{	
	if (tp==Parameter::Int)
	{
		int* pInt = (int*)ptr;
		*pInt = v._data.iValue;
		return true;
	}
	else if (tp==Parameter::Double)
	{
		double* pDouble = (double*)ptr;
		*pDouble = v._data.dValue;
		return true;
	}
	else if (tp==Parameter::Bool)
	{
		bool* pBool = (bool*)ptr;
		*pBool = v._data.bValue;
		return true;
	}
	else if (tp==Parameter::String)
	{
		char** pChar = (char**)ptr;
		*pChar = (char*)v._sdata.c_str();
		return true;
	}
	else if (tp==Parameter::Ptr)
	{
		void** pValue = (void**)ptr;
		*pValue = (char*)v._data.pValue;
		return true;
	}
	else
		return false;
}

template<typename T> T Parameter_cast(const Parameter &v)
{
	Parameter::Type type = v.getType();	 //const类型指针只能调用const类型函数
	if (type < (int)Parameter::UnKnown)    //如果是存在的类型就转换并返回
	{
		T t ;
		if (Parameter_cast_helper(v,type,&t))
			return t;
	}
	return T();
}

#endif