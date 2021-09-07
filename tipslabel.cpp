#include "tipslabel.h"

#include <QFontDatabase>

TipsLabel::TipsLabel(const QString &text, QWidget *parent)
    : QLabel(parent),
      m_nOpacity(500)
{
    // Formatting
    QFont font(QFontDatabase::applicationFontFamilies(0).at(0), 16, QFont::Normal);
    setFont(font);

    setFixedHeight(80);
    setMargin(20);

    // Initialize timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TipsLabel::updateBackground);

    // Popup label
    popup(text);
}

void TipsLabel::popup(const QString &text)
{
    setText(text);
    m_nOpacity = 500;
    updateBackground();
    show();
    m_timer->start(30);
}

void TipsLabel::updateBackground()
{
    if (m_nOpacity <= 0)
    {
        hide();
        m_timer->stop();
        return;
    }

    m_nOpacity -= 10;

    if (m_nOpacity > 250)
    {
        setStyleSheet("color:rgba(255,255,255,255); background-color:rgba(45,45,45,255);");
    }
    else
    {
        setStyleSheet(QString("color:rgba(255,255,255,%1); background-color:rgba(45,45,45,%1);").arg(QString::number(m_nOpacity)));
    }
}
