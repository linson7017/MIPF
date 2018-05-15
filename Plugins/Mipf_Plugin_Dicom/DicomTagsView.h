#ifndef DicomTagsView_h__ 
#define DicomTagsView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_DicomTagsView.h"
 
class DicomTagsView : public QWidget,public MitkPluginView  
{  
    Q_OBJECT
public:   
    typedef std::map<std::string, std::string> TagMapType;
    DicomTagsView(); 
    ~DicomTagsView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
private:
    mitk::DataNode::Pointer ReadDicom(const char* filename,const char* nodename);
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);
    static const TagMapType &GetDICOMTagsToTagMap();
protected slots:
void Load();
private:
    Ui::DicomTagsView m_ui;
};
#endif // DicomTagsView_h__ 