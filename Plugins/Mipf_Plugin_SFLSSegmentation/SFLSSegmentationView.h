#ifndef SFLSSegmentationView_h__
#define SFLSSegmentationView_h__

#include "MitkPluginView.h"
#include <QWidget>
#include "SFLSRobustStatSegmentor3DLabelMap_single.h"

#include "ui_WxAutoSegmentationViewControls.h"

class SFLSSegmentationView : public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    SFLSSegmentationView(QF::IQF_Main* pMain);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    virtual void SetFocus();

    void CreateView();
    public slots:

    void ApplySegment();

protected:

    Ui::WxAutoSegmentationViewControls* m_Controls;

    template < typename TPixel >
    void ItkImageRSSSegmentation(itk::Image< TPixel, 3 >* itkImage);
};

#endif // SFLSSegmentationView_h__