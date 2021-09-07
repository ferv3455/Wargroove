#include "gamestats.h"

GameStats::GameStats(QObject *parent, int coins) :
    QObject(parent),
    m_nCoins(coins),
    m_playerUnits(),
    m_enemyUnits()
{

}

int GameStats::getCoins() const
{
    return m_nCoins;
}

const QVector<Unit *> &GameStats::getPlayerUnits() const
{
    return m_playerUnits;
}

const QVector<Unit *> &GameStats::getEnemyUnits() const
{
    return m_enemyUnits;
}
