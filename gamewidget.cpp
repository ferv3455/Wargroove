#include "gamewidget.h"
#include "ui_gamewidget.h"

#include <QPainter>
#include <QMouseEvent>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , m_dragBeginPoint(0, 0)
{
    // Initialize ui
    ui->setupUi(this);

    // Initialize graphic widgets
    m_tipsLabel = ui->label;
    m_tipsLabel->setFixedHeight(80);
    m_tipsLabel->setStyleSheet("QLabel{color:#ffffff; background-color:#292c33; padding:10px}");
    m_tipsLabel->hide();

    // Initialize game data
    m_settings = new Settings(this);
    m_gameInfo = new GameInfo(this);

    m_map = new Map(m_settings->m_mapSize, this, m_settings->m_nBlockSize, QPoint(100, 100));
    m_map->loadFile(m_settings->m_mapFileName);

    // Initialize audio player
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setMedia(QUrl(m_settings->m_backgroundMusic));
    m_mediaPlayer->setVolume(m_settings->m_nVolume);
    m_mediaPlayer->play();

    // Initialize game engine
    m_unitMover = new UnitMover(m_settings, m_map, this);
    m_processer = new GameProcessor(this);

    // Initialize timers
    m_graphicsTimer = new QTimer(this);
    m_graphicsTimer->start(m_settings->m_nRefreshTime);
    connect(m_graphicsTimer, &QTimer::timeout, this, &GameWidget::updateAll);

    m_dynamicsTimer = new QTimer(this);
    m_dynamicsTimer->start(300);
    connect(m_dynamicsTimer, &QTimer::timeout, m_map, &Map::updateDynamics);

    // Connect signals and slots
    connect(this, &GameWidget::mapMoved, m_map, &Map::adjustOffset);
    connect(this, &GameWidget::mapZoomed, m_map, &Map::adjustScale);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &GameWidget::resetMedia);

    QVector<Block *> v;                 // WARNING: JUST FOR DEBUGGING MOVEMENTS
    for (int i = 0; i < 10; i++)
    {
        v.push_back(m_map->getBlock(i + 1, i));
    }
    m_unitMover->moveUnit(v);
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
    m_unitMover->paint(painter);

    delete painter;
}

void GameWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        m_dragBeginPoint = event->pos();
    }
}

void GameWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton)
    {
        const QPoint newPos = event->pos();
        emit mapMoved(newPos - m_dragBeginPoint);
        m_dragBeginPoint = newPos;
    }
}

void GameWidget::wheelEvent(QWheelEvent *event)
{
    emit mapZoomed((event->angleDelta().y() > 0 ? 1 : -1) * m_settings->m_nZoomScale, event->position());
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
