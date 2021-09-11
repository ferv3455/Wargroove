#include "block.h"
#include "building.h"

QVector<QPointF> Block::sm_pbasePoints =
{
    QPointF(-1, -0.6),
    QPointF(0, -1.2),
    QPointF(1, -0.6),
    QPointF(1, 0.6),
    QPointF(0, 1.2),
    QPointF(-1, 0.6)
};

QImage Block::grayImage(const QImage *image)
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

Block::Block(int terrain, int row, int col, QObject *parent, bool fogMode)
    : QObject(parent),
      m_area(),
      m_pCenter(),
      m_nBlockSize(0),

      m_nTerrain(terrain),
      m_unit(nullptr),

      m_bVisible(!fogMode),

      m_row(row),
      m_col(col)
{
    m_terrainImage[0] = QImage(":/image/terrain/" + QString::number(m_nTerrain));
    m_terrainImage[1] = grayImage(m_terrainImage);
}

void Block::updateArea(QPoint center, int size)
{
    m_pCenter = center;
    m_nBlockSize = size;

    QVector<QPoint> points;

    for (QPointF &point : sm_pbasePoints)
    {
        points.push_back((point * size + center).toPoint());
    }

    m_area = QPolygon(points);
}

void Block::paint(QPainter *painter, int part, int dynamicsId) const
{
    if (part != 2)
    {
        if (m_bVisible)
        {
            painter->drawImage(QRect(m_pCenter - QPoint(m_nBlockSize, 1.25 * m_nBlockSize),
                                     m_pCenter + QPoint(m_nBlockSize, 1.25 * m_nBlockSize)),
                               m_terrainImage[0]);
        }
        else
        {
            painter->drawImage(QRect(m_pCenter - QPoint(m_nBlockSize, 1.25 * m_nBlockSize),
                                     m_pCenter + QPoint(m_nBlockSize, 1.25 * m_nBlockSize)),
                               m_terrainImage[1]);
        }

        painter->setPen(QPen(QColor(0, 0, 0, 20), 3));
        painter->drawConvexPolygon(m_area);
    }

    if (part != 1 && m_unit != nullptr)
    {
        painter->setPen(Qt::white);
        painter->setBrush(Qt::black);
        if (m_unit->getId() == 19 && !m_bVisible)
        {
            // draw nobody-owned building
            m_unit->paint(painter,
                          QRect(m_pCenter - QPoint(m_nBlockSize, 1.5 * m_nBlockSize),
                                m_pCenter + QPoint(m_nBlockSize, 0.5 * m_nBlockSize)),
                          dynamicsId, -1);
        }
        else if (m_bVisible)
        {
            m_unit->paint(painter,
                          QRect(m_pCenter - QPoint(m_nBlockSize, 1.5 * m_nBlockSize),
                                m_pCenter + QPoint(m_nBlockSize, 0.5 * m_nBlockSize)),
                          dynamicsId);
        }
    }
}

void Block::paintPointer(QPainter *painter, QImage &image) const
{
    painter->drawImage(QRect(m_pCenter - QPoint(m_nBlockSize, 1.2 * m_nBlockSize),
                             m_pCenter + QPoint(m_nBlockSize, 1.2 * m_nBlockSize)), image);
}

void Block::setUnit(int unit, int side, int maxHP, int innerType)
{
    if (unit <= 18)
    {
        m_unit = new Unit(unit, side, maxHP, parent(), innerType);
    }
    else
    {
        m_unit = new Building(unit, side, maxHP, parent(), innerType);
    }
}

void Block::setUnit(Unit *newUnit)
{
    m_unit = newUnit;
}

void Block::setTerrain(int terrain)
{
    m_nTerrain = terrain;
    m_terrainImage[0] = QImage(":/image/terrain/" + QString::number(m_nTerrain));
    m_terrainImage[1] = grayImage(m_terrainImage);
}

void Block::setVisible(bool visible)
{
    m_bVisible = visible;
}

Unit *Block::getUnit() const
{
    return m_unit;
}

QPoint Block::getCenter() const
{
    return m_pCenter;
}

const QPolygon *Block::getArea() const
{
    return &m_area;
}

int Block::getTerrain() const
{
    return m_nTerrain;
}

int Block::getRow() const
{
    return m_row;
}

int Block::getColumn() const
{
    return m_col;
}

bool Block::isVisible() const
{
    return m_bVisible;
}

