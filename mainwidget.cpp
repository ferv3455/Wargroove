#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QFontDatabase>
#include <QIcon>

#include <QDebug>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MainWidget),
      m_gameWidget(nullptr),
      m_settingsWidget(nullptr),
      m_nTranslatorId(1),
      m_bStopped(false)

{
    // Initialize translators
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "Wargroove_" + QLocale(locale).name();
        if (m_translators[m_nTranslatorId].load(":/i18n/" + baseName))
        {
            m_nTranslatorId++;
        }
    }
    m_nTranslatorNumber = m_nTranslatorId;
    m_nTranslatorId = 0;

    // Initialize theme font
    int id = QFontDatabase::addApplicationFont(":/fonts/zpix.ttf");
    QFont font(QFontDatabase::applicationFontFamilies(id).at(0), 14, QFont::Bold);
    QApplication::setFont(font);
    qDebug() << "Font initialize:" << id << QFontDatabase::applicationFontFamilies(id).at(0);

    // Initialize cursor
    QPixmap pixmap(":/pointer");
    QCursor cursor(pixmap);
    setCursor(cursor);

    // Initialize ui
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Wargroove.png"));

    // Initialize background
    setAttribute(Qt::WA_StyledBackground, true);
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(QImage(":/wargroove-bg.png").scaled(size(),
                     Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    setPalette(palette);

    // Initialize game settings
    m_settings = new Settings(this);

    // Initialize background music
    m_bgMusic = new QMediaPlayer(this);
    m_bgMusic->setMedia(QUrl("./music/bgm01.mp3"));
    m_bgMusic->setVolume(m_settings->m_nVolume);
    m_bgMusic->play();

    // Connect signals to slots
    connect(ui->playButton, &QPushButton::clicked, this, &MainWidget::showGame);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWidget::showSettings);
    connect(ui->languageButton, &QPushButton::clicked, this, &MainWidget::retranslate);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainWidget::close);

    connect(m_bgMusic, &QMediaPlayer::stateChanged, this, &MainWidget::resetMedia);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // Initialize background
    setAttribute(Qt::WA_StyledBackground, true);
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(QImage(":/wargroove-bg.png").scaled(size(),
                     Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    setPalette(palette);
}

void MainWidget::retranslate()
{
    m_nTranslatorId++;
    if (m_nTranslatorId >= m_nTranslatorNumber)
    {
        m_nTranslatorId = 0;
    }

    if (m_nTranslatorId == 0)
    {
        QApplication::removeTranslator(m_translators + m_nTranslatorNumber - 1);
    }
    else
    {
        QApplication::installTranslator(m_translators + m_nTranslatorId);
    }

    ui->retranslateUi(this);
}

void MainWidget::showGame()
{
    m_gameWidget = new GameWidget(m_settings, this);
    connect(m_gameWidget, &GameWidget::windowClosed, this, &MainWidget::returnToMenu);
    ui->menuWidget->hide();
    ui->mainLayout->addWidget(m_gameWidget);
    m_gameWidget->show();

    m_bStopped = true;
    m_bgMusic->stop();
}

void MainWidget::showSettings()
{
    m_settingsWidget = new SettingsWidget(m_settings, this);
    connect(m_settingsWidget, &SettingsWidget::windowClosed, this, &MainWidget::returnToMenu);
    ui->menuWidget->hide();
    ui->mainLayout->addWidget(m_settingsWidget);
    m_settingsWidget->show();
}

void MainWidget::returnToMenu()
{
    m_bStopped = false;
    if (m_bgMusic->state() == QMediaPlayer::StoppedState)
    {
        m_bgMusic->play();
    }
    ui->menuWidget->show();
}

void MainWidget::resetMedia(QMediaPlayer::State newState)
{
    if (!m_bStopped && newState == QMediaPlayer::StoppedState)
    {
        m_bgMusic->play();
    }
}

void MainWidget::setMediaVolume(int volume)
{
    m_bgMusic->setVolume(volume);
}

