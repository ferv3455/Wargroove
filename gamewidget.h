#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "map.h"
#include "settings.h"
#include "gameinfo.h"
#include "unitmover.h"
#include "gameprocessor.h"
#include "tipslabel.h"

#include <QWidget>
#include <QTimer>
#include <QtMultimedia/QMediaPlayer>

namespace Ui
{
class GameWidget;
}

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    virtual void paintEvent(QPaintEvent *event);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    void retranslate();

public slots:
    void updateAll();
    void resetMedia(QMediaPlayer::State);

private:
    Ui::GameWidget *ui;

    // Game data objects
    Settings *m_settings;        // Game settings
    GameInfo *m_gameInfo;        // Game info
    Map *m_map;                  // Game map

    // Game widgets
    TipsLabel *m_tipsLabel;      // Label at the bottom of the screen

    // Game engine
    GameProcessor *m_processer;  // Game processer

    // Sound effects
    QMediaPlayer *m_mediaPlayer; // bgm player
    QMediaPlayer *m_SEPlayer;    // sound effect player

    // Mouse events related
    QPoint m_dragBeginPoint;     // Start point of dragging

    // Timer
    QTimer *m_graphicsTimer;     // Timer: update screen graphics
    QTimer *m_dynamicsTimer;     // Timer: update unit dynamics

signals:
    void mapMoved(QPoint);
    void mouseScroll(int, QPointF);
};

#endif // GAMEWIDGET_H
