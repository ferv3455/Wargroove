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
    m_contextMenu = new QMenu(this);

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
    m_processer = new GameProcessor(m_settings, m_gameInfo, m_map, m_tipsLabel, m_contextMenu, this);

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
    QFont font;
    font.setFamily(QFontDatabase::applicationFontFamilies(0).at(0));
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    setFont(font);

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
