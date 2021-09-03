#include "gameprocessor.h"

#include <QPainterPath>
#include <QDebug>

GameProcessor::GameProcessor(Settings *settings,
                             GameInfo *gameInfo,
                             Map *map,
                             TipsLabel *tipsLabel,
                             QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_gameInfo(gameInfo),
      m_map(map),
      m_tipsLabel(tipsLabel),

      m_nStage(0),
      m_selectedBlock(nullptr),
      m_cursorBlock(nullptr),

      m_nMovesLeft(0),
      m_movingRoute(),
      m_accessibleBlocks()
{
    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    // Connect signals and slots
    connect(m_unitMover, &UnitMover::movementFinished, this, [ = ]() {m_nStage = 0;});

    m_accessibleBlocks.push_back(m_map->getBlock(0, 0));
}

void GameProcessor::paint(QPainter *painter)
{
    m_unitMover->paint(painter);

    painter->setBrush(QBrush());
    int blockSize = m_map->getBlockSize();
    if (m_cursorBlock != nullptr)
    {
        // Paint the block with the cursor on
        painter->setPen(Qt::blue);
        painter->drawEllipse(m_cursorBlock->getCenter(), blockSize / 2, blockSize / 2);
    }
    if (m_selectedBlock != nullptr)
    {
        // Paint the selected block
        painter->setPen(Qt::red);
        painter->drawEllipse(m_selectedBlock->getCenter(), blockSize / 2, blockSize / 2);
    }

    if (m_nStage == 1)
    {
        // Paint accessible blocks
        painter->setPen(Qt::yellow);
        for (const auto &block : qAsConst(m_accessibleBlocks))
        {
            painter->drawEllipse(block->getCenter(), blockSize / 2, blockSize / 2);
        }

        // Paint the moving route
        QPainterPath path;
        path.moveTo(m_movingRoute.first()->getCenter());
        for (auto iter = m_movingRoute.begin() + 1; iter != m_movingRoute.end(); iter++)
        {
            path.lineTo((*iter)->getCenter());
        }

        QPen pen(QColor(255, 152, 0, 150), m_map->getBlockSize());
        pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
        painter->setPen(pen);
        painter->drawPath(path);

        // Paint hints
        pen.setWidth(3);
        painter->setPen(pen);
        painter->setBrush(QBrush(QColor(95, 95, 95, 255)));
        painter->drawRect(50, 50, 100, 150);
        m_selectedBlock->getUnit()->paint(painter, QRect(50, 50, 100, 100), 0);
        painter->drawImage(QRect(60, 160, 30, 30), QImage(":/image/icon/movement"));
        painter->drawText(120, 180, QString::number(m_nMovesLeft));
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

    QVector<Vertex> tempBlocks;
    tempBlocks.push_back({block, 0});    // the vector only has the origin at first

    while (true)
    {
        // sort by distance, get the one with the shortest distance
        std::sort(tempBlocks.begin(), tempBlocks.end(), [](const Vertex & v1, const Vertex & v2)
        {
            return v1.distance > v2.distance;
        });

        const Vertex temp = tempBlocks.last();
        tempBlocks.pop_back();
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
            for (auto iter = tempBlocks.begin(); iter != tempBlocks.end(); iter++)
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
                // not counted
                tempBlocks.push_back({adjBlock, distance});
            }
        }

        m_accessibleBlocks.push_back(temp.block);
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
            if (newBlock == nullptr)            // Cannot select an empty block
            {
                return;
            }

            if (newBlock->getUnit() == nullptr)
            {
                return;
            }
            m_selectedBlock = newBlock;
            int unitId = m_selectedBlock->getUnit()->getId();
            int unitType = m_gameInfo->getUnitInfo()[unitId][0];
            m_nMovesLeft = m_gameInfo->getUnitInfo()[unitId][1];
            updateAccessibleBlocks(m_selectedBlock, unitType, m_nMovesLeft);
            m_movingRoute.push_back(m_selectedBlock);
            m_nStage++;
            break;
        }

        case 1:                             // Select moving route
        {
            Block *newBlock = m_map->getBlock(position);
            if (newBlock == nullptr)            // Cannot select an empty block
            {
                return;
            }

            if (m_unitMover->isBusy())
            {
                return;
            }
            if (m_movingRoute.last() != newBlock)
            {
                return;         // cannot go there
            }

            m_selectedBlock = m_movingRoute.last();

            // Begin moving
            m_nStage++;
            m_unitMover->moveUnit(m_movingRoute);
            m_movingRoute.clear();
            break;
        }

        case 2:                             // Wait until movement finishes
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
            if (newBlock == m_cursorBlock || newBlock == nullptr)  // Cannot move to no/the same block
            {
                return;
            }
            m_cursorBlock = newBlock;
            break;
        }

        case 1:                             // Select moving route
        {
            Block *newBlock = m_map->getBlock(position);
            if (newBlock == m_cursorBlock || newBlock == nullptr)  // Cannot move to no/the same block
            {
                return;
            }
            m_cursorBlock = newBlock;

            if (m_cursorBlock->getUnit() != nullptr &&
                    m_cursorBlock->getUnit() != m_selectedBlock->getUnit())  // destination isn't empty
            {
                return;
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
                return;
            }

            QVector<Block *> v;
            m_map->getAdjacentBlocks(v, m_cursorBlock);
            if (!v.contains(m_movingRoute.last()))
            {
                // not adjacent
                return;
            }

            int terrainId = m_cursorBlock->getTerrain();
            int unitType = m_gameInfo->getUnitInfo()[m_selectedBlock->getUnit()->getId()][0];
            int moveCost = m_gameInfo->getTerrainInfo()[terrainId][unitType];
            if (moveCost > m_nMovesLeft)
            {
                return;
            }

            m_nMovesLeft -= moveCost;
            m_movingRoute.push_back(m_cursorBlock);
            break;
        }

        case 2:                             // Wait until movement finishes
        {
            break;
        }
    }
}

void GameProcessor::contextMenu(QPoint position)
{
    Q_UNUSED(position);

    switch (m_nStage)
    {
        case 1:
        {
            m_nStage = 0;
            m_movingRoute.clear();
            m_selectedBlock = nullptr;
            break;
        }
    }
}
