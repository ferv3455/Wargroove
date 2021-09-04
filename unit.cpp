#include "unit.h"

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

Unit::Unit(int unitId, int side, QObject *parent)
    : QObject(parent),
      m_nId(unitId),
      m_nSide(side),
      m_nDirection(0),
      m_bActive(true)
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
        imgFileName += "_1";                // TODO: should be updated
    }

    QImage img(imgFileName);
    int w = img.width();
    int h = img.height() / 2;
    m_images[0] = img.copy(0, 0, w, h);
    m_images[1] = img.copy(0, h, w, img.height() / 2);
    m_images[2] = m_images[0].mirrored(true, false);
    m_images[3] = m_images[1].mirrored(true, false);
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
}

int Unit::getId() const
{
    return m_nId;
}

int Unit::getSide() const
{
    return m_nSide;
}

bool Unit::getActivity() const
{
    return m_bActive;
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
