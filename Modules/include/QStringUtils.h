#ifndef QStringUtils_h__
#define QStringUtils_h__
#include <QString>
#include <QTextCodec>

class QStringUtils
{
public:
    static const char* GetLocalString(const QString& qStr,std::string& str)
    {
        str = qStr.toLocal8Bit().constData();
#ifdef _WIN32
        QTextCodec *code = QTextCodec::codecForName("gb2312");
        str = code->fromUnicode(qStr).data();
#endif // _WIN32
        return str.c_str();
    }
};


#endif // StringUtils_h__
