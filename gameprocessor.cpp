#include "gameprocessor.h"
#include "building.h"

#include <QPainterPath>
#include <QDebug>

GameProcessor::GameProcessor(Settings *settings,
                             GameInfo *gameInfo,
                             Map *map,
                             GameStats *stats,
                             QMediaPlayer *SEplayer,
                             TipsLabel *tipsLabel,
                             QWidget *moveWidget,
                             UnitSelectionWidget *unitSelectionWidget,
                             DescriptionWidget *descriptionWidget,
                             QMenu *actionContextMenu,
                             QMenu *mainContextMenu,
                             QObject *parent,
                             int movingSide,
                             int totalSides)
    : QObject(parent),
      m_settings(settings),
      m_gameInfo(gameInfo),
      m_map(map),
      m_stats(stats),
      m_SEplayer(SEplayer),

      m_tipsLabel(tipsLabel),
      m_moveWidget(moveWidget),
      m_unitSelectionWidget(unitSelectionWidget),
      m_descriptionWidget(descriptionWidget),
      m_battleWidget(nullptr),
      m_actionContextMenu(actionContextMenu),
      m_mainContextMenu(mainContextMenu),

      m_nMovingSide(movingSide),
      m_nTotalSides(totalSides),

      m_nStage(0),
      m_nRound(1),

      m_selectedBlock(nullptr),
      m_cursorBlock(nullptr),

      m_nMovesLeft(0),
      m_movingRoute(),
      m_accessibleBlocks(),
      m_attackableBlocks(),
      m_capturableBlocks(),
      m_carrierBlocks(),

      m_tempUnit(nullptr)
{
    // Initialize unit selection widgets
    m_unitSelectionWidget->loadUnits(m_gameInfo->getUnitNames(), m_gameInfo->getUnitInfo());

    // Initialize context menu actions
    m_actions[0] = new QAction(QPixmap(":/image/icon/damage").scaledToHeight(30), tr("Get in"), this);
    m_actions[1] = new QAction(QPixmap(":/image/icon/damage").scaledToHeight(30), tr("Get out"), this);
    m_actions[2] = new QAction(QPixmap(":/image/icon/commander").scaledToHeight(30), tr("Capture"), this);
    m_actions[3] = new QAction(QPixmap(":/image/icon/sword").scaledToHeight(30), tr("Attack"), this);
    m_actions[4] = new QAction(QPixmap(":/image/icon/unit").scaledToHeight(30), tr("Wait"), this);
    m_actions[5] = new QAction(tr("Cancel"), this);

    m_mainActions[0] = new QAction(QPixmap(":/image/icon/groove").scaledToHeight(30), tr("End turn"), this);
    m_mainActions[1] = new QAction(QPixmap(":/image/icon/rules").scaledToHeight(30), tr("Overview"), this);
    m_mainActions[2] = new QAction(QPixmap(":/image/icon/control").scaledToHeight(30), tr("Exit game"), this);
    m_mainActions[3] = new QAction(tr("Cancel"), this);

    // Initialize pointer images
    for (int i = 0; i < 6; i++)
    {
        m_pointerImage[i] = QImage(":/hightlight_pointer/" + QString::number(i));
    }

    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    // Connect signals and slots
    connect(this, &GameProcessor::enterStage, this, &GameProcessor::processStage);
    connect(this, &GameProcessor::endOfTurn, this, &GameProcessor::changeSide);
    connect(m_unitMover, &UnitMover::movementFinished, this, [ = ]()
    {
        // Movement (stage 4) completes
        emit enterStage(5);
    });
    connect(this, &GameProcessor::gameClosed, this, [ = ]()
    {
        // Game exit
        static_cast<QWidget *>(parent)->close();
    });

    connect(m_unitSelectionWidget, &UnitSelectionWidget::confirm, this, [ = ](int unitId)
    {
        // Unit selection: unitId
        createUnit(unitId, m_nMovingSide);
        emit enterStage(12);
    });
    connect(m_unitSelectionWidget, &UnitSelectionWidget::cancel, this, [ = ]()
    {
        // Unit selection: cancel
        emit enterStage(0);
    });

    // Action context menu
    connect(m_actions[0], &QAction::triggered, this, [ = ]()
    {
        m_capturableBlocks.clear();
        m_attackableBlocks.clear();
        emit enterStage(3);             // Action: get in
    });
    connect(m_actions[1], &QAction::triggered, this, [ = ]()
    {
        m_capturableBlocks.clear();
        m_attackableBlocks.clear();
        emit enterStage(3);             // Action: get out
    });
    connect(m_actions[2], &QAction::triggered, this, [ = ]()
    {
        m_carrierBlocks.clear();
        m_attackableBlocks.clear();
        emit enterStage(3);             // Action: capture
    });
    connect(m_actions[3], &QAction::triggered, this, [ = ]()
    {
        m_carrierBlocks.clear();
        m_capturableBlocks.clear();
        emit enterStage(3);             // Action: attack
    });
    connect(m_actions[4], &QAction::triggered, this, [ = ]()
    {
        m_selectedBlock = nullptr;
        emit enterStage(4);             // Action: wait
    });
    connect(m_actions[5], &QAction::triggered, this, [ = ]()
    {
        emit enterStage(0);             // Action: cancel
    });

    // Main context menu
    connect(m_mainActions[0], &QAction::triggered, this, [ = ]()
    {
        emit enterStage(16);            // Action: End turn
    });
    connect(m_mainActions[2], &QAction::triggered, this, [ = ]()
    {
        emit gameClosed();              // Action: Exit game
    });

    // Initialize stage
    processStage(0);
}

void GameProcessor::paint(QPainter *painter)
{
    m_unitMover->paint(painter);

    if (m_nMovingSide == 0 || m_nMovingSide == 1)       // TODO: write it in settings to decide human
    {
        switch (m_nStage)
        {
            case -1:        // Show selected enemy unit movement range
            {
                // Paint enemy unit accessible blocks
                for (const auto &block : qAsConst(m_accessibleBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[4]);
                }
                break;
            }

            case 1:         // Selecting moving route: show movement range and attack zone
            {
                // Paint accessible blocks
                for (const auto &block : qAsConst(m_accessibleBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[2]);
                }

                // Paint attack zone
                for (const auto &block : qAsConst(m_attackableBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[3]);
                }

                // Paint capturable blocks
                for (const auto &block : qAsConst(m_capturableBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[3]);
                }

                // Paint carrier-related blocks
                for (const auto &block : qAsConst(m_carrierBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[5]);
                }

                // Paint the moving route
                QPainterPath path;
                path.moveTo(m_movingRoute.first()->getCenter());
                for (auto iter = m_movingRoute.begin() + 1; iter != m_movingRoute.end(); iter++)
                {
                    path.lineTo((*iter)->getCenter());
                }

                QPen pen(QColor(255, 180, 35, 180), m_map->getBlockSize());
                pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
                painter->setPen(pen);
                painter->drawPath(path);
                break;
            }

            case 2:         // Choose whether to attack: show attack zone
            case 3:         // Selecting a unit to attack: show attack zone
            {
                // Paint attack zone and unit generating zone
                for (const auto &block : qAsConst(m_attackableBlocks))
                {
                    Unit *unit = block->getUnit();
                    if (unit != nullptr && unit->getSide() != m_nMovingSide)
                    {
                        block->paintPointer(painter, m_pointerImage[3]);
                    }
                    else if (unit == nullptr)
                    {
                        block->paintPointer(painter, m_pointerImage[5]);
                    }
                }

                // Paint capturable blocks
                for (const auto &block : qAsConst(m_capturableBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[3]);
                }

                // Paint carrier-related blocks
                for (const auto &block : qAsConst(m_carrierBlocks))
                {
                    block->paintPointer(painter, m_pointerImage[5]);
                }

                // Paint the moving route
                QPainterPath path;
                path.moveTo(m_movingRoute.first()->getCenter());
                for (auto iter = m_movingRoute.begin() + 1; iter != m_movingRoute.end(); iter++)
                {
                    path.lineTo((*iter)->getCenter());
                }

                QPen pen(QColor(255, 180, 35, 180), m_map->getBlockSize());
                pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
                painter->setPen(pen);
                painter->drawPath(path);
                break;
            }
        }

        if (m_cursorBlock != nullptr)
        {
            // Paint the block with the cursor on
            m_cursorBlock->paintPointer(painter, m_pointerImage[0]);
        }
        if (m_selectedBlock != nullptr)
        {
            // Paint the selected block
            m_selectedBlock->paintPointer(painter, m_pointerImage[1]);
        }
    }
}

void GameProcessor::updateAccessibleBlocks(Block *block, int unitType, int movement)
{
    int **terrainInfo = m_gameInfo->getTerrainInfo();  // move cost: terrainInfo[terrainId][unitType]
    m_accessibleBlocks.clear();

    struct Vertex
    {
        Block *block;
        int distance;
    };

    QVector<Vertex> remainingBlocks;
    remainingBlocks.push_back({block, 0});    // the vector only has the origin at first

    while (!remainingBlocks.isEmpty())
    {
        // sort by distance, get the one with the shortest distance
        std::sort(remainingBlocks.begin(), remainingBlocks.end(), [](const Vertex & v1, const Vertex & v2)
        {
            return v1.distance > v2.distance;
        });

        const Vertex temp = remainingBlocks.last();
        remainingBlocks.pop_back();
        if (temp.distance > movement)           // other blocks are all out of reach
        {
            break;
        }

        QVector<Block *> adjacentBlocks;
        m_map->getAdjacentBlocks(adjacentBlocks, temp.block);

        for (const auto &adjBlock : qAsConst(adjacentBlocks))
        {
            if (adjBlock->getUnit() != nullptr)
            {
                continue;
            }

            int distance = temp.distance + terrainInfo[adjBlock->getTerrain()][unitType];
            bool counted = false;

            // find out whether the block is counted
            for (auto iter = remainingBlocks.begin(); iter != remainingBlocks.end(); iter++)
            {
                if (iter->block == adjBlock)
                {
                    // yes
                    if (iter->distance > distance)
                    {
                        // update distance
                        iter->distance = distance;
                    }
                    counted = true;
                    break;
                }
            }

            if (!counted)
            {
                counted = m_accessibleBlocks.contains(adjBlock);
                if (!counted)
                {
                    // not counted
                    remainingBlocks.push_back({adjBlock, distance});
                }
            }
        }

        m_accessibleBlocks.push_back(temp.block);
    }
}

void GameProcessor::updateOperatableBlocks(Block *block, int rangeLow, int rangeHigh, bool capture)
{
    m_attackableBlocks.clear();
    m_capturableBlocks.clear();

    // Attackable
    QVector<Block *> blocksInRange;
    m_map->getAdjacentBlocks(blocksInRange, block, rangeHigh, rangeLow);

    for (const auto &block : qAsConst(blocksInRange))
    {
        // submit blocks with enemy units for attackable blocks
        Unit *unit = block->getUnit();
        if (unit != nullptr && unit->getSide() != m_nMovingSide && unit->getSide() >= 0)
        {
            m_attackableBlocks.push_back(block);
        }
    }

    if (!capture)
    {
        return;
    }

    // Capturable
    QVector<Block *> nearbyBlocks;
    m_map->getAdjacentBlocks(nearbyBlocks, block);

    for (const auto &block : qAsConst(nearbyBlocks))
    {
        // remove blocks far away or captured buildings for capturable blocks
        Unit *unit = block->getUnit();
        if (unit != nullptr && unit->getSide() < 0)
        {
            m_capturableBlocks.push_back(block);
        }
    }
}

void GameProcessor::updateCarrierBlocks(Block *block, Unit *unit)
{
    m_carrierBlocks.clear();

    if ((!unit->isCarrier() && !m_gameInfo->getUnitInfo()[unit->getId()][8]) ||
            (unit->isCarrier() && unit->getCarrier() == nullptr))
    {
        // not related or nothing in the carrier
        return;
    }

    QVector<Block *> nearbyBlocks;
    m_map->getAdjacentBlocks(nearbyBlocks, block);

    if (!unit->isCarrier())
    {
        // can be carried: search for nearby empty carriers
        for (const auto &adjBlock : qAsConst(nearbyBlocks))
        {
            if (adjBlock->getUnit() != nullptr &&
                    adjBlock->getUnit()->getSide() == m_nMovingSide &&
                    adjBlock->getUnit()->isCarrier() &&
                    adjBlock->getUnit()->getCarrier() == nullptr)
            {
                m_carrierBlocks.push_back(adjBlock);
            }
        }
    }

    else
    {
        // occupied carrier: search for nearby empty blocks to drop
        int **terrainInfo = m_gameInfo->getTerrainInfo();
        int unitType = m_gameInfo->getUnitInfo()[unit->getCarrier()->getId()][0];
        for (const auto &adjBlock : qAsConst(nearbyBlocks))
        {
            if (adjBlock->getUnit() == nullptr && terrainInfo[adjBlock->getTerrain()][unitType] < 10)
            {
                m_carrierBlocks.push_back(adjBlock);
            }
        }
    }
}

void GameProcessor::updateVisibleBlocks()
{
    // Reset visibility
    int mapRows = m_map->getSize().height();
    int mapCols = m_map->getSize().width();
    for (int i = 0; i < mapRows; i++)
    {
        for (int j = 0; j < mapCols; j++)
        {
            Block *block = m_map->getBlock(i, j);
            if (block != nullptr)
            {
                block->setVisible(false);
            }
        }
    }

    // Set own visibility
    float **unitInfo = m_gameInfo->getUnitInfo();

    QVector<Block *> ownBlocks;
    m_map->getAllBlocks(ownBlocks, m_nMovingSide);
    QVector<Block *> visibleBlocks;

    for (const auto &ownBlock : qAsConst(ownBlocks))
    {
        int unitId = ownBlock->getUnit()->getId();
        m_map->getAdjacentBlocks(visibleBlocks, ownBlock, unitInfo[unitId][4]);
        for (const auto &block : qAsConst(visibleBlocks))
        {
            block->setVisible(true);
        }
    }
}

void GameProcessor::createUnit(int unitId, int side)
{
    if (m_tempUnit != nullptr)
    {
        delete m_tempUnit;
    }
    int maxHP = m_gameInfo->getUnitInfo()[unitId][6];
    m_tempUnit = new Unit(unitId, side, maxHP, m_map);
}

void GameProcessor::confrontUnit()
{
    // check whether there is a battle
    bool battle = false;
    Unit *activeUnit = m_movingRoute.last()->getUnit();
    if (m_selectedBlock != nullptr)
    {
        Unit *passiveUnit = m_selectedBlock->getUnit();

        if (passiveUnit != nullptr)
        {
            if (passiveUnit->isCarrier() && passiveUnit->getSide() == m_nMovingSide)
            {
                // Get in the carrier
                passiveUnit->setCarrier(activeUnit);
                m_movingRoute.last()->setUnit(nullptr);
            }
            else
            {
                m_battleWidget = new BattleWidget(m_movingRoute.last(), m_selectedBlock,
                                                  m_gameInfo, static_cast<QWidget *>(parent()));
                battle = true;
                connect(m_battleWidget, &BattleWidget::end, this, [ = ]()
                {
                    // Battle animation ends
                    emit enterStage(8);
                });
                m_battleWidget->begin();

                // Attack/Capture
                int **terrainInfo = m_gameInfo->getTerrainInfo();
                float **unitInfo = m_gameInfo->getUnitInfo();
                float **damageMatrix = m_gameInfo->getDamageMatrix();

                int activeId = activeUnit->getId();
                int passiveId = passiveUnit->getId();
                int activeAttack = unitInfo[activeId][5];
                int passiveAttack = unitInfo[passiveId][5];

                // Check critical hit
                bool critical = activeUnit->checkCritical();
                double multiplier = (critical ? unitInfo[activeId][11] : 1.0);

                // Check terrain shields
                if (passiveId < 10 || passiveId > 13)
                {
                    // Enemy not an air unit
                    int shields = terrainInfo[m_selectedBlock->getTerrain()][5];
                    multiplier *= ((10.0 - shields) / 10.0);
                }

                // Calculate damage
                bool killed = false;

                double activeRandom = (std::rand() % 10 + 95) / 100.0;
                int activeDamage = damageMatrix[activeId][passiveId] * multiplier * activeAttack * activeRandom;
                killed = passiveUnit->injured(activeDamage);
                m_battleWidget->setActiveAttack(activeDamage, killed, critical);

                if (killed)
                {
                    // killed
                    if (passiveUnit->getId() <= 18)
                    {
                        // not a building
                        if (passiveUnit->getCarrier() != nullptr)
                        {
                            // is a carrier and with a unit in it
                            m_stats->removeUnit(passiveUnit->getCarrier());
                            delete passiveUnit->getCarrier();
                        }

                        m_stats->removeUnit(passiveUnit);
                        delete passiveUnit;
                        m_selectedBlock->setUnit(nullptr);
                    }
                    else
                    {
                        // building
                        Building *building = static_cast<Building *>(passiveUnit);
                        if (building->getSide() >= 0)
                        {
                            m_stats->removeUnit(building);
                            building->setSide(-1);
                        }
                        else
                        {
                            building->regenerate(0.5);
                            building->setSide(m_nMovingSide);
                            building->setActivity(false);
                            m_stats->addUnit(building);
                        }
                    }
                }

                if (!killed)
                {
                    // Find out if they are adjacent blocks
                    QVector<Block *> v;
                    m_map->getAdjacentBlocks(v, m_movingRoute.last());
                    if (v.contains(m_selectedBlock))
                    {
                        double passiveRandom = (std::rand() % 10 + 95) / 100.0;
                        int passiveDamage = damageMatrix[passiveId][activeId] * passiveAttack * passiveRandom / 2;
                        killed = activeUnit->injured(passiveDamage);
                        m_battleWidget->setPassiveAttack(passiveDamage, killed);
                        if (killed)
                        {
                            m_stats->removeUnit(activeUnit);
                            delete activeUnit;
                            m_movingRoute.last()->setUnit(nullptr);
                            activeUnit = nullptr;
                        }
                    }
                }
            }
        }
        else
        {
            if (activeUnit->isCarrier())
            {
                // Get out of the carrier
                m_selectedBlock->setUnit(activeUnit->getCarrier());
                activeUnit->setCarrier(nullptr);
            }
            else
            {
                // A building generating a unit
                m_tempUnit->setActivity(false);
                m_selectedBlock->setUnit(m_tempUnit);
                m_stats->addUnit(m_tempUnit);
                m_stats->addCoins(-m_gameInfo->getUnitInfo()[m_tempUnit->getId()][9], m_nMovingSide);
                m_tempUnit = nullptr;
            }
        }

        // Battle ends (if there is a battle)
        m_selectedBlock = nullptr;
    }

    // Invalidate the unit (if it exists)
    if (activeUnit != nullptr)
    {
        activeUnit->setActivity(false);
    }

    if (!battle)
    {
        // no battles, no need to wait
        emit enterStage(0);
    }
}

void GameProcessor::checkWin()
{
    qDebug() << m_stats->getUnits(0).size() << m_stats->getUnits(1).size();

    // Check commanders and strongholds
    bool foundCommander = false;
    bool foundStronghold = false;
    for (const auto &unit : qAsConst(m_stats->getUnits(1)))
    {
        if (unit->getId() == 18)
        {
            foundCommander = true;
        }

        if (unit->getId() == 19 && unit->getInnerType() == 0)
        {
            foundStronghold = true;
        }
    }

    if (!foundCommander && !foundStronghold)
    {
        // Check win
        parent()->findChild<QLabel *>("gameOverLabel")->setText(tr("Game over! You win! "));
        emit enterStage(99);
        return;
    }

    foundCommander = false;
    foundStronghold = false;
    for (const auto &unit : qAsConst(m_stats->getUnits(0)))
    {
        if (unit->getId() == 18)
        {
            foundCommander = true;
        }

        if (unit->getId() == 19 && unit->getInnerType() == 0)
        {
            foundStronghold = true;
        }
    }

    if (!foundCommander && !foundStronghold)
    {
        // Check lose
        parent()->findChild<QLabel *>("gameOverLabel")->setText(tr("Game over! You lose! "));
        emit enterStage(99);
        return;
    }
}

void GameProcessor::processStage(int stage)
{
    m_nStage = stage;
    qDebug() << "Enter stage" << m_nStage;

    switch (m_nStage)
    {
        // stage -1: no necessary operations

        case 0:
        {
            if (m_settings->m_bFogMode)
            {
                updateVisibleBlocks();
            }
            m_selectedBlock = nullptr;
            m_movingRoute.clear();
            m_accessibleBlocks.clear();
            m_attackableBlocks.clear();
            m_capturableBlocks.clear();
            m_carrierBlocks.clear();
            checkWin();
            break;
        }

        case 1:
        {
            // Update accessible blocks
            int unitId = m_selectedBlock->getUnit()->getId();

            if (m_selectedBlock->getUnit()->getSide() == m_nMovingSide && unitId > 18)
            {
                // Select an own building
                emit enterStage(11);
                break;
            }

            int unitType = m_gameInfo->getUnitInfo()[unitId][0];
            m_nMovesLeft = m_gameInfo->getUnitInfo()[unitId][1];
            updateAccessibleBlocks(m_selectedBlock, unitType, m_nMovesLeft);

            if (m_selectedBlock->getUnit()->getSide() != m_nMovingSide)
            {
                // Select an enemy unit
                emit enterStage(-1);
                break;
            }

            m_movingRoute.push_back(m_selectedBlock);
            updateOperatableBlocks(m_selectedBlock,
                                   m_gameInfo->getUnitInfo()[unitId][2],
                                   m_gameInfo->getUnitInfo()[unitId][3],
                                   m_gameInfo->getUnitInfo()[unitId][7]);
            updateCarrierBlocks(m_selectedBlock, m_selectedBlock->getUnit());

            // Show move widget
            QVector<QImage> unitImages;
            m_selectedBlock->getUnit()->getImages(&unitImages);
            m_moveWidget->findChild<QLabel *>("unitPicLabel")->setPixmap(QPixmap::fromImage(unitImages[0]));
            m_moveWidget->findChild<QLabel *>("unitMoveLabel")->setText(QString::number(m_nMovesLeft));
            m_moveWidget->show();
            break;
        }

        case 2:
        {
            // Hide move widget
            m_moveWidget->hide();

            // Show context menu
            m_actionContextMenu->clear();
            if (!m_carrierBlocks.isEmpty())
            {
                if (m_movingRoute.first()->getUnit()->isCarrier())
                {
                    m_actionContextMenu->addAction(m_actions[1]);
                }
                else
                {
                    m_actionContextMenu->addAction(m_actions[0]);
                }
            }
            if (!m_capturableBlocks.isEmpty())
            {
                m_actionContextMenu->addAction(m_actions[2]);
            }
            if (!m_attackableBlocks.isEmpty())
            {
                m_actionContextMenu->addAction(m_actions[3]);
            }
            m_actionContextMenu->addAction(m_actions[4]);
            m_actionContextMenu->addAction(m_actions[5]);
            m_actionContextMenu->exec(QCursor::pos());
            break;
        }

        // stage 3: no necessary operations

        case 4:
        {
            // Hide move widget
            m_moveWidget->hide();

            // Begin moving
            m_unitMover->moveUnit(m_movingRoute);
            break;
        }

        case 5:
        {
            // Battle field or other operation
            confrontUnit();
            break;
        }

        case 8:
        {
            // Battle ends
            delete m_battleWidget;

            emit enterStage(0);

            break;
        }

        case 11:
        {
            // Show unit selection widget
            m_unitSelectionWidget->showUnits(m_selectedBlock->getUnit()->getInnerType() - 1,
                                             m_stats->getCoins(m_nMovingSide));
            break;
        }

        case 12:
        {
            // Update area that can be selected to generate a unit
            m_attackableBlocks.clear();
            QVector<Block *> blocks;
            m_map->getAdjacentBlocks(blocks, m_selectedBlock);
            int **terrainInfo = m_gameInfo->getTerrainInfo();
            int unitType = m_gameInfo->getUnitInfo()[m_tempUnit->getId()][0];
            for (const auto &adjBlock : qAsConst(blocks))
            {
                if (adjBlock->getUnit() == nullptr &&
                        terrainInfo[adjBlock->getTerrain()][unitType] < 10)
                {
                    m_attackableBlocks.push_back(adjBlock);
                }
            }

            // Clear other block vectors (for stage 3)
            m_capturableBlocks.clear();
            m_carrierBlocks.clear();

            // Define move route (for stage 5)
            m_movingRoute.push_back(m_selectedBlock);

            emit enterStage(3);
            break;
        }

        case 16:
        {
            // End turn
            // Refresh all units, regenerate buildings, add coins
            for (const auto &unit : m_stats->getUnits(m_nMovingSide))
            {
                // Refresh
                unit->setActivity(true);
                if (unit->getCarrier() != nullptr)
                {
                    unit->getCarrier()->setActivity(true);
                }

                // Regenerate buildings
                if (unit->getId() > 18)
                {
                    unit->regenerate(0.1);
                    if (unit->getInnerType() >= 4 || unit->getInnerType() == 0)
                    {
                        // Village/Base: coins + 100
                        m_stats->addCoins(100, m_nMovingSide);
                    }
                }
            }

            emit endOfTurn();
            break;
        }

        // stage 20: no neccessary operations

        case 99:
        {
            // Game over
            parent()->findChild<QLabel *>("gameOverLabel")->show();
            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);
            connect(timer, &QTimer::timeout, this, &GameProcessor::gameClosed);
            timer->start(3000);
        }
    }
}

void GameProcessor::changeSide()
{
    qDebug() << "=============Round" << m_nRound << "Side" << m_nMovingSide << "finished===============";
    m_nMovingSide++;
    if (m_nMovingSide >= m_nTotalSides)
    {
        m_nMovingSide = 0;
        m_nRound++;
    }

    m_tipsLabel->popup(tr("Round ") + QString::number(m_nRound) +
                       tr(": Side ") + QString::number(m_nMovingSide + 1));
    emit enterStage(0);
    emit roundForSide(m_nMovingSide);
}

void GameProcessor::moveMap(QPoint deltaPos)
{
    if (m_nStage == 11)
    {
        return;
    }
    m_map->adjustOffset(deltaPos);
}

void GameProcessor::zoomMap(int direction, QPointF point)
{
    if (m_nStage == 11)
    {
        return;
    }
    m_map->adjustScale(direction * m_settings->m_nZoomScale, point);
    m_tipsLabel->popup(tr("Map Scale: ") + QString::number(m_map->getScale()) + "\%");
}

void GameProcessor::selectPosition(QPoint position)
{
    m_SEplayer->play();

    switch (m_nStage)           // Check stage first
    {
        case 0:                             // Select unit
        {
            Block *newBlock = m_map->getBlock(position);
            if (newBlock == nullptr)
            {
                // Cannot select a "no" block
                break;
            }
            if (!newBlock->isVisible())
            {
                // Cannot select an invisible block
                break;
            }

            if (newBlock->getUnit() == nullptr)
            {
                // Select an empty block: pop up main context menu
                m_mainContextMenu->clear();
                m_mainContextMenu->addAction(m_mainActions[0]);
                m_mainContextMenu->addAction(m_mainActions[1]);
                m_mainContextMenu->addAction(m_mainActions[2]);
                m_mainContextMenu->addAction(m_mainActions[3]);
                m_mainContextMenu->exec(QCursor::pos());
                break;
            }

            if (!newBlock->getUnit()->isOperable())
            {
                // Cannot operate this unit
                break;
            }

            if (!newBlock->getUnit()->getActivity())
            {
                // Cannot operate an inactive unit
                break;
            }

            m_selectedBlock = newBlock;
            emit enterStage(1);
            break;
        }

        case 1:                             // Select moving route
        {
            Block *newBlock = m_map->getBlock(position);
            if (newBlock == nullptr)
            {
                // Cannot select an empty block
                break;
            }
            if (m_unitMover->isBusy())
            {
                // Cannot move now
                break;
            }

            if (m_movingRoute.last() != newBlock)
            {
                // cannot go there
                if (m_attackableBlocks.contains(newBlock) ||
                        m_capturableBlocks.contains(newBlock) ||
                        m_carrierBlocks.contains(newBlock))
                {
                    // skip stage 2 & 3, move directly
                    m_selectedBlock = newBlock;
                    emit enterStage(4);
                    break;
                }
                break;
            }

            m_selectedBlock = m_movingRoute.last();
            emit enterStage(2);
            break;
        }

        case 2:                             // Cancel selection
        {
            emit enterStage(0);
            break;
        }

        case 3:                             // Select a unit to continue
        {
            Block *newBlock = m_map->getBlock(position);
            if (!m_attackableBlocks.contains(newBlock) &&
                    !m_capturableBlocks.contains(newBlock) &&
                    !m_carrierBlocks.contains(newBlock))
            {
                // Cannot operate the block
                break;
            }

            m_selectedBlock = newBlock;
            emit enterStage(4);
            break;
        }

        case 20:                             // Back to game
        {
            m_descriptionWidget->closeWindow();
            emit enterStage(0);
            break;
        }
            // Wait: stage 2, 4, 5, 8, 11, 16, 25
    }
}

void GameProcessor::mouseToPosition(QPoint position)
{
    switch (m_nStage)           // Check stage first
    {
        case 0:
        {
            Block *newBlock = m_map->getBlock(position);
            m_cursorBlock = newBlock;
            break;
        }

        case 1:                             // Select moving route
        {
            Block *newBlock = m_map->getBlock(position);
            if (newBlock == m_cursorBlock)
            {
                // Cannot move to the same block
                break;
            }

            m_cursorBlock = newBlock;
            if (m_cursorBlock == nullptr)
            {
                // Cannot move to a "no" block
                break;
            }

            if (m_cursorBlock->getUnit() != nullptr &&
                    m_cursorBlock->getUnit() != m_selectedBlock->getUnit())
            {
                // destination isn't empty and isn't the starting point
                break;
            }

            int idx = m_movingRoute.indexOf(m_cursorBlock);
            if (idx >= 0)
            {
                // block reselected: undo selection
                while (m_movingRoute.size() > idx + 1)
                {
                    int terrainId = m_movingRoute.last()->getTerrain();
                    int unitType = m_gameInfo->getUnitInfo()[m_selectedBlock->getUnit()->getId()][0];
                    int moveCost = m_gameInfo->getTerrainInfo()[terrainId][unitType];
                    m_nMovesLeft += moveCost;
                    m_movingRoute.pop_back();
                }

                int unitId = m_selectedBlock->getUnit()->getId();
                updateOperatableBlocks(m_cursorBlock,
                                       m_gameInfo->getUnitInfo()[unitId][2],
                                       m_gameInfo->getUnitInfo()[unitId][3],
                                       m_gameInfo->getUnitInfo()[unitId][7]);
                updateCarrierBlocks(m_cursorBlock, m_selectedBlock->getUnit());
                m_moveWidget->findChild<QLabel *>("unitMoveLabel")->setText(QString::number(m_nMovesLeft));
                break;
            }

            QVector<Block *> v;
            m_map->getAdjacentBlocks(v, m_cursorBlock);
            if (!v.contains(m_movingRoute.last()))
            {
                // not adjacent
                break;
            }

            int terrainId = m_cursorBlock->getTerrain();
            int unitId = m_selectedBlock->getUnit()->getId();
            int unitType = m_gameInfo->getUnitInfo()[unitId][0];
            int moveCost = m_gameInfo->getTerrainInfo()[terrainId][unitType];
            if (moveCost > m_nMovesLeft)
            {
                break;
            }

            m_nMovesLeft -= moveCost;
            m_movingRoute.push_back(m_cursorBlock);
            updateOperatableBlocks(m_cursorBlock,
                                   m_gameInfo->getUnitInfo()[unitId][2],
                                   m_gameInfo->getUnitInfo()[unitId][3],
                                   m_gameInfo->getUnitInfo()[unitId][7]);
            updateCarrierBlocks(m_cursorBlock, m_selectedBlock->getUnit());
            m_moveWidget->findChild<QLabel *>("unitMoveLabel")->setText(QString::number(m_nMovesLeft));
            break;
        }

        case 3:                             // Select a unit to continue
        {
            Block *newBlock = m_map->getBlock(position);
            m_cursorBlock = newBlock;
            break;
        }

            // Wait: stage 2, 4, 5, 8, 11, 16, 20, 25
    }
}

void GameProcessor::unselectPosition(QPoint position)
{
    Q_UNUSED(position);
    if (m_nStage == -1)
    {
        emit enterStage(0);
    }
}

void GameProcessor::escapeMenu(QPoint position)
{
    m_SEplayer->play();

    Q_UNUSED(position);
    switch (m_nStage)
    {
        case 0:
        {
            // Show description
            Block *newBlock = m_map->getBlock(position);
            if (newBlock == nullptr)
            {
                // Cannot select a "no" block
                break;
            }
            if (!newBlock->isVisible())
            {
                // Cannot select an invisible block
                break;
            }

            m_descriptionWidget->setBlock(m_map->getBlock(position));
            emit enterStage(20);
            break;
        }

        case 1:
        {
            // Hide move widget
            m_moveWidget->hide();

            // Cancel selection
            emit enterStage(0);
            break;
        }

        case 2:
        case 3:
        {
            // Cancel selection
            emit enterStage(0);
            break;
        }
            // Wait: stage 4, 5, 8, 11, 12, 16, 20, 25
    }
}
