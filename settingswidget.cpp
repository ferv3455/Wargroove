#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "mainwidget.h"

#include <QPushButton>

SettingsWidget::SettingsWidget(Settings *settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget),
    m_settings(settings)
{
    // Initialize ui
    ui->setupUi(this);

    // Initialize background
    setAttribute(Qt::WA_StyledBackground, true);
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(QImage(":/wargroove-bg.png").scaled(size(),
                     Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    setPalette(palette);

    // Initialize menu
    for (const auto &mapName : qAsConst(m_settings->m_mapNames))
    {
        ui->mapComboBox->addItem(mapName);
    }
    ui->mapComboBox->setCurrentIndex(m_settings->m_nCurrentMap);

    // Connect signals and slots
    connect(ui->backButton, &QPushButton::clicked, this, &SettingsWidget::close);

    connect(ui->volumeSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        ui->volumeLabel->setText(QString::number(value));
    });
    connect(ui->volumeSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        m_settings->m_nVolume = value;
    });
    connect(ui->volumeSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        static_cast<MainWidget *>(parent)->setMediaVolume(value);
    });

    connect(ui->SEvolumeSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        ui->SEvolumeLabel->setText(QString::number(value));
    });
    connect(ui->SEvolumeSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        m_settings->m_nSEVolume = value;
    });

    connect(ui->refreshSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        ui->refreshLabel->setText(QString::number(value));
    });
    connect(ui->refreshSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        m_settings->m_nRefreshTime = value;
    });

    connect(ui->blockSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        ui->blockLabel->setText(QString::number(value));
    });
    connect(ui->blockSlider, &QSlider::valueChanged, this, [ = ](int value)
    {
        m_settings->m_nBlockSize = value;
    });

    connect(ui->mapComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeMap(int)));

    // Initial values
    ui->volumeSlider->setValue(m_settings->m_nVolume);
    ui->SEvolumeSlider->setValue(m_settings->m_nSEVolume);
    ui->refreshSlider->setValue(m_settings->m_nRefreshTime);
    ui->blockSlider->setValue(m_settings->m_nBlockSize);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QWidget::closeEvent(event);
}

void SettingsWidget::changeMap(int index)
{
    m_settings->m_nCurrentMap = index;
}
