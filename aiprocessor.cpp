#include "aiprocessor.h"

#include <QDebug>

AIProcessor::AIProcessor(Settings *settings,
                         GameInfo *gameInfo,
                         Map *map,
                         GameStats *stats,
                         QMediaPlayer *SEplayer,
                         QObject *parent,
                         int side)
    : QObject(parent),
      m_settings(settings),
      m_gameInfo(gameInfo),
      m_map(map),
      m_stats(stats),

      m_SEplayer(SEplayer),
      m_battleWidget(nullptr),

      m_remainingBlocks(),
      m_movingRoute(),

      m_nSide(side),
      m_nStage(0)
{
    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    connect(m_unitMover, &UnitMover::movementFinished, this, &AIProcessor::nextMove);
    connect(this, &AIProcessor::operationFinished, this, &AIProcessor::nextMove);
}

void AIProcessor::activate()
{
    qDebug() << "AI for side" << m_nSide << "activated";

    float **unitInfo = m_gameInfo->getUnitInfo();
    m_remainingBlocks.clear();
    m_map->getAllBlocks(m_remainingBlocks, m_nSide);
    std::sort(m_remainingBlocks.begin(), m_remainingBlocks.end(), [ = ](const Block * b1, const Block * b2)
    {
        // sort by attacking range in ascending order (process last block first)
        return unitInfo[b1->getUnit()->getId()][3] < unitInfo[b2->getUnit()->getId()][3];
    });

    nextMove();
}

void AIProcessor::paint(QPainter *painter)
{
    m_unitMover->paint(painter);
}

void AIProcessor::nextMove()
{
    qDebug() << m_remainingBlocks.size();

    if (m_remainingBlocks.size() <= 0)
    {
        emit finished();
        return;
    }

    Block *block = m_remainingBlocks.last();
    m_remainingBlocks.pop_back();

    int unitId = block->getUnit()->getId();
    if (unitId <= 18)
    {
        qDebug() << "AI moving block" << block->getRow() << block->getColumn();

        // Set moving route (use a new funtion)
        m_movingRoute.clear();
        m_movingRoute.push_back(block);
        m_movingRoute.push_back(m_map->getBlock(block->getRow(), block->getColumn() - 1));
        m_unitMover->moveUnit(m_movingRoute);
    }
    else
    {
        qDebug() << "AI operating block" << block->getRow() << block->getColumn();
        emit operationFinished();
    }
}
