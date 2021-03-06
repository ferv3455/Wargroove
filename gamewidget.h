#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "map.h"
#include "settings.h"
#include "gameinfo.h"
#include "gamestats.h"
#include "unitmover.h"
#include "gameprocessor.h"
#include "aiprocessor.h"
#include "tipslabel.h"
#include "unitselectionwidget.h"
#include "descriptionwidget.h"

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
    explicit GameWidget(Settings *settings, QWidget *parent = nullptr);
    ~GameWidget();

    virtual void paintEvent(QPaintEvent *event);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void closeEvent(QCloseEvent *event);

public slots:
    void updateAll();
    void resetMedia(QMediaPlayer::State);
    void initializeSide(int);

private:
    Ui::GameWidget *ui;

    // Game data objects
    Settings *m_settings;        // Game settings
    GameInfo *m_gameInfo;        // Game info
    Map *m_map;                  // Game map
    GameStats *m_stats;          // Game stats

    // Game widgets
    TipsLabel *m_tipsLabel;      // Label at the bottom of the screen
    UnitSelectionWidget *m_unitSelectionWidget;   // Unit selection widget
    DescriptionWidget *m_descriptionWidget;       // Block discription widget

    QMenu *m_actionContextMenu;  // Action context menu
    QMenu *m_mainContextMenu;    // Main context menu

    // Game engine
    GameProcessor *m_processor;  // Game processor
    AIProcessor **m_ai;           // Game AI Processors

    // Sound effects
    QMediaPlayer *m_mediaPlayer; // bgm player
    QMediaPlayer *m_SEPlayer;    // sound effect player
    bool m_bClosing;             // closing the widget

    // Mouse events related
    QPoint m_pDragBeginPoint;    // Start point of dragging
    bool m_bMouseEventConnected; // Whether mouse events are received

    // Timer
    QTimer *m_graphicsTimer;     // Timer: update screen graphics
    QTimer *m_dynamicsTimer;     // Timer: update unit dynamics

signals:
    void mouseLeftButtonClicked(QPoint);
    void mouseLeftButtonReleased(QPoint);
    void mouseRightButtonClicked(QPoint);
    void mouseMiddleButtonClicked(QPoint);
    void mouseMiddleButtonMoved(QPoint);
    void mouseMoved(QPoint);
    void mouseScrolled(int, QPointF);

    void AIFinished();

    void windowClosed();
};

#endif // GAMEWIDGET_H
