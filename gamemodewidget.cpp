#include "gamemodewidget.h"
#include "ui_gamemodewidget.h"

GameModeWidget::GameModeWidget(Settings *settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameModeWidget),
    m_settings(settings)
{
    // Initialize ui
    ui->setupUi(this);

    // Initialize checkbox
    ui->fogCheckBox->setChecked(m_settings->m_bFogMode);

    // Initialize menu
    for (const auto &mapName : qAsConst(m_settings->m_mapNames))
    {
        ui->mapComboBox->addItem(mapName);
    }
    ui->mapComboBox->setCurrentIndex(m_settings->m_nCurrentMap);

    // Connect signals and slots
    connect(ui->fogCheckBox, &QCheckBox::stateChanged, this, [ = ]()
    {
        m_settings->m_bFogMode = ui->fogCheckBox->isChecked();
    });
    connect(ui->singlePlayerButton, &QPushButton::clicked, this, [ = ]()
    {
        m_settings->m_bAI = true;
        emit startGame();
        close();
    });
    connect(ui->multiplayerButton, &QPushButton::clicked, this, [ = ]()
    {
        m_settings->m_bAI = false;
        emit startGame();
        close();
    });
    connect(ui->backButton, &QPushButton::clicked, this, [ = ]()
    {
        emit windowClosed();
        close();
    });
    connect(ui->mapComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeMap(int)));
}

GameModeWidget::~GameModeWidget()
{
    delete ui;
}

void GameModeWidget::changeMap(int index)
{
    m_settings->m_nCurrentMap = index;
}
