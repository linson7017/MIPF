/*!
 * \file Parameter.h
 *
 * \author ����
 * \date ���� 2016
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
 *  ����QParameter��һ���������ı�����,��ǰ��Ҫ����config�ļ���������Ϣ�Ĵ洢
 *  ��ʱֻ֧��int��bool��string��double��ָ�����ͱ���������
 *  Parameter v(3.14);
 *  double val = v.value<double>();����double val = v.getDouble();
 *
 * \author ����
 * \date ���� 2016
 */
class Parameter{
public:
	//��������
	enum Type{
		Int,
		Bool,
		String,
		Double,
		Ptr,
		UnKnown
	};
	//���캯��
	//��������
	//����ֵ����
    Parameter() {}
	//���캯��
	//������
	//	val: int������ֵ
	//����ֵ����
	Parameter(int val)
	{
		setInt(val);
	}
	//���캯��
	//������
	//	val: bool������ֵ
	//����ֵ����
	Parameter(bool val)
	{
		setBool(val);
	}
	//���캯��
	//������
	//	val: �ַ�������ֵ
	//����ֵ����
	Parameter(const char* val)
	{
		setString(val);
	}
	//���캯��
	//������
	//	val: double������ֵ
	//����ֵ����
	Parameter(double val)
	{
		setDouble(val);
	}
	//���캯��
	//������
	//	val: ָ��������ֵ
	//����ֵ����
	Parameter(void* val)
	{
		setPtr(val);
	}
	//��������
	//��������
	//����ֵ����
    ~Parameter() {}
	//��ñ�������
	//��������
	//����ֵ����������
	Type getType() const
	{
		return _type;
	}
	//���ñ�������
	//������
	//	type: ��������
	//����ֵ����
	void setType(Type type){_type = type;}
	//���ñ���ֵ
	//������
	//	val: int�ͱ���
	//����ֵ����
	void setInt(int val)
	{
		_data.iValue=val;
		char temp[50];
		sprintf(temp,"%d",val);
		_content = temp;
		_type=Int;
	}
	//���ñ���ֵ
	//������
	//	val: bool�ͱ���
	//����ֵ����
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
	//���ñ���ֵ
	//������
	//	val: �ַ��ͱ���
	//����ֵ����
	void setString(const char* val)
	{
		_sdata = val;
		_content = _sdata;
		_type = String;
	}
	//���ñ���ֵ
	//������
	//	val: double�ͱ���
	//����ֵ����
	void setDouble(double val)
	{
		_data.dValue=val;
		char temp[50];
		sprintf(temp,"%lf",val);
		_content = temp;
		_type = Double;
	}
	//���ñ���ֵ
	//������
	//	val: ָ���ͱ���
	//����ֵ����
	void setPtr(void* val)
	{
		_data.pValue=val;
		_type = Ptr;
	}
	//��ñ���ֵ
	//��������
	//����ֵ��int�ͱ���ֵ
	int getInt() const
	{
		if (_type==Int)
		{
			return _data.iValue;
		}
		else
			return -1111111;
	}
	//��ñ���ֵ
	//��������
	//����ֵ��bool�ͱ���ֵ
	bool getBool() const
	{
		if (_type==Bool)
		{
			return _data.bValue;
		}
		else
			return false;
	}
	//��ñ���ֵ
	//��������
	//����ֵ��double�ͱ���ֵ
	double getDouble() const
	{
		if (_type==Double)
		{
			return _data.dValue;
		}
		else
			return -11111111;
	}
	//��ñ���ֵ
	//��������
	//����ֵ���ַ��ͱ���ֵ
	const char* getString() const
	{
		if (_type==String)
		{
			return _sdata.c_str();
		}
		else
			return "";
	}
	//��ñ�������
	//��������
	//����ֵ����������
	const char* getContent() const
	{
		return _content.c_str();
	}
	//���ñ�������
	//������
	//		conten����������
	//����ֵ����
	void setContent(const char* content)
	{
		_content = content;
	}
	//��ñ���ֵ
	//��������
	//����ֵ��ָ���ͱ���ֵ
	void* getPtr() const
	{
		if (_type==Ptr)
		{
			return _data.pValue;
		}
		else
			return NULL;
	}
	//����ģ�����
	//C++Ҫ��ģ�庯����������ʵ�ֶ������߱��붼�ɼ���������Ҫ����ͬһ���ļ�������߰���ʵ�ֵ��ļ�
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


inline bool Parameter_cast_helper(const Parameter &v, Parameter::Type tp, void *ptr)  //����ת��
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
	Parameter::Type type = v.getType();	 //const����ָ��ֻ�ܵ���const���ͺ���
	if (type < (int)Parameter::UnKnown)    //����Ǵ��ڵ����;�ת��������
	{
		T t ;
		if (Parameter_cast_helper(v,type,&t))
			return t;
	}
	return T();
}

#endif