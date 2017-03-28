#ifndef mitk_main_msg_h__
#define mitk_main_msg_h__


/**
选中节点变化
int 选中节点数量
void* IQF_MitkDataManager*指针
*/
#define MITK_MESSAGE_SELECTION_CHANGED "MITK_MESSAGE_SELECTION_CHANGED"


/**
节点被移除
int 0
void* 被删除节点指针 mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_REMOVED "MITK_MESSAGE_NODE_REMOVED"

/**
增加节点
int 0
void* 增加的节点指针 mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_ADDED "MITK_MESSAGE_NODE_ADDED"

/**
节点被更改
int 0
void* 增加的节点指针 mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_CHANGED "MITK_MESSAGE_NODE_CHANGED"

/**
节点被删除
int 0
void* 增加的节点指针 mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_DELETED "MITK_MESSAGE_NODE_DELETED"

/**
节点交互器改变
int 0
void* 增加的节点指针 mitk::DataNode*
*/
#define MITK_MESSAGE_NODE_INTERACTOR_CHANGED "MITK_MESSAGE_NODE_INTERACTOR_CHANGED"


/**
MitkMultiWidgets 初始化完成
int 0
void* IQF_MitkRenderWindow*指针
*/
#define MITK_MESSAGE_MULTIWIDGET_INIT "MITK_MESSAGE_MULTIWIDGET_INIT"





#endif // mitk_main_msg_h__
