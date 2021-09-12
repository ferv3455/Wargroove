#ifndef AIPROCESSOR_H
#define AIPROCESSOR_H

#include "settings.h"
#include "gameinfo.h"
#include "map.h"
#include "gamestats.h"
#include "unitmover.h"
#include "battlewidget.h"

#include <QObject>
#include <QMediaPlayer>

class AIProcessor : public QObject
{
    Q_OBJECT
public:
    explicit AIProcessor(Settings *settings,
                         GameInfo *gameInfo,
                         Map *map,
                         GameStats *stats,
                         QMediaPlayer *SEplayer,
                         QObject *parent = nullptr,
                         int side = 1);

    void paint(QPainter *painter);

    // Inner struct, used in choosing blocks
    struct BlockValue
    {
        Block *block;
        Block *lastBlock;
        Block *confrontingBlock;
        int distance;
        int value;
    };

    // Get accessible blocks from the given block
    void getAccessibleBlocks(QVector<BlockValue> &accessibleBlocks, Block *block, int unitType, int movement);

    // Add attack points to each accessible block
    // Detail: Point is given according to the opponent unit which offers the greatest point
    // (because you can only attack once in a round)
    void updateAttackPoints(QVector<BlockValue> &accessibleBlocks, Unit *unit);

    // Add defence points to each accessible block
    // Detail: Point is given according to the terrain defence and how many enemy units around
    void updateDefencePoints(QVector<BlockValue> &accessibleBlocks, Unit *unit);

    // If the unit isn't able to attack, find the shortest path to an enemy unit, save it in accessible blocks
    // The block with the most distance lower than movement is given a value 1000 to stand out
    void updateNearbyBlocks(QVector<BlockValue> &accessibleBlocks, Block *block, int unitType, int movement);

    void moveSingleUnit(Block *block);
    void operateBuilding(Block *block);
    void createUnit(int unitId, int side);

public slots:
    void activate();
    void nextMove();
    void confrontUnit();
    void finishRound();

private:
    // Game stats
    Settings *m_settings;               // Game settings
    GameInfo *m_gameInfo;               // Game info
    Map *m_map;                         // Game map
    GameStats *m_stats;                 // Game stats

    // Sound Effect
    QMediaPlayer *m_SEplayer;           // Sound effect

    // Widgets
    BattleWidget *m_battleWidget;       // Battle Widget

    // Game processers
    UnitMover *m_unitMover;             // Army unit mover

    // AI processing related
    QVector<Block *> m_remainingBlocks; // Blocks that can be moved
    QVector<Block *> m_movingRoute;     // Moving route
    Block *m_confrontingBlock;          // Confronting units
    Unit *m_tempUnit;                   // Temporary unit used for generating units

    // Other members
    int m_nSide;                        // Side in the game
    int m_nStage;                       // Stage of a round

signals:
    void finished();
    void operationFinished();
};

#endif // AIPROCESSOR_H
