#include "building.h"

#include <QDebug>

Building::Building(int unitId, int side, int maxHP, QObject *parent, int innerType)
    : Unit(unitId, side, maxHP, parent, innerType)
{
    // Reload images
    for (int i = 0; i < 3; i++)
    {
        QImage img(QString(":/image/building/%1_%2_%3").arg(unitId).arg(innerType).arg(i - 1));
        m_images[i] = img.copy(0, 0, img.width(), img.height() / 2);
    }

    // Uncaptured buildings
    if (m_nSide < 0)
    {
        m_nHealthPoint = 0;
    }
}

void Building::paint(QPainter *painter, const QRect &rect, int dynamicsId) const
{
    Q_UNUSED(dynamicsId);

    if (m_bActive)
    {
        painter->drawImage(rect, m_images[m_nSide + 1]);
    }
    else
    {
        painter->drawImage(rect, grayImage(m_images + m_nSide + 1));
    }

//    painter->setFont(QFont("Arial", 20, QFont::Bold));
    painter->drawText(rect.center(), QString::number(m_nHealthPoint));
}

void Building::setSide(int side)
{
    m_nSide = side;
}

bool Building::isOperable() const
{
    return Unit::isOperable() || (m_nId == 19 && m_nSide == 0 && m_nInnerType >= 1 && m_nInnerType <= 3);
}
