#include "unit.h"

#include <QDebug>

QImage Unit::grayImage(const QImage *image)
{
    QImage img(*image);
    for (int i = 0; i < img.width(); ++i)
    {
        for (int j = 0; j < img.height(); ++j)
        {
            QColor color = img.pixelColor(i, j);
            int gray = qGray(color.rgb());
            int alpha = color.alpha();
            img.setPixelColor(i, j, QColor(gray, gray, gray, alpha));
        }
    }
    return img;
}

Unit::Unit(int unitId, int side, int maxHP, QObject *parent, int innerType)
    : QObject(parent),
      m_nId(unitId),
      m_nInnerType(innerType),
      m_nSide(side),
      m_nDirection(0),
      m_bActive(true),
      m_carryingUnit(nullptr),
      m_nMaxHealthPoint(maxHP)
{
    m_nHealthPoint = m_nMaxHealthPoint;

    if (m_nId <= 18)
    {
        QString imgFileName(":/image/");

        if (side == 0)
        {
            imgFileName += "unit/";
        }
        else if (side == 1)
        {
            imgFileName += "enemy_unit/";
        }

        imgFileName += QString::number(m_nId);
        if (unitId == 18)
        {
            imgFileName += ("_" + QString::number(m_nInnerType));
        }

        QImage img(imgFileName);
        int w = img.width();
        int h = img.height() / 2;
        m_images[0] = img.copy(0, 0, w, h);
        m_images[1] = img.copy(0, h, w, img.height() / 2);
        m_images[2] = m_images[0].mirrored(true, false);
        m_images[3] = m_images[1].mirrored(true, false);
    }
}

void Unit::paint(QPainter *painter, const QRect &rect, int dynamicsId) const
{
    if (m_bActive)
    {
        painter->drawImage(rect, m_images[2 * m_nDirection + dynamicsId]);
    }
    else
    {
        painter->drawImage(rect, grayImage(m_images + 2 * m_nDirection + dynamicsId));
    }

//    painter->setFont(QFont("Arial", 20, QFont::Bold));
    painter->drawText(rect.center(), QString::number(m_nHealthPoint));
}

int Unit::getId() const
{
    return m_nId;
}

int Unit::getInnerType() const
{
    return m_nInnerType;
}

int Unit::getSide() const
{
    return m_nSide;
}

bool Unit::getActivity() const
{
    return m_bActive;
}

int Unit::getHP() const
{
    return m_nHealthPoint;
}

int Unit::getMaxHP() const
{
    return m_nMaxHealthPoint;
}

Unit *Unit::getCarrier() const
{
    return m_carryingUnit;
}

bool Unit::isOperable() const
{
    return m_nId <= 18;
}

bool Unit::isCarrier() const
{
    return m_nId == 7 || m_nId == 10 || m_nId == 14;
}

void Unit::setDirection(int direction)
{
    if (direction > 0)
    {
        m_nDirection = 1;
    }
    else
    {
        m_nDirection = 0;
    }
}

void Unit::setActivity(bool active)
{
    m_bActive = active;
}

void Unit::setCarrier(Unit *unit)
{
    m_carryingUnit = unit;
}

bool Unit::injured(int damage)
{
    m_nHealthPoint -= damage;
    if (m_nHealthPoint <= 0)
    {
        m_nHealthPoint = 0;
    }
    qDebug() << m_nId << "injured" << damage << "remaining" << m_nHealthPoint;
    return m_nHealthPoint <= 0;
}

bool Unit::checkCritical() const
{
    return false;       // TODO: REFINED
}

void Unit::regenerate(double ratio)
{
    m_nHealthPoint += (ratio * m_nMaxHealthPoint);
    if (m_nHealthPoint > m_nMaxHealthPoint)
    {
        m_nHealthPoint = m_nMaxHealthPoint;
    }
}
