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
      m_movingRoute()
{
    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    // Connect signals and slots
    connect(m_unitMover, &UnitMover::movementFinished, this, [ = ]() {m_nStage = 0;});
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

    if (m_movingRoute.size() > 1)
    {
        // Paint the moving route
        QPainterPath path;
        path.moveTo(m_movingRoute[0]->getCenter());
        for (auto iter = m_movingRoute.begin() + 1; iter != m_movingRoute.end(); iter++)
        {
            path.lineTo((*iter)->getCenter());
        }

        QPen pen(QColor(255, 152, 0, 150), m_map->getBlockSize());
        pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
        painter->setPen(pen);
        painter->drawPath(path);
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
    Block *newBlock = m_map->getBlock(position);
    if (newBlock == nullptr)            // Cannot select an empty block
    {
        return;
    }

    switch (m_nStage)
    {
        case 0:                             // Select unit
        {
            if (newBlock->getUnit() == nullptr)
            {
                return;
            }
            m_selectedBlock = newBlock;
            m_movingRoute.push_back(m_selectedBlock);
            m_nStage++;
            break;
        }

        case 1:                             // Select moving route
        {
            if (m_unitMover->isBusy())
            {
                return;
            }

            if (m_movingRoute.last() != newBlock)
            {
                m_movingRoute.push_back(newBlock);
            }
            m_unitMover->moveUnit(m_movingRoute);
            m_movingRoute.clear();
            m_selectedBlock = nullptr;
            m_nStage++;
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
    Block *newBlock = m_map->getBlock(position);
    if (newBlock == m_cursorBlock || newBlock == nullptr)  // Cannot move to no/the same block
    {
        return;
    }
    m_cursorBlock = newBlock;

    switch (m_nStage)
    {
        case 1:                             // Select moving route
        {
            if (m_cursorBlock->getUnit() != nullptr &&
                    m_cursorBlock->getUnit() != m_selectedBlock->getUnit())
            {
                return;
            }

            int idx = m_movingRoute.indexOf(m_cursorBlock);
            if (idx >= 0)
            {
                // block reselected: undo selection
                while (m_movingRoute.size() > idx + 1)
                {
                    m_movingRoute.pop_back();
                }
                return;
            }

            QVector<Block *> v;
            m_map->getAdjacentBlocks(v, m_cursorBlock);
            if (!v.contains(m_movingRoute.last()))
            {
                return;
            }

            m_movingRoute.push_back(m_cursorBlock);
            break;
        }

        case 2:                             // Wait until movement finishes
        {
            break;
        }
    }
}
