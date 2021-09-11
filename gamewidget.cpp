#include "gamewidget.h"
#include "ui_gamewidget.h"

#include <QPainter>
#include <QFontDatabase>
#include <QMouseEvent>

GameWidget::GameWidget(Settings *settings, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GameWidget),
      m_bClosing(false),
      m_pDragBeginPoint(0, 0),
      m_bMouseEventConnected(false)
{
    // Initialize cursor
    QPixmap pixmap(":/pointer");
    QCursor cursor(pixmap);
    setCursor(cursor);

    // Initialize ui
    ui->setupUi(this);
    ui->unitMoveWidget->hide();
    ui->gameOverLabel->hide();

    // Initialize background
    setAttribute(Qt::WA_StyledBackground, true);
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(QPixmap(":/image/bricks_background")));
    setPalette(palette);

    // Initialize game data
    m_settings = settings;
    m_gameInfo = new GameInfo(this);
    m_stats = new GameStats(this);

    int mapIndex = m_settings->m_nCurrentMap;
    m_map = new Map(m_settings->m_mapSizes[mapIndex], this,
                    m_settings->m_nBlockSize, QPoint(100, 100));
    m_map->loadTerrain(":/maps/terrain/" + m_settings->m_mapNames[mapIndex], m_settings->m_bFogMode);
    m_map->loadUnits(":/maps/units/" + m_settings->m_mapNames[mapIndex], m_gameInfo, m_stats);

    // Initialize graphic widgets
    m_tipsLabel = new TipsLabel(tr("Here are the tips. "), this);
    ui->verticalLayout->addWidget(m_tipsLabel);
    m_unitSelectionWidget = new UnitSelectionWidget(m_gameInfo, this);
    m_descriptionWidget = new DescriptionWidget(m_gameInfo, this);

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

    // Initialize audio player
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setMedia(QUrl(m_settings->m_backgroundMusic));
    m_mediaPlayer->setVolume(m_settings->m_nVolume);
    m_mediaPlayer->play();

    m_SEPlayer = new QMediaPlayer(this);
    m_SEPlayer->setMedia(QUrl("./music/click.mp3"));
    m_SEPlayer->setVolume(m_settings->m_nSEVolume);

    // Initialize game engine
    m_processor = new GameProcessor(m_settings, m_gameInfo, m_map, m_stats, m_SEPlayer,
                                    m_tipsLabel, ui->unitMoveWidget,
                                    m_unitSelectionWidget, m_descriptionWidget,
                                    m_actionContextMenu, m_mainContextMenu, this);
    m_ai = new AIProcessor *[1];        // Currently one ai
    for (int i = 0; i < 1; i++)     // Number of AIs
    {
        m_ai[i] = new AIProcessor(m_settings, m_gameInfo, m_map, m_stats, m_SEPlayer, this, i + 1);
    }

    // Initialize graphics timers
    m_graphicsTimer = new QTimer(this);
    m_graphicsTimer->start(m_settings->m_nRefreshTime);
    connect(m_graphicsTimer, &QTimer::timeout, this, &GameWidget::updateAll);

    m_dynamicsTimer = new QTimer(this);
    m_dynamicsTimer->start(300);
    connect(m_dynamicsTimer, &QTimer::timeout, m_map, &Map::updateDynamics);

    // Connect signals related to AI
    connect(m_processor, &GameProcessor::roundForSide, this, &GameWidget::initializeSide);
    for (int i = 0; i < 1; i++)     // Number of AIs
    {
        connect(m_ai[i], &AIProcessor::finished, m_processor, &GameProcessor::changeSide);
    }
    initializeSide(0);

    // Connect other signals to slots
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
    m_processor->paint(painter);
    for (int i = 0; i < 1; i++)
    {
        m_ai[i]->paint(painter);
    }
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
    m_descriptionWidget->adjustBackground();
}

void GameWidget::closeEvent(QCloseEvent *event)
{
    m_bClosing = true;
    m_mediaPlayer->stop();
    emit windowClosed();
    QWidget::closeEvent(event);
}

void GameWidget::updateAll()
{
    update();
}

void GameWidget::resetMedia(QMediaPlayer::State newState)
{
    if (!m_bClosing && newState == QMediaPlayer::StoppedState)
    {
        m_mediaPlayer->play();
    }
}

void GameWidget::initializeSide(int side)
{
    if (m_settings->m_bAI)
    {
        // Single player: activate AI and disconnect signals properly
        if (side <= 0)
        {
            if (!m_bMouseEventConnected)
            {
                m_bMouseEventConnected = true;
                connect(this, &GameWidget::mouseLeftButtonClicked, m_processor, &GameProcessor::selectPosition);
                connect(this, &GameWidget::mouseLeftButtonReleased, m_processor, &GameProcessor::unselectPosition);
                connect(this, &GameWidget::mouseRightButtonClicked, m_processor, &GameProcessor::escapeMenu);
                connect(this, &GameWidget::mouseMiddleButtonMoved, m_processor, &GameProcessor::moveMap);
                connect(this, &GameWidget::mouseMoved, m_processor, &GameProcessor::mouseToPosition);
                connect(this, &GameWidget::mouseScrolled, m_processor, &GameProcessor::zoomMap);
            }
        }
        else
        {
            if (m_bMouseEventConnected)
            {
                m_bMouseEventConnected = false;
                disconnect(this, &GameWidget::mouseLeftButtonClicked, m_processor, &GameProcessor::selectPosition);
                disconnect(this, &GameWidget::mouseLeftButtonReleased, m_processor, &GameProcessor::unselectPosition);
                disconnect(this, &GameWidget::mouseRightButtonClicked, m_processor, &GameProcessor::escapeMenu);
                disconnect(this, &GameWidget::mouseMiddleButtonMoved, m_processor, &GameProcessor::moveMap);
                disconnect(this, &GameWidget::mouseMoved, m_processor, &GameProcessor::mouseToPosition);
                disconnect(this, &GameWidget::mouseScrolled, m_processor, &GameProcessor::zoomMap);
            }
            m_ai[side - 1]->activate();
        }
    }
    else
    {
        // Multiplayer: no AIs, connect signals
        if (!m_bMouseEventConnected)
        {
            m_bMouseEventConnected = true;
            connect(this, &GameWidget::mouseLeftButtonClicked, m_processor, &GameProcessor::selectPosition);
            connect(this, &GameWidget::mouseLeftButtonReleased, m_processor, &GameProcessor::unselectPosition);
            connect(this, &GameWidget::mouseRightButtonClicked, m_processor, &GameProcessor::escapeMenu);
            connect(this, &GameWidget::mouseMiddleButtonMoved, m_processor, &GameProcessor::moveMap);
            connect(this, &GameWidget::mouseMoved, m_processor, &GameProcessor::mouseToPosition);
            connect(this, &GameWidget::mouseScrolled, m_processor, &GameProcessor::zoomMap);
        }
    }
}
