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

Unit::Unit(int unitId, QObject *parent)
    : QObject(parent),
      m_nId(unitId),
      m_nDirection(0),
      m_bActive(true)
{
    QImage img = QImage(":/image/unit/" + QString::number(m_nId));
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
