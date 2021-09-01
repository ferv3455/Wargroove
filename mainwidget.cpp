#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QIcon>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Wargroove.png"));

    m_gameWidget = new GameWidget(this);
    ui->mainLayout->addWidget(m_gameWidget);
}

MainWidget::~MainWidget()
{
    delete ui;
}
