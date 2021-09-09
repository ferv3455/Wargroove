#ifndef GAMEPROCESSOR_H
#define GAMEPROCESSOR_H

#include "tipslabel.h"
#include "settings.h"
#include "gameinfo.h"
#include "map.h"
#include "gamestats.h"
#include "unitmover.h"
#include "unitselectionwidget.h"
#include "descriptionwidget.h"
#include "battlewidget.h"

#include <QObject>
#include <QMediaPlayer>
#include <QMenu>

class GameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit GameProcessor(Settings *settings,
                           GameInfo *gameInfo,
                           Map *map,
                           GameStats *stats,
                           QMediaPlayer *SEplayer,
                           TipsLabel *tipsLabel,
                           UnitSelectionWidget *unitSelectionWidget,
                           DescriptionWidget *descriptionWidget,
                           QMenu *actionContextMenu,
                           QMenu *mainContextMenu,
                           QObject *parent = nullptr,
                           int movingSide = 0,
                           int totalSides = 2);

    void paint(QPainter *painter);

    // accessible blocks
    void updateAccessibleBlocks(Block *block, int unitType, int movement);

    // attackable or capturable blocks
    void updateOperatableBlocks(Block *block, int rangeLow, int rangeHigh,
                                bool capture = false);

    // carrier-related blocks
    void updateCarrierBlocks(Block *block, Unit *unit);

    // create a unit
    void createUnit(int unitId, int side);

    // the interaction of two units (battle/get in/get out)
    void confrontUnit();

public slots:
    void processStage(int);
    void changeSide();

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
    GameStats *m_stats;                 // Game stats

    // Sound Effect
    QMediaPlayer *m_SEplayer;           // Sound effect

    // Widgets
    TipsLabel *m_tipsLabel;             // Tips label at the bottom of the screen
    UnitSelectionWidget *m_unitSelectionWidget;  // Unit selection widget
    DescriptionWidget *m_descriptionWidget;      // Unit Description Widget
    BattleWidget *m_battleWidget;       // Battle Widget

    QMenu *m_actionContextMenu;         // Action context menu
    QMenu *m_mainContextMenu;           // Main context menu

    // Menu actions
    QAction *m_actions[6];              // Get in, Get out, Capture, Attack, Wait, Cancel
    QAction *m_mainActions[4];          // End turn, overview, mission, cancel

    // Pointer images
    QImage m_pointerImage[6];           // Block highlights

    // Game processers
    UnitMover *m_unitMover;             // Army unit mover

    // Moving side
    int m_nMovingSide;                  // Moving side: 0-player, 1-opponent
    int m_nTotalSides;                  // Number of sides

    // Other members
    int m_nStage;                       // Stage of a round
    Block *m_selectedBlock;             // Selected block
    Block *m_cursorBlock;               // The block with the pointer

    int m_nMovesLeft;                   // Move costs left
    QVector<Block *> m_movingRoute;     // Route saved

    QVector<Block *> m_accessibleBlocks;// Accessible blocks highlighted when choosing the route
    QVector<Block *> m_attackableBlocks;// Attackable blocks highlighted when choosing the route
    QVector<Block *> m_capturableBlocks;// Capturable blocks highlighted when choosing the route
    QVector<Block *>m_carrierBlocks;    // Blocks highlighted that are related to carriers

    Unit *m_tempUnit;                   // Used when creating a unit

signals:
    void enterStage(int);
    void endOfTurn();
};

#endif // GAMEPROCESSOR_H
