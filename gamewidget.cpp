#include "gamewidget.h"
#include "ui_gamewidget.h"

#include <QPainter>
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

    // Initialize game data
    m_settings = new Settings(this);
    m_gameInfo = new GameInfo(this);
    m_map = new Map(m_settings->m_mapSize, this,
                    m_settings->m_nBlockSize, QPoint(100, 100), m_settings->m_mapFileName);

    // Initialize audio player
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setMedia(QUrl(m_settings->m_backgroundMusic));
    m_mediaPlayer->setVolume(m_settings->m_nVolume);
    m_mediaPlayer->play();

    // Initialize game engine
    m_processer = new GameProcessor(m_settings, m_gameInfo, m_map, m_tipsLabel, this);

    // Initialize graphics timers
    m_graphicsTimer = new QTimer(this);
    m_graphicsTimer->start(m_settings->m_nRefreshTime);
    connect(m_graphicsTimer, &QTimer::timeout, this, &GameWidget::updateAll);

    m_dynamicsTimer = new QTimer(this);
    m_dynamicsTimer->start(300);
    connect(m_dynamicsTimer, &QTimer::timeout, m_map, &Map::updateDynamics);

    // Connect other signals to slots
    connect(this, &GameWidget::mouseLeftButtonClicked, m_processer, &GameProcessor::selectPosition);
    connect(this, &GameWidget::mouseRightButtonMoved, m_processer, &GameProcessor::moveMap);
    connect(this, &GameWidget::mouseMoved, m_processer, &GameProcessor::mouseToPosition);
    connect(this, &GameWidget::mouseScrolled, m_processer, &GameProcessor::zoomMap);

    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &GameWidget::resetMedia);

    // Track mouse movements
    setMouseTracking(true);

//    QVector<Block *> v;                 // WARNING: JUST FOR DEBUGGING MOVEMENTS
//    for (int i = 0; i < 10; i++)
//    {
//        v.push_back(m_map->getBlock(i + 1, i));
//    }
//    m_unitMover->moveUnit(v);
}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter *painter = new QPainter(this);

    m_map->paint(painter);
    m_processer->paint(painter);

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
        m_pDragBeginPoint = event->pos();
    }
}

void GameWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton)
    {
        const QPoint newPos = event->pos();
        emit mouseRightButtonMoved(newPos - m_pDragBeginPoint);
        m_pDragBeginPoint = newPos;
    }

    emit mouseMoved(event->pos());
}

void GameWidget::wheelEvent(QWheelEvent *event)
{
    emit mouseScrolled(event->angleDelta().y() > 0 ? 1 : -1, event->position());
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
