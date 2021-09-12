#include "unitmover.h"

#include <QTimer>
#include <QDebug>

UnitMover::UnitMover(Settings *settings, Map *map, QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_map(map),
      m_nCurrentMove(0),
      m_bVisible(true),
      m_movingBlock{nullptr, nullptr},
      m_bMoving(false),
      m_movingUnit(nullptr),
      m_movingRoute(),
      m_currentStep()
{
    m_nTotalMoves = m_settings->m_nMoveSteps;
    m_refreshTimer = new QTimer(this);

    connect(m_refreshTimer, &QTimer::timeout, this, &UnitMover::updateSingleMovement);
    connect(this, &UnitMover::stepFinished, this, &UnitMover::updateRouteMovement);
}

void UnitMover::paint(QPainter *painter) const
{
    painter->setPen(Qt::white);
    if (m_bMoving && m_bVisible)
    {
        QPoint pos[2] = {m_movingBlock[0]->getCenter(), m_movingBlock[1]->getCenter()};
        QPoint center = (pos[0] * (m_nTotalMoves - m_nCurrentMove) +
                         pos[1] * m_nCurrentMove) / m_nTotalMoves;
        int size = m_map->getBlockSize();
        m_movingUnit->paint(painter,
                            QRect(center - QPoint(size, 1.5 * size), center + QPoint(size, 0.5 * size)),
                            m_map->getDynamicsId());
    }
}

void UnitMover::moveUnit(Block *fromBlock, Block *toBlock)
{
    if (m_movingUnit == nullptr)
    {
        return;
    }

    m_movingBlock[0] = fromBlock;
    m_movingBlock[1] = toBlock;
    m_bVisible = fromBlock->isVisible() || toBlock->isVisible();
    m_movingUnit->setDirection(toBlock->getCenter().x() < fromBlock->getCenter().x());
    m_nCurrentMove = 0;
}

void UnitMover::moveUnit(QVector<Block *> &blocks)
{
    if (m_bMoving)
    {
        return;     // invalid
    }

    if (blocks.size() < 2)
    {
        qDebug() << "Movement finished";
        emit movementFinished(blocks.first()->getUnit());
        return;     // no movement required
    }

    if (blocks.first()->getUnit() == nullptr)
    {
        return;     // nothing to move
    }

    if (blocks.last()->getUnit() != nullptr)
    {
        return;     // cannot move
    }

    m_movingRoute = blocks;
    m_movingUnit = m_movingRoute.first()->getUnit();
    m_movingRoute.first()->setUnit(nullptr);
    m_currentStep = m_movingRoute.begin();
    moveUnit(*m_currentStep, *(m_currentStep + 1));
    m_bMoving = true;
    m_refreshTimer->start(m_settings->m_nRefreshTime);
}

bool UnitMover::isBusy() const
{
    return m_bMoving;
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
        emit stepFinished();
    }
}

void UnitMover::updateRouteMovement()
{
    m_currentStep++;

    if (m_currentStep + 1 == m_movingRoute.end())
    {
        // moving finished
        qDebug() << "Movement finished";
        m_movingRoute.last()->setUnit(m_movingUnit);
        m_movingRoute.clear();
        m_bMoving = false;
        m_refreshTimer->stop();
        emit movementFinished(m_movingUnit);
    }
    else
    {
        // next step
        moveUnit(*m_currentStep, *(m_currentStep + 1));
    }
}
