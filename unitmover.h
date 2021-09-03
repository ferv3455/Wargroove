#ifndef UNITMOVER_H
#define UNITMOVER_H

#include "settings.h"
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
    void moveUnit(Block *fromBlock, Block *toBlock);        // shouldn't be used
    void moveUnit(QVector<Block *> &blocks);                // use this

    // Check state
    bool isBusy() const;

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

    //      Route movement
    bool m_bMoving;
    Unit *m_movingUnit;
    QVector<Block *> m_movingRoute;
    QVector<Block *>::Iterator m_currentStep;

    // Update timer
    QTimer *m_refreshTimer;         // WARNING: REFRESH OVERLAPPING, or add mover->updatesingle in updateall

signals:
    void stepFinished();
    void movementFinished(Unit *);
};

#endif // UNITMOVER_H
