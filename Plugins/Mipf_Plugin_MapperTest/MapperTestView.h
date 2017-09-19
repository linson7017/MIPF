#ifndef MapperTestView_h__
#define MapperTestView_h__

#include "MitkPluginView.h"
#include <QWidget>

#include "mitkDataInteractor.h"

#include "ui_MapperTest.h"

class MapperTestView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    MapperTestView();
    void CreateView() override;
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    protected slots:
    void Apply();
    void Cut(bool enableCut);
    void Undo();
    void Redo();
    void InsideOut(bool flag);

private:
    Ui::MapperTestView m_ui;

    mitk::DataInteractor::Pointer m_curveDrawInteractor;
};

#endif // MapperTestView_h__