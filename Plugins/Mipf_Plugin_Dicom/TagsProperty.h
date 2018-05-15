#ifndef TagsProperty_h__
#define TagsProperty_h__
/********************************************************************
	FileName:    TagsProperty
	Author:        Ling Song
	Date:           Month 5 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "mitkBaseProperty.h"

class TagsProperty : mitk::BaseProperty
{
public:
    mitkClassMacro(TagsProperty, BaseProperty);
    itkCloneMacro(Self);
    typedef std::pair<std::string, std::string> TagContentType;
    typedef std::map< std::string, TagContentType > TagMapType;
protected:
    TagsProperty() {}
    TagsProperty(const std::string& tag, const std::string& value, const std::string& description = "Unknown");
    TagsProperty(const TagsProperty &other) : BaseProperty(other), m_PropertyContent(other.m_PropertyContent) {}
    void InsertTag(const std::string& tag, const std::string& value, const std::string& description = "Unknown")
    {
        m_PropertyContent[tag] = TagContentType(value, description);
    }
    void GetTag()
private:
    // purposely not implemented
    TagsProperty &operator=(const TagsProperty &);

    virtual itk::LightObject::Pointer InternalClone() const override
    {
        itk::LightObject::Pointer result(new Self(*this));
        result->UnRegister();
        return result;
    }

    virtual bool IsEqual(const BaseProperty &other) const override
    {
        return (this->m_PropertyContent == static_cast<const Self &>(other).m_PropertyContent);
    }

    virtual bool Assign(const BaseProperty &other) override
    {
        this->m_PropertyContent = static_cast<const Self &>(other).m_PropertyContent;
        return true;
    }

    TagMapType m_PropertyContent;
};
#endif // TagsProperty_h__
