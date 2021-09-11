#ifndef MAP_H
#define MAP_H

#include "block.h"
#include "gameinfo.h"
#include "gamestats.h"

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
                 QPoint offset = QPoint(200, 200));
    ~Map();
    void loadTerrain(const QString &filename, bool fogMode = false);
    void loadUnits(const QString &filename, GameInfo *gameInfo, GameStats *stats);

    // Get functions
    Block *getBlock(int row, int col) const;
    Block *getBlock(QPoint position) const;
    QSize getSize() const;
    int getDynamicsId() const;
    QPoint getCenterPosition(const Block *block) const;
    int getBlockSize() const;
    int getScale() const;
    void getAdjacentBlocks(QVector<Block *> &blockVector, Block *block) const;
    void getAdjacentBlocks(QVector<Block *> &blockVector, Block *block, int rangeHigh, int rangeLow = 0) const;
    void getAllBlocks(QVector<Block *> &blockVector, int side) const;

    // Adjust functions
    void adjustOffset(QPoint deltaPos);
    void adjustScale(int deltaScale, QPointF center);

    // Update after adjustments to blocks
    void updateAllBlocks() const;

    // Paint functions
    // @param part:      0-terrain & units, 1-terrain, 2-units
    void paint(QPainter *painter, int part = 0) const;

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
