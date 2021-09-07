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

Block::Block(int terrain, int row, int col, QObject *parent)
    : QObject(parent),
      m_area(),
      m_pCenter(),
      m_nBlockSize(0),

      m_nTerrain(terrain),
      m_terrainImage(":/image/terrain/" + QString::number(m_nTerrain)),
      m_unit(nullptr),

      m_row(row),
      m_col(col)
{

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
        painter->setPen(QPen(QColor(0, 0, 0, 20), 3));
        painter->drawImage(QRect(m_pCenter - QPoint(m_nBlockSize, 1.25 * m_nBlockSize),
                                 m_pCenter + QPoint(m_nBlockSize, 1.25 * m_nBlockSize)), m_terrainImage);
        painter->drawConvexPolygon(m_area);
    }

    if (part != 1 && m_unit != nullptr)
    {
        painter->setPen(Qt::white);
        m_unit->paint(painter,
                      QRect(m_pCenter - QPoint(m_nBlockSize, 1.5 * m_nBlockSize),
                            m_pCenter + QPoint(m_nBlockSize, 0.5 * m_nBlockSize)),
                      dynamicsId);
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

