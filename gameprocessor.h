#ifndef GAMEPROCESSOR_H
#define GAMEPROCESSOR_H

#include "tipslabel.h"
#include "settings.h"
#include "gameinfo.h"
#include "map.h"
#include "unitmover.h"

#include <QObject>

class GameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit GameProcessor(Settings *settings,
                           GameInfo *gameInfo,
                           Map *map,
                           TipsLabel *tipsLabel,
                           QObject *parent = nullptr);

    void paint(QPainter *painter);
    void updateAccessibleBlocks(Block *block, int unitType, int movement);

public slots:
    void moveMap(QPoint);
    void zoomMap(int, QPointF);

    void selectPosition(QPoint);
    void mouseToPosition(QPoint);

    void contextMenu(QPoint);

private:
    // Game stats
    Settings *m_settings;               // Game settings
    GameInfo *m_gameInfo;               // Game info
    Map *m_map;                         // Game map

    // Widgets
    TipsLabel *m_tipsLabel;

    // Pointer images
    QImage m_pointerImage[3];

    // Game processers
    UnitMover *m_unitMover;             // Army unit mover

    // Other members
    int m_nStage;                       // Stage of a round
    Block *m_selectedBlock;             // Selected block
    Block *m_cursorBlock;               // The block with the pointer

    int m_nMovesLeft;                   // Move costs left
    QVector<Block *> m_movingRoute;     // Route saved

    QVector<Block *> m_accessibleBlocks;// Blocks highlighted when choosing the route

};

#endif // GAMEPROCESSOR_H
