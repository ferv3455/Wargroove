#include "tipslabel.h"

#include <QDebug>

TipsLabel::TipsLabel(const QString &text, QWidget *parent)
    : QLabel(parent),
      m_nOpacity(500)
{
    // Formatting
    QFont font;
    font.setFamily(QString::fromUtf8("\346\226\271\346\255\243\345\203\217\347\264\24012"));
    font.setPointSize(16);
    setFont(font);

    setFixedHeight(80);

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
    m_timer->start(30);
}

void TipsLabel::updateBackground()
{
    if (m_nOpacity <= 0)
    {
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
