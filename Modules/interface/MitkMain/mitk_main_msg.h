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

/**
MitkMultiWidgets 隐藏
int 0
void* QmitkStdMultiWidget*
*/
#define MITK_MESSAGE_MULTIWIDGET_HIDE "MITK_MESSAGE_MULTIWIDGET_HIDE"

/**
MitkMultiWidgets 显示
int 0
void* QmitkStdMultiWidget*
*/
#define MITK_MESSAGE_MULTIWIDGET_SHOW "MITK_MESSAGE_MULTIWIDGET_SHOW"

/**
MitkMultiWidgets 关闭
int 0
void* QmitkStdMultiWidget*
*/
#define MITK_MESSAGE_MULTIWIDGET_CLOSE "MITK_MESSAGE_MULTIWIDGET_CLOSE"

/**
Reference 整型信息修改
int     0 
void* &std::pair<const char*,int>
*/
#define MITK_MESSAGE_REFERENCE_INT_CHANGED "MITK_MESSAGE_REFERENCE_INT_CHANGED"

/**
Reference 浮点型信息修改
int     0
void* &std::pair<const char*,double>
*/
#define MITK_MESSAGE_REFERENCE_DOUBLE_CHANGED "MITK_MESSAGE_REFERENCE_DOUBLE_CHANGED"

/**
Reference 布尔信息修改
int     0
void* &std::pair<const char*,bool>
*/
#define MITK_MESSAGE_REFERENCE_BOOL_CHANGED "MITK_MESSAGE_REFERENCE_BOOL_CHANGED"

/**
Reference 字符串信息修改
int     0
void* &std::pair<const char*,const char*>
*/
#define MITK_MESSAGE_REFERENCE_STRING_CHANGED "MITK_MESSAGE_REFERENCE_STRING_CHANGED"

#endif // mitk_main_msg_h__
