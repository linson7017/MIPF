#ifndef ObjectFactory_h__
#define ObjectFactory_h__

#include <map>
#include <functional>

class ObjectFactory
{
public:
    void *CreateObject(const std::string &name)
    {
        std::map<std::string, std::function<void*(void)>>::const_iterator it;
        it = mapCls_.find(name);
        if (it == mapCls_.end())
            return  0;
        else
            return it->second();  //func();
    }

    static ObjectFactory * ObjectFactory::Instance()
    {
        static ObjectFactory factory;
        return &factory;
    }

    void Register(const std::string &name, std::function<void*(void)> func)
    {
        mapCls_[name] = func;
    }
private:
    std::map<std::string, std::function<void*(void)>> mapCls_;
};

class ObjectRegister
{
public:
    ObjectRegister(const std::string &name, std::function<void*(void)> func)
    {
        ObjectFactory::Instance()->Register(name, func);
    }
}; 

#define REGISTER_GENERAL_CLASS(NAME, TYPE)  static ObjectRegister TYPE##ObjectRegister(NAME, [](void) -> void * { return new TYPE();});


#endif // ObjectFactory_h__
