#include "SplashWindow.h"

#include "iqf_main.h"
#include <QLabel>
#include <QApplication>
#include <QPainter>
#include <QResizeEvent>


SplashWindow::SplashWindow(QF::IQF_Main* pMain, const char* backgrouUrl):
    m_pMain(pMain),
    QWidget(NULL, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen)
{
    m_pMain->Attach(this);


    m_pMessageLabel = new QLabel("Loading...",this);
    m_pMessageLabel->setAttribute(Qt::WA_TranslucentBackground);

    m_backgroundImage.load(backgrouUrl);
    resize(m_backgroundImage.size());


}


SplashWindow::~SplashWindow()
{
}

void SplashWindow::showMessage(const char* message)
{
    m_pMessageLabel->setText(message);
    qApp->processEvents();
}

void SplashWindow::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "QFMAIN_LOAD_COMPONENT_MESSAGE") == 0)
    {
        char* str = (char*)pValue;
        showMessage(str);
    }
}

void SplashWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(rect(), m_backgroundImage);
}

void SplashWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    setMask(createMaskRegion(m_backgroundImage));
    int margin = 10;
    
    m_pMessageLabel->setGeometry(margin, resizeEvent->size().height() - m_pMessageLabel->height(), 
        resizeEvent->size().width() - margin, m_pMessageLabel->height());
}

QRegion SplashWindow::createMaskRegion(const QImage &image, bool posMask)
{
    if (image.isNull())
        return QRegion();

    if (image.depth() != 32)
    {
        QImage img32 = image.convertToFormat(QImage::Format_RGB32);
        return createMaskRegion(img32, posMask);
    }

    int width = image.width();
    int height = image.height();
    QRegion retVal;

    for (int y = 0; y < height; ++y)
    {
        // guarented to be 32 bit by the check above
        QRgb *currLine = (QRgb *)image.scanLine(y);
        int xstart = -1;
        int xcurr = -1;
        QRgb *currVal = currLine;
        for (int x = 0; x < width; ++x, ++currVal)
        {
            int alpha = qAlpha(*currVal);
            if ((posMask && alpha != 0) || (!posMask && alpha == 0))
            {
                // on non-alpha pixel
                if (xstart == -1)
                {
                    // remember start of non-alpha line-segment (if necessary)
                    xstart = x;
                }
                xcurr = x;
            }
            else               // alpha-pixel
                if (xcurr != -1) // if this alpha pixel is following a non-alpha line-segment
                {
                    retVal += QRegion(xstart, y, xcurr - xstart + 1, 1);
                    xstart = -1;
                    xcurr = -1;
                }
        }
        if (xcurr != -1)
        {
            retVal += QRegion(xstart, y, xcurr - xstart + 1, 1);
        }
    }

    return retVal;
}

