#include "descriptionwidget.h"
#include "ui_descriptionwidget.h"

#include <QDebug>

DescriptionWidget::DescriptionWidget(GameInfo *gameInfo, QWidget *parent, bool selectUnits)
    : QWidget(parent),
      ui(new Ui::DescriptionWidget),
      m_gameInfo(gameInfo),
      m_bSelectUnits(selectUnits),
      m_nDynamicsId(0)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->widget->setAttribute(Qt::WA_StyledBackground, true);

    if (m_bSelectUnits)
    {
        ui->terrainMainWidget->hide();
    }

    hide();

    m_displayingBlock = new Block(1, 0, 0, this);
    m_unitPictureLabel = ui->unitPictureLabel;
    m_unitImages = new QImage[2]();

    ui->interunitsLayout->setColumnStretch(0, 2);
    m_interunitLabels = new QLabel **[3];
    for (int i = 0; i < 3; i++)
    {
        m_interunitLabels[i] = new QLabel *[7];
        for (int j = 0; j < 7; j++)
        {
            m_interunitLabels[i][j] = new QLabel(this);
            m_interunitLabels[i][j]->setFixedSize(33, 33);
            m_interunitLabels[i][j]->setPixmap(QPixmap());
            ui->interunitsLayout->addWidget(m_interunitLabels[i][j], i + 1, j + 1);
            ui->interunitsLayout->setColumnStretch(j + 1, 1);
        }
    }

    // Initialize timers
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &DescriptionWidget::updateUnitPicture);
}

DescriptionWidget::~DescriptionWidget()
{
    delete ui;
}

void DescriptionWidget::setUnit(const Unit *unit, int terrain)
{
    m_displayingBlock->setTerrain(terrain);
    if (m_displayingBlock->getUnit() != nullptr)
    {
        delete m_displayingBlock->getUnit();
    }

    if (unit != nullptr)
    {
        // Copy a unit
        m_displayingBlock->setUnit(unit->getId(), unit->getSide(),
                                   m_gameInfo->getUnitInfo()[unit->getId()][6],
                                   unit->getInnerType());
        m_nDynamicsId = 0;
    }
    else
    {
        m_displayingBlock->setUnit(nullptr);
    }

    updateScreen();
    adjustBackground();
    show();

    m_refreshTimer->start(300);
}

void DescriptionWidget::setBlock(const Block *block)
{
    setUnit(block->getUnit(), block->getTerrain());
}

void DescriptionWidget::adjustBackground()
{
    // Initial visible condition
    bool visible = isVisible();

    if (!m_bSelectUnits)
    {
        // Reset size
        int parentWidth = static_cast<QWidget *>(parent())->width();
        int parentHeight = static_cast<QWidget *>(parent())->height();
        setGeometry(parentWidth / 2 - 500, parentHeight / 2 - 350, 1000, 700);
    }

    if (m_displayingBlock->getUnit() != nullptr)
    {
        m_unitPictureLabel->setPixmap(QPixmap::fromImage(m_unitImages[m_nDynamicsId].scaledToHeight(ui->widget->height() / 3)));
    }

    // show() has to be before loading background ?
    show();

    // Reload background picture
    QImage img;
    Unit *unit = m_displayingBlock->getUnit();
    if (unit != nullptr)
    {
        int unitId = unit->getId();
        if (unitId >= 10 && unitId <= 13)
        {
            // Air unit
            img = QImage(":/image/background/0");
        }
        else
        {
            img = QImage(":/image/background/" + QString::number(m_displayingBlock->getTerrain()));
        }
    }
    else
    {
        img = QImage(":/image/background/" + QString::number(m_displayingBlock->getTerrain()));
    }

    // Set background picture
    QWidget *displayWidget = ui->widget;
    QPalette palette = displayWidget->palette();
    palette.setBrush(displayWidget->backgroundRole(),
                     QBrush(img.scaled(displayWidget->size(),
                                       Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    displayWidget->setPalette(palette);

    // Return to initial condition
    setVisible(visible);
}

void DescriptionWidget::updateScreen()
{
    if (!m_bSelectUnits)
    {
        // Terrain info
        int **terrainInfo = m_gameInfo->getTerrainInfo();
        int terrain = m_displayingBlock->getTerrain();
        ui->moveLabel1->setText(terrainInfo[terrain][0] < 10 ? QString::number(terrainInfo[terrain][0]) : "-");
        ui->moveLabel2->setText(terrainInfo[terrain][1] < 10 ? QString::number(terrainInfo[terrain][1]) : "-");
        ui->moveLabel3->setText(terrainInfo[terrain][2] < 10 ? QString::number(terrainInfo[terrain][2]) : "-");
        ui->moveLabel4->setText(terrainInfo[terrain][3] < 10 ? QString::number(terrainInfo[terrain][3]) : "-");
        ui->moveLabel5->setText(terrainInfo[terrain][4] < 10 ? QString::number(terrainInfo[terrain][4]) : "-");
        ui->defenceLabel->setText(QString::number(terrainInfo[terrain][5]));
        ui->terrainLabel->setText(m_gameInfo->getTerrainNames()[terrain]);
    }

    // Unit info
    Unit *unit = m_displayingBlock->getUnit();
    if (unit != nullptr)
    {
        // Relative game data
        int unitId = unit->getId();
        float **unitInfo = m_gameInfo->getUnitInfo();

        // Set unit names
        ui->unitNameLabel->show();
        if (unitId < 18)
        {
            // ordinary units
            ui->unitNameLabel->setText(m_gameInfo->getUnitNames()[unitId]);
        }
        else if (unitId == 18)
        {
            // commander
            ui->unitNameLabel->setText(m_gameInfo->getCommanderNames()[3 * unit->getSide() + unit->getInnerType()]);
        }
        else
        {
            // building
            ui->unitNameLabel->setText(m_gameInfo->getBuildingNames()[unit->getInnerType()]);
        }

        // Set unit icon
        ui->unitIconLabel->show();
        ui->unitIconLabel->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(unitId)));

        // Set unit picture
        QVector<QImage> v;
        unit->getImages(&v);
        for (int i = 0; i < 2; i++)
        {
            m_unitImages[i] = v[i];
        }
        m_unitPictureLabel->show();
        m_unitPictureLabel->setPixmap(QPixmap::fromImage(m_unitImages[m_nDynamicsId].scaledToHeight(ui->widget->height() / 3)));

        // Set battle data
        ui->unitBattleDataWidget->show();
        ui->attackLabel->setText(QString::number(unitInfo[unitId][5]));
        ui->maxHPLabel->setText(QString::number(unit->getMaxHP()));

        // Set unit data widget
        ui->unitDataWidget->show();

        //  Set moving type and costs
        ui->moveTypeLabel->setPixmap(QPixmap(":/image/move_icon/" + QString::number(unitInfo[unitId][0])));
        ui->moveLabel->setText(QString::number(unitInfo[unitId][1]));

        //  Set range
        int rangeLow = unitInfo[unitId][2];
        int rangeHigh = unitInfo[unitId][3];
        if (rangeLow == rangeHigh)
        {
            ui->rangeLabel->setText(QString::number(rangeLow));
        }
        else
        {
            ui->rangeLabel->setText(QString("%1-%2").arg(rangeLow).arg(rangeHigh));
        }

        //  Set sight
        ui->sightLabel->setText(QString::number(unitInfo[unitId][4]));

        // Set description
        ui->descriptionLabel->show();
        if (unitId < 18)
        {
            // ordinary units
            ui->descriptionLabel->setText(m_gameInfo->getUnitDescription()[unitId]);
        }
        else if (unitId == 18)
        {
            // commander
            ui->descriptionLabel->setText(m_gameInfo->getCommanderDescription()[0]);
        }
        else
        {
            // building
            ui->descriptionLabel->setText(m_gameInfo->getBuildingDescription()[unit->getInnerType()]);
        }

        // Set effective and vulnerable box
        float **damageMatrix = m_gameInfo->getDamageMatrix();
        // Effective units
        int effIndex = 0;
        for (int i = 0; i < 20; i++)
        {
            if (effIndex >= 7)
            {
                break;
            }
            if (damageMatrix[unitId][i] > 1.4)
            {
                m_interunitLabels[0][effIndex]->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(i)));
                m_interunitLabels[0][effIndex]->setStyleSheet("background-color: #20918b;");
                effIndex++;
            }
        }
        for (int i = 0; i < 20; i++)
        {
            if (effIndex >= 7)
            {
                break;
            }
            if (damageMatrix[unitId][i] < 1.4 && damageMatrix[unitId][i] > 1.0)
            {
                m_interunitLabels[0][effIndex]->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(i)));
                m_interunitLabels[0][effIndex]->setStyleSheet("background-color: #335eb0;");
                effIndex++;

            }
        }
        for (; effIndex < 7; effIndex++)
        {
            m_interunitLabels[0][effIndex]->setPixmap(QPixmap());
            m_interunitLabels[0][effIndex]->setStyleSheet("");
        }

        // Void units
        int voidIndex = 0;
        for (int i = 0; i < 20; i++)
        {
            if (voidIndex >= 7)
            {
                break;
            }
            if (damageMatrix[unitId][i] < 0.4)
            {
                m_interunitLabels[1][voidIndex]->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(i)));
                m_interunitLabels[1][voidIndex]->setStyleSheet("background-color: #a4802b;");
                voidIndex++;
            }
        }
        for (int i = 0; i < 20; i++)
        {
            if (voidIndex >= 7)
            {
                break;
            }
            if (damageMatrix[unitId][i] > 0.4 && damageMatrix[unitId][i] < 0.7)
            {
                m_interunitLabels[1][voidIndex]->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(i)));
                m_interunitLabels[1][voidIndex]->setStyleSheet("background-color: #eac282;");
                voidIndex++;
            }
        }
        for (; voidIndex < 7; voidIndex++)
        {
            m_interunitLabels[1][voidIndex]->setPixmap(QPixmap());
            m_interunitLabels[1][voidIndex]->setStyleSheet("");
        }

        // Vulnerable units
        int vulIndex = 0;
        for (int i = 0; i < 20; i++)
        {
            if (vulIndex >= 7)
            {
                break;
            }
            if (damageMatrix[i][unitId] > 1.4)
            {
                m_interunitLabels[2][vulIndex]->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(i)));
                m_interunitLabels[2][vulIndex]->setStyleSheet("background-color: #aa003f;");
                vulIndex++;
            }
        }
        for (int i = 0; i < 20; i++)
        {
            if (vulIndex >= 7)
            {
                break;
            }
            if (damageMatrix[i][unitId] < 1.4 && damageMatrix[i][unitId] > 1.0)
            {
                m_interunitLabels[2][vulIndex]->setPixmap(QPixmap(":/image/unit_icon/" + QString::number(i)));
                m_interunitLabels[2][vulIndex]->setStyleSheet("background-color: #cd4c18;");
                vulIndex++;
            }
        }
        for (; vulIndex < 7; vulIndex++)
        {
            m_interunitLabels[2][vulIndex]->setPixmap(QPixmap());
            m_interunitLabels[2][vulIndex]->setStyleSheet("");
        }

        ui->interunitsWidget->show();
    }
    else
    {
        ui->unitNameLabel->hide();
        ui->unitIconLabel->hide();
        ui->unitBattleDataWidget->hide();
        ui->unitDataWidget->hide();
        ui->descriptionLabel->hide();
        m_unitPictureLabel->hide();
        ui->interunitsWidget->hide();
    }
}

void DescriptionWidget::closeWindow()
{
    hide();
    m_refreshTimer->stop();
}

void DescriptionWidget::updateUnitPicture()
{
    if (m_displayingBlock->getUnit() != nullptr)
    {
        m_nDynamicsId = 1 - m_nDynamicsId;
        m_unitPictureLabel->setPixmap(QPixmap::fromImage(m_unitImages[m_nDynamicsId].scaledToHeight(ui->widget->height() / 3)));
    }
}
