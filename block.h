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
    explicit Block(int terrain, int row, int col, QObject *parent = nullptr);

    void updateArea(QPoint center, int size);

    // Paint block
    // @param center:    the absolute position of the center
    // @param size:      the distance between the center and a side
    void paint(QPainter *painter, int part = 0, int dynamicsId = 0) const;
    void paintPointer(QPainter *painter, QImage &image) const;

    // Getter and setter
    void setUnit(int unit, int side, int maxHP, int innerType = 0);
    void setUnit(Unit *newUnit);
    void setTerrain(int terrain);

    Unit *getUnit() const;
    QPoint getCenter() const;
    const QPolygon *getArea() const;
    int getTerrain() const;
    int getRow() const;
    int getColumn() const;

private:
    static QVector<QPointF> sm_pbasePoints;

    // Graphics related
    QPolygon m_area;         // area of the block (coordinates)
    QPoint m_pCenter;        // center position
    int m_nBlockSize;        // size of the block

    // Properties
    int m_nTerrain;           // map terrain (1~9)
    QImage m_terrainImage;   // depict terrain
    Unit *m_unit;            // army unit (nullptr - nothing)

    int m_row;
    int m_col;
};

#endif // BLOCK_H
