#include "gamewidget.h"
#include "ui_gamewidget.h"

#include <QPainter>
#include <QFontDatabase>
#include <QMouseEvent>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GameWidget),
      m_pDragBeginPoint(0, 0)
{
    // Initialize cursor
    QPixmap pixmap(":/pointer");
    QCursor cursor(pixmap);
    setCursor(cursor);

    // Initialize ui
    ui->setupUi(this);

    // Initialize graphic widgets
    m_tipsLabel = new TipsLabel(tr("Here are the tips. "), this);
    ui->verticalLayout->addWidget(m_tipsLabel);
    m_unitSelectionWidget = new UnitSelectionWidget(this);

    QString customStyleSheet = "\
           QMenu {\
                background-color: #d1bead;\
                border: 3px solid #f4f6e9;\
           }\
           QMenu::item {\
                font-size: 10pt; \
                color: #3d4265;\
                border: 3px solid #f4f6e9;\
                background-color: #dfc686;\
                padding: 2px 10px; \
                margin: 2px 2px;\
           }\
           QMenu::item:selected {\
                color: #f4f4e8;\
                background-color: #a47750;\
           }\
           QMenu::item:pressed {\
                color: #f4f4e8;\
                background-color: #a47750;\
           }";
    m_actionContextMenu = new QMenu(this);
    m_actionContextMenu->setStyleSheet(customStyleSheet);
    m_mainContextMenu = new QMenu(this);
    m_mainContextMenu->setStyleSheet(customStyleSheet);

    // Initialize game data
    m_settings = new Settings(this);
    m_gameInfo = new GameInfo(this);
    m_stats = new GameStats(this);
    m_map = new Map(m_settings->m_mapSize, this,
                    m_settings->m_nBlockSize, QPoint(100, 100));
    m_map->loadTerrain(m_settings->m_mapTerrainFileName);
    m_map->loadUnits(m_settings->m_mapUnitsFileName, m_gameInfo, m_stats);

    // Initialize audio player
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setMedia(QUrl(m_settings->m_backgroundMusic));
    m_mediaPlayer->setVolume(m_settings->m_nVolume);
    m_mediaPlayer->play();

    // Initialize game engine
    m_processer = new GameProcessor(m_settings, m_gameInfo, m_map, m_stats,
                                    m_tipsLabel, m_unitSelectionWidget,
                                    m_actionContextMenu, m_mainContextMenu, this);

    // Initialize graphics timers
    m_graphicsTimer = new QTimer(this);
    m_graphicsTimer->start(m_settings->m_nRefreshTime);
    connect(m_graphicsTimer, &QTimer::timeout, this, &GameWidget::updateAll);

    m_dynamicsTimer = new QTimer(this);
    m_dynamicsTimer->start(300);
    connect(m_dynamicsTimer, &QTimer::timeout, m_map, &Map::updateDynamics);

    // Connect other signals to slots
    connect(this, &GameWidget::mouseLeftButtonClicked, m_processer, &GameProcessor::selectPosition);
    connect(this, &GameWidget::mouseLeftButtonReleased, m_processer, &GameProcessor::unselectPosition);
    connect(this, &GameWidget::mouseRightButtonClicked, m_processer, &GameProcessor::escapeMenu);
    connect(this, &GameWidget::mouseMiddleButtonMoved, m_processer, &GameProcessor::moveMap);
    connect(this, &GameWidget::mouseMoved, m_processer, &GameProcessor::mouseToPosition);
    connect(this, &GameWidget::mouseScrolled, m_processer, &GameProcessor::zoomMap);

    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &GameWidget::resetMedia);

    // Track mouse movements
    setMouseTracking(true);
}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter *painter = new QPainter(this);

    // Set general font
    QFont font(QFontDatabase::applicationFontFamilies(0).at(0), 12, QFont::Bold);
    painter->setFont(font);

    m_map->paint(painter, 1);       // terrain
    m_processer->paint(painter);
    m_map->paint(painter, 2);       // units

    delete painter;
}

void GameWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit mouseLeftButtonClicked(event->pos());
    }
    else if (event->button() == Qt::RightButton)
    {
        emit mouseRightButtonClicked(event->pos());
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_pDragBeginPoint = event->pos();
        emit mouseMiddleButtonClicked(event->pos());
    }
}

void GameWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        const QPoint newPos = event->pos();
        emit mouseMiddleButtonMoved(newPos - m_pDragBeginPoint);
        m_pDragBeginPoint = newPos;
    }

    emit mouseMoved(event->pos());
}

void GameWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit mouseLeftButtonReleased(event->pos());
    }
}

void GameWidget::wheelEvent(QWheelEvent *event)
{
    emit mouseScrolled(event->angleDelta().y() > 0 ? 1 : -1, event->position());
}

void GameWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_unitSelectionWidget->adjustSize();
}

void GameWidget::retranslate()
{
    ui->retranslateUi(this);
}

void GameWidget::updateAll()
{
    update();
}

void GameWidget::resetMedia(QMediaPlayer::State newState)
{
    if (newState == QMediaPlayer::StoppedState)
    {
        m_mediaPlayer->play();
    }
}
