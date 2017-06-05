#ifndef QfResult_h__
#define QfResult_h__

#include <string.h>
enum RESULT_CODE
{

    RESULT_NORMAL,
    RESULT_OK,
    ///not success
    RESULT_ERROR,
    RESULT_EXCEPTION,
    RESLUT_UNKNOWN
};

class QfResult
{
public:
	QfResult():m_resultMessage(""),m_resultCode(RESULT_OK){}
	~QfResult(){}
	
	//function
    void Init() {
        m_resultCode = RESULT_OK;
        m_resultMessage = "";
    }

	void SetResultCode(const RESULT_CODE code){m_resultCode = code;}
	RESULT_CODE GetResultCode() const {return m_resultCode;}
	
	const char* GetResultMessage() const {return m_resultMessage.c_str();}
	void SetResultMessage(const std::string& message){m_resultMessage = message;}
    void AppendMessage(const std::string& message) { m_resultMessage.append(message); }
	
	bool IsSuccess() const {return m_resultCode<=RESULT_OK;}
private:
	RESULT_CODE m_resultCode;
	std::string m_resultMessage;
};

#endif // QfResult_h__