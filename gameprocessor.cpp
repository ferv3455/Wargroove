#include "gameprocessor.h"
#include "building.h"

#include <QPainterPath>
#include <QDebug>

GameProcessor::GameProcessor(Settings *settings,
                             GameInfo *gameInfo,
                             Map *map,
                             GameStats *stats,
                             TipsLabel *tipsLabel,
                             UnitSelectionWidget *unitSelectionWidget,
                             QMenu *actionContextMenu,
                             QMenu *mainContextMenu,
                             QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_gameInfo(gameInfo),
      m_map(map),
      m_stats(stats),

      m_tipsLabel(tipsLabel),
      m_unitSelectionWidget(unitSelectionWidget),
      m_actionContextMenu(actionContextMenu),
      m_mainContextMenu(mainContextMenu),

      m_nStage(0),
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
    m_mainActions[2] = new QAction(QPixmap(":/image/icon/control").scaledToHeight(30), tr("Settings"), this);
    m_mainActions[3] = new QAction(tr("Cancel"), this);

    // Initialize pointer images
    for (int i = 0; i < 5; i++)
    {
        m_pointerImage[i] = QImage(":/hightlight_pointer/" + QString::number(i));
    }

    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    // Connect signals and slots
    connect(this, &GameProcessor::enterStage, this, &GameProcessor::processStage);
    connect(m_unitMover, &UnitMover::movementFinished, this, [ = ]()
    {
        // Movement (stage 4) completes
        emit enterStage(5);
    });

    connect(m_unitSelectionWidget, &UnitSelectionWidget::confirm, this, [ = ](int unitId)
    {
        // Unit selection: unitId
        createUnit(unitId, 0);
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
        // Action: get in
        m_capturableBlocks.clear();
        m_attackableBlocks.clear();
        emit enterStage(3);
    });
    connect(m_actions[1], &QAction::triggered, this, [ = ]()
    {
        // Action: get out
        m_capturableBlocks.clear();
        m_attackableBlocks.clear();
        emit enterStage(3);
    });
    connect(m_actions[2], &QAction::triggered, this, [ = ]()
    {
        // Action: capture
        m_carrierBlocks.clear();
        m_attackableBlocks.clear();
        emit enterStage(3);
    });
    connect(m_actions[3], &QAction::triggered, this, [ = ]()
    {
        // Action: attack
        m_carrierBlocks.clear();
        m_capturableBlocks.clear();
        emit enterStage(3);
    });
    connect(m_actions[4], &QAction::triggered, this, [ = ]()
    {
        // Action: wait
        m_selectedBlock = nullptr;
        emit enterStage(4);
    });
    connect(m_actions[5], &QAction::triggered, this, [ = ]()
    {
        // Action: cancel
        emit enterStage(0);
    });

    // Main context menu
    connect(m_mainActions[0], &QAction::triggered, this, [ = ]()
    {
        // Action: End turn
        emit enterStage(16);            // WARNING: number is randomized
    });
}

void GameProcessor::paint(QPainter *painter)
{
    m_unitMover->paint(painter);

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
                block->paintPointer(painter, m_pointerImage[4]);
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

            // Paint hints
            // TODO: use widget to avoid overlapping
            pen.setWidth(5);
            painter->setPen(pen);
            painter->setBrush(QBrush(QColor(95, 95, 95, 255)));
            painter->drawRect(50, 50, 100, 150);
            m_selectedBlock->getUnit()->paint(painter, QRect(50, 50, 100, 100), 0);
            painter->drawImage(QRect(60, 160, 30, 30), QImage(":/image/icon/movement"));
            painter->drawText(120, 180, QString::number(m_nMovesLeft));
            break;
        }

        case 2:         // Choose whether to attack: show attack zone
        case 3:         // Selecting a unit to attack: show attack zone
        {
            // Paint attack zone and unit generating zone
            for (const auto &block : qAsConst(m_attackableBlocks))
            {
                Unit *unit = block->getUnit();
                if (unit != nullptr && unit->getSide() != 0)
                {
                    block->paintPointer(painter, m_pointerImage[3]);
                }
                else if (unit == nullptr)
                {
                    block->paintPointer(painter, m_pointerImage[3]);
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
                block->paintPointer(painter, m_pointerImage[4]);
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

void GameProcessor::updateAccessibleBlocks(Block *block, int unitType, int movement)
{
    int **terrainInfo = m_gameInfo->getTerrainInfo();       // move cost: terrainInfo[terrainId][unitType]
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

    struct Vertex
    {
        Block *block;
        int distance;
    };

    QVector<Vertex> remainingBlocks;
    QVector<Vertex> countedBlocks;
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
        if (temp.distance > rangeHigh)           // other blocks are all out of reach
        {
            break;
        }

        QVector<Block *> adjacentBlocks;
        m_map->getAdjacentBlocks(adjacentBlocks, temp.block);

        for (const auto &adjBlock : qAsConst(adjacentBlocks))
        {
            int distance = temp.distance + 1;
            bool counted = false;

            // find out whether the block is counted
            for (auto iter = remainingBlocks.begin(); iter != remainingBlocks.end(); iter++)
            {
                if (iter->block == adjBlock)
                {
                    // yes
                    counted = true;
                    break;
                }
            }

            if (!counted)
            {
                for (auto iter = countedBlocks.begin(); iter != countedBlocks.end(); iter++)
                {
                    if (iter->block == adjBlock)
                    {
                        // yes
                        counted = true;
                        break;
                    }
                }

                if (!counted)
                {
                    // not counted
                    remainingBlocks.push_back({adjBlock, distance});
                }
            }
        }

        countedBlocks.push_back(temp);
    }

    for (const auto &vertex : qAsConst(countedBlocks))
    {
        if (vertex.distance >= rangeLow)
        {
            // remove blocks nearer than rangeLow or without enemy units for attackable blocks
            Block *block = vertex.block;
            Unit *unit = block->getUnit();
            if (unit != nullptr && unit->getSide() > 0)
            {
                m_attackableBlocks.push_back(block);
            }
        }

        if (capture && vertex.distance == 1)
        {
            // remove blocks far away or without uncaptured buildings for capturable blocks
            Block *block = vertex.block;
            Unit *unit = block->getUnit();
            if (unit != nullptr && unit->getSide() < 0)
            {
                m_capturableBlocks.push_back(block);
            }
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
        for (const auto &adjBlock : qAsConst(nearbyBlocks))
        {
            if (adjBlock->getUnit() == nullptr)
            {
                m_carrierBlocks.push_back(adjBlock);
            }
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
    Unit *playerUnit = m_movingRoute.last()->getUnit();
    if (m_selectedBlock != nullptr)
    {
        Unit *otherUnit = m_selectedBlock->getUnit();

        if (otherUnit != nullptr)
        {
            if (otherUnit->isCarrier() && otherUnit->getSide() == 0)
            {
                // Get in the carrier
                otherUnit->setCarrier(playerUnit);
                m_movingRoute.last()->setUnit(nullptr);
            }
            else
            {
                // Choose to attack/capture
                Unit *enemyUnit = otherUnit;

                // Game info
                int **terrainInfo = m_gameInfo->getTerrainInfo();
                float **unitInfo = m_gameInfo->getUnitInfo();
                float **damageMatrix = m_gameInfo->getDamageMatrix();

                int playerId = playerUnit->getId();
                int enemyId = enemyUnit->getId();
                int playerAttack = unitInfo[playerId][5];
                int enemyAttack = unitInfo[enemyId][5];

                // Check critical hit
                double multiplier = (playerUnit->checkCritical() ? unitInfo[playerId][5] : 1);

                // Check terrain shields
                if (enemyId < 10 || enemyId > 13)
                {
                    // Enemy not an air unit
                    int shields = terrainInfo[m_selectedBlock->getTerrain()][5];
                    multiplier *= ((10.0 - shields) / 10.0);        // WARNING: TO BE CONFIRMED
                }

                // Calculate damage
                bool battleOver = false;

                int playerDamage = damageMatrix[playerId][enemyId] * multiplier * playerAttack;
                if (enemyUnit->injured(playerDamage))
                {
                    // killed
                    if (enemyUnit->getId() <= 18)
                    {
                        // not a building
                        delete enemyUnit;
                        m_selectedBlock->setUnit(nullptr);
                    }
                    else
                    {
                        // building
                        Building *building = static_cast<Building *>(enemyUnit);
                        if (building->getSide() >= 0)
                        {
                            building->setSide(-1);
                        }
                        else
                        {
                            building->regenerate(0.5);
                            building->setSide(0);
                        }
                    }

                    battleOver = true;
                }

                if (!battleOver)
                {
                    int enemyDamage = damageMatrix[enemyId][playerId] * enemyAttack / 2;
                    if (playerUnit->injured(enemyDamage))
                    {
                        delete playerUnit;
                        m_movingRoute.last()->setUnit(nullptr);
                        playerUnit = nullptr;
                    }
                }
            }
        }
        else
        {
            if (playerUnit->isCarrier())
            {
                // Get out of the carrier
                m_selectedBlock->setUnit(playerUnit->getCarrier());
                playerUnit->setCarrier(nullptr);
            }
            else
            {
                // A building generating a unit
                m_tempUnit->setActivity(false);
                m_selectedBlock->setUnit(m_tempUnit);
                m_stats->m_nCoins -= m_gameInfo->getUnitInfo()[m_tempUnit->getId()][9];
                m_tempUnit = nullptr;
            }
        }

        // Battle ends (if there is a battle)
        m_selectedBlock = nullptr;
    }

    // Invalidate the unit (if it exists)
    if (playerUnit != nullptr)
    {
        playerUnit->setActivity(false);
    }

    emit enterStage(0);
}

void GameProcessor::processStage(int stage)
{
    m_nStage = stage;
    switch (m_nStage)
    {
        // stage -1: no necessary operations

        case 0:
        {
            m_selectedBlock = nullptr;
            m_movingRoute.clear();
            m_accessibleBlocks.clear();
            m_attackableBlocks.clear();
            m_capturableBlocks.clear();
            m_carrierBlocks.clear();
            break;
        }

        case 1:
        {
            // Update accessible blocks
            int unitId = m_selectedBlock->getUnit()->getId();

            if (m_selectedBlock->getUnit()->getSide() == 0 && unitId > 18)
            {
                // Select an own building
                emit enterStage(11);
                break;
            }

            int unitType = m_gameInfo->getUnitInfo()[unitId][0];
            m_nMovesLeft = m_gameInfo->getUnitInfo()[unitId][1];
            updateAccessibleBlocks(m_selectedBlock, unitType, m_nMovesLeft);

            if (m_selectedBlock->getUnit()->getSide() != 0)
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
            break;
        }

        case 2:
        {
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

        case 11:
        {
            // Show unit selection widget
            m_unitSelectionWidget->showUnits(m_selectedBlock->getUnit()->getInnerType() - 1,
                                             m_stats->m_nCoins);
            break;
        }

        case 12:
        {
            // Update area that can be selected to generate a unit
            m_attackableBlocks.clear();
            QVector<Block *> blocks;
            m_map->getAdjacentBlocks(blocks, m_selectedBlock);
            for (const auto &adjBlock : qAsConst(blocks))
            {
                if (adjBlock->getUnit() == nullptr)
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
            // End turn: refresh all units
            QSize mapSize = m_map->getSize();
            int rows = mapSize.height();
            int cols = mapSize.width();
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    Block *block = m_map->getBlock(i, j);
                    if (block != nullptr)
                    {
                        Unit *unit = block->getUnit();
                        if (unit != nullptr)
                        {
                            unit->setActivity(true);
                            if (unit->getCarrier() != nullptr)
                            {
                                unit->getCarrier()->setActivity(true);
                            }
                        }
                    }
                }
            }

            emit enterStage(0);
            break;
        }
    }
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
                break;
            }
            if (m_movingRoute.last() != newBlock)
            {
                // cannot go there
                break;
            }

            m_selectedBlock = m_movingRoute.last();
            emit enterStage(2);
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

            // Wait: stage 2, 4, 5, 11
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
            break;
        }

        case 3:                             // Select a unit to continue
        {
            Block *newBlock = m_map->getBlock(position);
            m_cursorBlock = newBlock;
            break;
        }

            // Wait: stage 2, 4, 5, 11
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
    Q_UNUSED(position);
    switch (m_nStage)
    {
        case 0:
        {
            // TODO: Show description
            break;
        }

        case 1:
        case 2:
        case 3:
        {
            // Cancel selection
            emit enterStage(0);
            break;
        }
            // Wait: stage 4, 5, 11, 12
    }
}
