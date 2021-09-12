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

    void moveSingleUnit(Block *block);
    void operateBuilding(Block *block);

public slots:
    void activate();
    void nextMove();
    void confrontUnit();

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
