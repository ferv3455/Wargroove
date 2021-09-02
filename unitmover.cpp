#include "unitmover.h"

#include <QTimer>
#include <QDebug>

UnitMover::UnitMover(Settings *settings, Map *map, QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_map(map),
      m_nCurrentMove(0),
      m_movingBlock{nullptr, nullptr},
      m_movingUnit(nullptr),
      m_bMoving(false),
      m_movingRoute()
{
    m_nTotalMoves = m_settings->m_nMoveTime / m_settings->m_nRefreshTime;

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->start(m_settings->m_nRefreshTime);
    connect(m_refreshTimer, &QTimer::timeout, this, &UnitMover::updateSingleMovement);
    connect(this, &UnitMover::stepFinished, this, &UnitMover::updateRouteMovement);
}

void UnitMover::paint(QPainter *painter) const
{
    if (m_movingUnit != nullptr)
    {
        QPoint pos[2] =
        {
            m_map->getCenterPosition(m_movingBlock[0]),
            m_map->getCenterPosition(m_movingBlock[1])
        };

        QPoint center = (pos[0] * (m_nTotalMoves - m_nCurrentMove) + pos[1] * m_nCurrentMove) / m_nTotalMoves;
        int size = m_map->getBlockSize();
        m_movingUnit->paint(painter,
                            QRect(center - QPoint(size, 1.5 * size), center + QPoint(size, 0.5 * size)),
                            m_map->getDynamicsId());
    }
}

void UnitMover::moveUnit(Block *fromBlock, Block *toBlock)
{
    if (m_movingUnit != nullptr)
    {
        return;
    }

    m_movingUnit = fromBlock->getUnit();
    if (m_movingUnit == nullptr)
    {
        return;
    }

    fromBlock->setUnit(nullptr);
    m_movingBlock[0] = fromBlock;
    m_movingBlock[1] = toBlock;
}

void UnitMover::moveUnit(QVector<Block *> blocks)
{
    if (m_bMoving)
    {
        return;     // invalid
    }

    if (blocks.size() < 2)
    {
        return;     // invalid
    }

    if (blocks[0]->getUnit() == nullptr)
    {
        return;     // nothing to move
    }

    m_movingRoute = blocks;
    moveUnit(m_movingRoute[0], m_movingRoute[1]);
    m_movingRoute.pop_front();
    m_bMoving = true;
}

void UnitMover::updateSingleMovement()
{
    if (m_movingUnit == nullptr)
    {
        return;
    }

    if (m_nCurrentMove < m_nTotalMoves)
    {
        m_nCurrentMove++;
    }
    else if (m_nCurrentMove == m_nTotalMoves)
    {
        // moving finished
        m_movingBlock[1]->setUnit(m_movingUnit);
        m_movingUnit = nullptr;
        m_movingBlock[0] = nullptr;
        m_movingBlock[1] = nullptr;
        m_nCurrentMove = 0;
        emit stepFinished();
    }
}

void UnitMover::updateRouteMovement()
{
    if (m_movingRoute.size() < 2)
    {
        // moving finished
        qDebug() << "Moving finished: to" << m_movingRoute[0]->getRow() << m_movingRoute[0]->getColumn();
        m_bMoving = false;
        emit movementFinished(m_movingRoute[0]->getUnit());
        m_movingRoute.pop_back();
    }
    else
    {
        // next step
        moveUnit(m_movingRoute[0], m_movingRoute[1]);
        m_movingRoute.pop_front();
    }
}
