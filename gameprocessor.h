#ifndef GAMEPROCESSOR_H
#define GAMEPROCESSOR_H

#include "tipslabel.h"
#include "settings.h"
#include "gameinfo.h"
#include "map.h"
#include "unitmover.h"

#include <QObject>
#include <QMenu>

class GameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit GameProcessor(Settings *settings,
                           GameInfo *gameInfo,
                           Map *map,
                           TipsLabel *tipsLabel,
                           QMenu *contextMenu,
                           QObject *parent = nullptr);

    void paint(QPainter *painter);
    void updateAccessibleBlocks(Block *block, int unitType, int movement);
    void updateAttackableBlocks(Block *block, int rangeLow, int rangeHigh);
    void battle();

public slots:
    void processStage(int);

    void moveMap(QPoint);
    void zoomMap(int, QPointF);

    void selectPosition(QPoint);
    void mouseToPosition(QPoint);
    void unselectPosition(QPoint);

    void escapeMenu(QPoint);

private:
    // Game stats
    Settings *m_settings;               // Game settings
    GameInfo *m_gameInfo;               // Game info
    Map *m_map;                         // Game map

    // Widgets
    TipsLabel *m_tipsLabel;             // Tips label at the bottom of the screen
    QMenu *m_contextMenu;               // Right-button context menu

    // Menu actions
    QAction *m_actions[3];              // Attack, Wait, Cancel

    // Pointer images
    QImage m_pointerImage[5];           // Block highlights

    // Game processers
    UnitMover *m_unitMover;             // Army unit mover

    // Other members
    int m_nStage;                       // Stage of a round
    Block *m_selectedBlock;             // Selected block
    Block *m_cursorBlock;               // The block with the pointer

    int m_nMovesLeft;                   // Move costs left
    QVector<Block *> m_movingRoute;     // Route saved

    QVector<Block *> m_accessibleBlocks;// Accessible blocks highlighted when choosing the route
    QVector<Block *> m_attackableBlocks;// Attackable blocks highlighted when choosing the route

signals:
    void enterStage(int);
};

#endif // GAMEPROCESSOR_H
