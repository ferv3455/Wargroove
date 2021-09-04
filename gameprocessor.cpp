#include "gameprocessor.h"

#include <QPainterPath>
#include <QDebug>

GameProcessor::GameProcessor(Settings *settings,
                             GameInfo *gameInfo,
                             Map *map,
                             TipsLabel *tipsLabel,
                             QMenu *contextMenu,
                             QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_gameInfo(gameInfo),
      m_map(map),

      m_tipsLabel(tipsLabel),
      m_contextMenu(contextMenu),

      m_nStage(0),
      m_selectedBlock(nullptr),
      m_cursorBlock(nullptr),

      m_nMovesLeft(0),
      m_movingRoute(),
      m_accessibleBlocks(),
      m_attackableBlocks()
{
    // Initialize context menu actions
    m_actions[0] = new QAction(tr("Attack"), this);
    m_actions[0]->setIcon(QIcon(":/image/icon/unit"));
    m_actions[1] = new QAction(tr("Wait"), this);
    m_actions[2] = new QAction(tr("Cancel"), this);

    // Initialize pointer images
    for (int i = 0; i < 5; i++)
    {
        m_pointerImage[i] = QImage(":/hightlight_pointer/" + QString::number(i));
    }

    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    // Connect signals and slots
    connect(this, &GameProcessor::enterStage, this, &GameProcessor::processStage);

    connect(m_actions[0], &QAction::triggered, this, [ = ]()
    {
        // Action: attack
        emit enterStage(3);
    });
    connect(m_actions[1], &QAction::triggered, this, [ = ]()
    {
        // Action: wait
        m_selectedBlock = nullptr;
        emit enterStage(4);
    });
    connect(m_actions[2], &QAction::triggered, this, [ = ]()
    {
        // Action: cancel
        emit enterStage(0);
    });

    connect(m_unitMover, &UnitMover::movementFinished, this, [ = ]()
    {
        // Movement (stage 4) completes
        emit enterStage(5);
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
            // Paint attack zone
            for (const auto &block : qAsConst(m_attackableBlocks))
            {
                Unit *unit = block->getUnit();
                if (unit != nullptr && unit->getSide() != 0)
                {
                    block->paintPointer(painter, m_pointerImage[3]);
                }
            }
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

void GameProcessor::updateAttackableBlocks(Block *block, int rangeLow, int rangeHigh)
{
    m_attackableBlocks.clear();

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

    // remove blocks nearer than rangeLow or without enemy units
    for (const auto &vertex : qAsConst(countedBlocks))
    {
        if (vertex.distance >= rangeLow)
        {
            Block *block = vertex.block;
            Unit *unit = block->getUnit();
            if (unit != nullptr && unit->getSide() != 0)
            {
                m_attackableBlocks.push_back(block);
            }
        }
    }
}

void GameProcessor::battle()
{
    Unit *playerUnit = m_movingRoute.last()->getUnit();
    if (m_selectedBlock != nullptr)
    {
        // Choose to attack
        Unit *enemyUnit = m_selectedBlock->getUnit();
        if (enemyUnit != nullptr)
        {
            // SHOULD GO IN HERE
            // TODO: BATTLE HERE
            delete enemyUnit;
            m_selectedBlock->setUnit(nullptr);
        }

        // Battle ends (if there is a battle)
        m_selectedBlock = nullptr;
    }

    // Invalidate the unit
    playerUnit->setActivity(false);
    emit enterStage(0);
}

void GameProcessor::processStage(int stage)
{
    m_nStage = stage;
    switch (m_nStage)
    {
        case -1:
        {
            break;
        }

        case 0:
        {
            m_selectedBlock = nullptr;
            m_movingRoute.clear();
            break;
        }

        case 1:
        {
            // Update accessible blocks
            int unitId = m_selectedBlock->getUnit()->getId();
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
            updateAttackableBlocks(m_selectedBlock, m_gameInfo->getUnitInfo()[unitId][2],
                                   m_gameInfo->getUnitInfo()[unitId][3]);
            break;
        }

        case 2:
        {
            // Show context menu
            m_contextMenu->clear();
            if (!m_attackableBlocks.isEmpty())
            {
                m_contextMenu->addAction(m_actions[0]);
            }
            m_contextMenu->addAction(m_actions[1]);
            m_contextMenu->addAction(m_actions[2]);
            m_contextMenu->exec(QCursor::pos());
            break;
        }

        case 3:
        {
            break;
        }

        case 4:
        {
            // Begin moving
            m_unitMover->moveUnit(m_movingRoute);
            break;
        }

        case 5:
        {
            // Battle field
            battle();               // TODO: REFINED
            break;
        }
    }
}

void GameProcessor::moveMap(QPoint deltaPos)
{
    m_map->adjustOffset(deltaPos);
}

void GameProcessor::zoomMap(int direction, QPointF point)
{
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
                // Cannot select an empty block
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

        case 2:                             // Choose whether to attack, wait now
        {
            break;
        }

        case 3:                             // Select an enemy unit to attack
        {
            Block *newBlock = m_map->getBlock(position);
            if (!m_attackableBlocks.contains(newBlock))
            {
                // Cannot attack the block
                break;
            }

            m_selectedBlock = newBlock;
            emit enterStage(4);
            break;
        }

        case 4:                             // wait until movement/attacking finishes
        case 5:
        {
            break;
        }
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
                updateAttackableBlocks(m_cursorBlock, m_gameInfo->getUnitInfo()[unitId][2],
                                       m_gameInfo->getUnitInfo()[unitId][3]);
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
            updateAttackableBlocks(m_cursorBlock, m_gameInfo->getUnitInfo()[unitId][2],
                                   m_gameInfo->getUnitInfo()[unitId][3]);
            break;
        }

        case 2:                             // Choose whether to attack, wait now
        {
            break;
        }

        case 3:                             // Select an enemy unit to attack
        {
            Block *newBlock = m_map->getBlock(position);
            m_cursorBlock = newBlock;
            break;
        }

        case 4:                             // wait until movement finishes
        case 5:                             // wait until attacking finishes
        {
            break;
        }
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
            // Show description
            // TODO: REFINED
            m_contextMenu->clear();
            m_contextMenu->addAction(m_actions[0]);
            m_contextMenu->exec(QCursor::pos());
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

        case 4:
        case 5:
        {
            // Wait
            break;
        }
    }
}
