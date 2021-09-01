#ifndef BLOCK_H
#define BLOCK_H

#include "unit.h"

#include <QObject>
#include <QPainter>
#include <QImage>

class Block : public QObject
{
    Q_OBJECT
public:
    explicit Block(int terrain, int unit, int row, int col, QObject *parent = nullptr);

    void updateArea(QPoint center, int size);

    // Paint block
    // @param center:    the absolute position of the center
    // @param size:      the distance between the center and a side
    void paint(QPainter *painter, int dynamicsId = 0) const;

    // Getter and setter
    Unit *getUnit() const;
    void setUnit(int unit);
    void setUnit(Unit *newUnit);
    int getRow() const;
    int getColumn() const;

private:
    static QVector<QPointF> sm_pbasePoints;

    // Graphics related
    QPolygon m_area;         // area of the block (coordinates)
    QPoint m_center;         // center position
    int m_nBlockSize;        // size of the block

    // Properties
    int m_terrain;           // map terrain (1~9)
    QImage m_terrainImage;   // depict terrain
    Unit *m_unit;            // army unit (nullptr - nothing)

    int m_row;
    int m_col;
};

#endif // BLOCK_H
