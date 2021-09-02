#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QIcon>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MainWidget),
      m_gameWidget(nullptr),
      m_nTranslatorId(1)
{
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

    ui->setupUi(this);
    setWindowIcon(QIcon(":/Wargroove.png"));

    m_pushButton_1 = new QPushButton(this);             // TODO: NEED TO BE REFINED: game add background color
    m_pushButton_1->setText(tr("Refresh"));             // cover these
    m_pushButton_1->setGeometry(0, 0, 100, 30);
    m_pushButton_2 = new QPushButton(this);
    m_pushButton_2->setText(tr("Show game"));
    m_pushButton_2->setGeometry(0, 30, 100, 30);

    connect(m_pushButton_1, &QPushButton::clicked, this, &MainWidget::retranslate);
    connect(m_pushButton_2, &QPushButton::clicked, this, &MainWidget::showGame);
}

MainWidget::~MainWidget()
{
    delete ui;
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
    m_pushButton_1->setText(tr("Refresh"));
    m_pushButton_2->setText(tr("Show game"));

    if (m_gameWidget != nullptr)
    {
        m_gameWidget->retranslate();
    }
}

void MainWidget::showGame()
{
    if (m_gameWidget != nullptr)
    {
        return;
    }

    m_gameWidget = new GameWidget(this);
    ui->mainLayout->addWidget(m_gameWidget);
    m_gameWidget->show();
}
