#ifndef MAP_H
#define MAP_H

#include "block.h"

#include <QObject>
#include <QSize>

class Map : public QObject
{
    Q_OBJECT
public:
    // Constructor-related functions
    explicit Map(QSize size,
                 QObject *parent = nullptr,
                 int blockSize = 100,
                 QPoint offset = QPoint(200, 200),
                 QString file = "");
    ~Map();
    void loadFile(QString filename);

    // Get functions
    Block *getBlock(int row, int col) const;
    Block *getBlock(QPoint position) const;
    QSize getSize() const;
    int getDynamicsId() const;
    QPoint getCenterPosition(const Block *block) const;
    int getBlockSize() const;
    int getScale() const;
    void getAdjacentBlocks(QVector<Block *> &blockVector, const Block *block) const;

    // Adjust functions
    void adjustOffset(QPoint deltaPos);
    void adjustScale(int deltaScale, QPointF center);

    // Update after adjustments to blocks
    void updateAllBlocks() const;

    // Paint functions
    void paint(QPainter *painter) const;

public slots:
    void updateDynamics();

private:
    Block ***m_matrix;
    QSize m_size;
    QPoint m_pOffset;       // the relative position of the upper-left corner
    int m_nDynamicsId;      // showing dynamics of the units
    int m_nBlockSize;
    int m_nScale;           // 30 to 150

signals:

};

#endif // MAP_H
