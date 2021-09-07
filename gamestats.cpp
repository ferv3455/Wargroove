#include "gamestats.h"

GameStats::GameStats(QObject *parent, int totalSides, int coins) :
    QObject(parent), m_nTotalSides(totalSides)
{
    m_nCoins = new int[m_nTotalSides]();
    m_units = new QVector<Unit *>[m_nTotalSides]();

    for (int i = 0; i < m_nTotalSides; i++)
    {
        m_nCoins[i] = coins;
    }
}

int GameStats::getCoins(int side) const
{
    return m_nCoins[side];
}

void GameStats::addCoins(int deltaCoins, int side)
{
    m_nCoins[side] += deltaCoins;
}

const QVector<Unit *> &GameStats::getUnits(int side) const
{
    return m_units[side];
}

void GameStats::addUnit(Unit *unit)
{
    if (unit != nullptr)
    {
        m_units[unit->getSide()].push_back(unit);
    }
}

void GameStats::removeUnit(Unit *unit)
{
    if (unit != nullptr)
    {
        m_units[unit->getSide()].removeOne(unit);
    }
}

