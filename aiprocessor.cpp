#include "aiprocessor.h"
#include "building.h"

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
      m_confrontingBlock(nullptr),
      m_tempUnit(nullptr),

      m_nSide(side),
      m_nStage(0)
{
    // Initialize unit mover
    m_unitMover = new UnitMover(m_settings, m_map, this);

    connect(m_unitMover, &UnitMover::movementFinished, this, &AIProcessor::confrontUnit);
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

void AIProcessor::moveSingleUnit(Block *block)
{
    // Selecting an ordinary unit
    qDebug() << "AI moving block" << block->getRow() << block->getColumn();

    // Set moving route (with at least one element)
    m_movingRoute.clear();
    m_movingRoute.push_back(block);





    // Set unit confrontation (confrontingBlock = nullptr - wait/do not battle)
    m_confrontingBlock = nullptr;
//    m_confrontingBlock = m_map->getBlock(block->getRow(), block->getColumn() - 2);

    // End operation, begin simulation
    m_unitMover->moveUnit(m_movingRoute);       // end with a signal connected to confrontUnit()
}

void AIProcessor::operateBuilding(Block *block)
{
    // Selecting a building
    qDebug() << "AI operating block" << block->getRow() << block->getColumn();

    // Set moving route (only the building)
    m_movingRoute.clear();
    m_movingRoute.push_back(block);

    // Set unit confrontation & new unit (if needed) (confrontingBlock = nullptr - do not generate)
    int coins = m_stats->getCoins(m_nSide);
    m_confrontingBlock = nullptr;

    // End operation, begin simulation
    // m_movingRoute:      one element - the building
    // m_confrontingBlock: where the unit generates
    confrontUnit();                              //  no need to move, go directly to confrontUnit()
}

void AIProcessor::nextMove()
{
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
        moveSingleUnit(block);
    }
    else
    {
        if (block->getUnit()->getInnerType() >= 1 || block->getUnit()->getInnerType() <= 3)
        {
            operateBuilding(block);
        }
    }
}

void AIProcessor::confrontUnit()
{
    if (m_confrontingBlock == nullptr || m_movingRoute.last() == nullptr)
    {
        emit operationFinished();
        return;
    }

    // Battle
    bool battle = false;
    Unit *activeUnit = m_movingRoute.last()->getUnit();
    Unit *passiveUnit = m_confrontingBlock->getUnit();

    if (passiveUnit != nullptr)
    {
        if (passiveUnit->isCarrier() && passiveUnit->getSide() == m_nSide)
        {
            // Get in the carrier
            passiveUnit->setCarrier(activeUnit);
            m_movingRoute.last()->setUnit(nullptr);
        }
        else
        {
            // Two real units battle
            m_battleWidget = new BattleWidget(m_movingRoute.last(), m_confrontingBlock,
                                              m_gameInfo, static_cast<QWidget *>(parent()));
            battle = true;
            connect(m_battleWidget, &BattleWidget::end, this, [ = ]()
            {
                // Battle animation ends: delete battle widget
                delete m_battleWidget;
                emit operationFinished();
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
                int shields = terrainInfo[m_confrontingBlock->getTerrain()][5];
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
                    m_confrontingBlock->setUnit(nullptr);
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
                        building->setSide(m_nSide);
                        m_stats->addUnit(building);
                    }
                }
            }

            if (!killed)
            {
                // Find out if they are adjacent blocks
                QVector<Block *> v;
                m_map->getAdjacentBlocks(v, m_movingRoute.last());
                if (v.contains(m_confrontingBlock))
                {
                    // Adjacent: counter-attack
                    double passiveRandom = (std::rand() % 10 + 95) / 100.0;
                    int passiveDamage = damageMatrix[passiveId][activeId] * passiveAttack * passiveRandom / 2;
                    killed = activeUnit->injured(passiveDamage);
                    m_battleWidget->setPassiveAttack(passiveDamage, killed);
                    if (killed)
                    {
                        m_stats->removeUnit(activeUnit);
                        delete activeUnit;
                        m_movingRoute.last()->setUnit(nullptr);
                        // activeUnit = nullptr;
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
            m_confrontingBlock->setUnit(activeUnit->getCarrier());
            activeUnit->setCarrier(nullptr);
        }
        else
        {
            // A building generating a unit
            m_confrontingBlock->setUnit(m_tempUnit);
            m_stats->addUnit(m_tempUnit);
            m_stats->addCoins(-m_gameInfo->getUnitInfo()[m_tempUnit->getId()][9], m_nSide);
            // m_tempUnit = nullptr;
        }
    }

    // Battle ends (if there is a battle)
    m_confrontingBlock = nullptr;

    if (!battle)
    {
        emit operationFinished();
    }
}
