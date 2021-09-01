#ifndef UNITMOVER_H
#define UNITMOVER_H

#include "settings.h"
#include "gameinfo.h"
#include "map.h"
#include "block.h"
#include "unit.h"

#include <QObject>
#include <QPainter>

class UnitMover : public QObject
{
    Q_OBJECT
public:
    explicit UnitMover(Settings *settings, Map *map, QObject *parent = nullptr);

    // Paint graphics
    void paint(QPainter *painter) const;

    // Moving operation
    void moveUnit(Block *fromBlock, Block *toBlock);
    void moveUnit(QVector<Block *> blocks);

public slots:
    void updateSingleMovement();      // updated when screen is refreshed
    void updateRouteMovement();       // updated when single step is finished

private:
    // Game stats
    Settings *m_settings;        // Game settings
    Map *m_map;                  // Game map

    // Moving related
    //      Single movement (step)
    int m_nTotalMoves;
    int m_nCurrentMove;
    Block *m_movingBlock[2];
    Unit *m_movingUnit;

    //      Route movement
    bool m_bMoving;
    QVector<Block *> m_movingRoute;

    // Update timer
    QTimer *m_refreshTimer;         // WARNING: REFRESH OVERLAPPING, or add mover->updatesingle in updateall

signals:
    void stepFinished();
    void movementFinished(Unit *);
};

#endif // UNITMOVER_H
