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





#endif // mitk_main_msg_h__
