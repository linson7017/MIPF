#ifndef mitk_main_msg_h__
#define mitk_main_msg_h__


/**
ѡ�нڵ�仯
int ѡ�нڵ�����
void* IQF_MitkDataManager*ָ��
*/
#define MITK_MESSAGE_SELECTION_CHANGED "MITK_MESSAGE_SELECTION_CHANGED"


/**
�ڵ㱻�Ƴ�
int 0
void* ��ɾ���ڵ�ָ�� mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_REMOVED "MITK_MESSAGE_NODE_REMOVED"

/**
���ӽڵ�
int 0
void* ���ӵĽڵ�ָ�� mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_ADDED "MITK_MESSAGE_NODE_ADDED"

/**
�ڵ㱻����
int 0
void* ���ӵĽڵ�ָ�� mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_CHANGED "MITK_MESSAGE_NODE_CHANGED"

/**
�ڵ㱻ɾ��
int 0
void* ���ӵĽڵ�ָ�� mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_DELETED "MITK_MESSAGE_NODE_DELETED"

/**
�ڵ㽻�����ı�
int 0
void* ���ӵĽڵ�ָ�� mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_INTERACTOR_CHANGED "MITK_MESSAGE_NODE_INTERACTOR_CHANGED"


/**
MitkMultiWidgets ��ʼ�����
int 0
void* IQF_MitkRenderWindow*ָ��
*/
#define MITK_MESSAGE_MULTIWIDGET_INIT "MITK_MESSAGE_MULTIWIDGET_INIT"

/**
MitkMultiWidgets ����
int 0
void* QmitkStdMultiWidget*
*/
#define MITK_MESSAGE_MULTIWIDGET_HIDE "MITK_MESSAGE_MULTIWIDGET_HIDE"

/**
MitkMultiWidgets ��ʾ
int 0
void* QmitkStdMultiWidget*
*/
#define MITK_MESSAGE_MULTIWIDGET_SHOW "MITK_MESSAGE_MULTIWIDGET_SHOW"

/**
MitkMultiWidgets �ر�
int 0
void* QmitkStdMultiWidget*
*/
#define MITK_MESSAGE_MULTIWIDGET_CLOSE "MITK_MESSAGE_MULTIWIDGET_CLOSE"

/**
Reference ������Ϣ�޸�
int     0 
void* &std::pair<const char*,int>
*/
#define MITK_MESSAGE_REFERENCE_INT_CHANGED "MITK_MESSAGE_REFERENCE_INT_CHANGED"

/**
Reference ��������Ϣ�޸�
int     0
void* &std::pair<const char*,double>
*/
#define MITK_MESSAGE_REFERENCE_DOUBLE_CHANGED "MITK_MESSAGE_REFERENCE_DOUBLE_CHANGED"

/**
Reference ������Ϣ�޸�
int     0
void* &std::pair<const char*,bool>
*/
#define MITK_MESSAGE_REFERENCE_BOOL_CHANGED "MITK_MESSAGE_REFERENCE_BOOL_CHANGED"

/**
Reference �ַ�����Ϣ�޸�
int     0
void* &std::pair<const char*,const char*>
*/
#define MITK_MESSAGE_REFERENCE_STRING_CHANGED "MITK_MESSAGE_REFERENCE_STRING_CHANGED"

#endif // mitk_main_msg_h__
