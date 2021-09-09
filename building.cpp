#include "building.h"

#include <QDebug>

Building::Building(int unitId, int side, int maxHP, QObject *parent, int innerType)
    : Unit(unitId, side, maxHP, parent, innerType)
{
    // Reload images
    for (int i = 0; i < 3; i++)
    {
        m_images[i] = QImage(QString(":/image/building/%1_%2_%3").arg(unitId).arg(innerType).arg(i - 1));
    }

    // Uncaptured buildings
    if (m_nSide < 0)
    {
        m_nHealthPoint = 0;
    }

    // Base
    if (m_nInnerType == 0)
    {
        m_nHealthPoint *= 2;
        m_nMaxHealthPoint *= 2;
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

void Building::getImages(QVector<QImage> *images) const
{
    images->clear();
    if (m_nId <= 18)
    {
        Unit::getImages(images);
    }
    else
    {
        images->push_back(m_images[m_nSide + 1]);
        images->push_back(m_images[m_nSide + 1]);
    }
}


bool Building::isOperable() const
{
    return Unit::isOperable() || (m_nId == 19 && m_nInnerType >= 1 && m_nInnerType <= 3);
}
