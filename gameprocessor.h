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

public slots:
    void zoomMap(int, QPointF);

private:
    // Game stats
    Settings *m_settings;        // Game settings
    GameInfo *m_gameInfo;        // Game info
    Map *m_map;                  // Game map

    // Widgets
    TipsLabel *m_tipsLabel;

    // Game processers
    UnitMover *m_unitMover;      // Army unit mover

signals:

};

#endif // GAMEPROCESSOR_H
