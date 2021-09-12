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

void AIProcessor::paint(QPainter *painter)
{
    m_unitMover->paint(painter);
}

void AIProcessor::getAccessibleBlocks(QVector<BlockValue> &accessibleBlocks, Block *block, int unitType, int movement)
{
    int **terrainInfo = m_gameInfo->getTerrainInfo();  // move cost: terrainInfo[terrainId][unitType]
    accessibleBlocks.clear();

    QVector<BlockValue> remainingBlocks;
    remainingBlocks.push_back({block, nullptr, nullptr, 0, 0});    // the vector only has the origin at first

    while (!remainingBlocks.isEmpty())
    {
        // sort by distance, get the one with the shortest distance
        std::sort(remainingBlocks.begin(), remainingBlocks.end(), [](const BlockValue & v1, const BlockValue & v2)
        {
            return v1.distance > v2.distance;
        });

        const BlockValue temp = remainingBlocks.last();
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

            // find out whether the block is contained
            for (auto iter = remainingBlocks.begin(); iter != remainingBlocks.end(); iter++)
            {
                if (iter->block == adjBlock)
                {
                    // yes
                    if (iter->distance > distance)
                    {
                        // update distance and route
                        iter->distance = distance;
                        iter->lastBlock = temp.block;
                    }
                    counted = true;
                    break;
                }
            }

            if (!counted)
            {
                // find out whether the block has been submitted
                for (auto iter = accessibleBlocks.begin(); iter != accessibleBlocks.end(); iter++)
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
                    // not considered
                    remainingBlocks.push_back({adjBlock, temp.block, nullptr, distance, 0});
                }
            }
        }

        accessibleBlocks.push_back(temp);
    }
}

void AIProcessor::updateAttackPoints(QVector<BlockValue> &accessibleBlocks, Unit *unit)
{
    int **terrainInfo = m_gameInfo->getTerrainInfo();
    float **unitInfo = m_gameInfo->getUnitInfo();
    float **damageMatrix = m_gameInfo->getDamageMatrix();

    int unitId = unit->getId();
    int rangeLow = unitInfo[unitId][2];
    int rangeHigh = unitInfo[unitId][3];
    int attack = unitInfo[unitId][5];

    for (auto &blockValue : accessibleBlocks)
    {
        Block *block = blockValue.block;
        QVector<Block *> blocksInRange;

        int maxAttackPoints = 0;
        Block *maxPassiveBlock = nullptr;

        for (int radius = rangeLow; radius <= rangeHigh; radius++)
        {
            // different distances are also given different points (*1 ~ *1.5)
            m_map->getAdjacentBlocks(blocksInRange, block, radius, radius);

            for (const auto &passiveBlock : qAsConst(blocksInRange))
            {
                Unit *passiveUnit = passiveBlock->getUnit();
                if (passiveUnit != nullptr && passiveUnit->getSide() != m_nSide)
                {
                    if ((passiveUnit->getId() == 19 && passiveUnit->getSide() < 0)
                            && (radius > 1 || unitInfo[unitId][7] == 0))
                    {
                        // Can't capture a building or it's far away
                        break;
                    }

                    // Able to attack the unit
                    int passiveId = passiveUnit->getId();

                    // Check damage matrix
                    double multiplier = damageMatrix[unitId][passiveId];

                    // Check terrain shields
                    if (passiveId < 10 || passiveId > 13)
                    {
                        // Enemy not an air unit
                        int shields = terrainInfo[passiveBlock->getTerrain()][5];
                        multiplier *= ((10.0 - shields) / 10.0);
                    }

                    int damagePoints = multiplier * attack;
                    int passiveHP = passiveUnit->getHP();
                    if (damagePoints > passiveHP)
                    {
                        // killed
                        damagePoints *= 2;
                    }

                    if (passiveId >= 18)
                    {
                        // commander or building
                        damagePoints *= 1.5;
                    }

                    // Multiplied by a ratio related to distance
                    damagePoints *= ((radius - 1) * 0.1 + 1);

                    // Check whether the damagePoints is the greatest
                    if (damagePoints > maxAttackPoints)
                    {
                        maxAttackPoints = damagePoints;
                        maxPassiveBlock = passiveBlock;
                    }
                }
            }
        }

        blockValue.value += maxAttackPoints;
        blockValue.confrontingBlock = maxPassiveBlock;
    }
}

void AIProcessor::updateDefencePoints(QVector<BlockValue> &accessibleBlocks, Unit *unit)
{
    int **terrainInfo = m_gameInfo->getTerrainInfo();
    float **unitInfo = m_gameInfo->getUnitInfo();
    float **damageMatrix = m_gameInfo->getDamageMatrix();

    int unitId = unit->getId();

    for (auto &blockValue : accessibleBlocks)
    {
        Block *block = blockValue.block;

        int defensivePoints = 0;

        QVector<Block *> blocksInRange;
        m_map->getAdjacentBlocks(blocksInRange, block, 4);          // The distance is currently set to 4

        for (const auto &objBlock : qAsConst(blocksInRange))
        {
            Unit *objUnit = objBlock->getUnit();
            if (objUnit != nullptr && objUnit->getSide() != m_nSide)
            {
                // May get attacked
                int passiveId = objUnit->getId();
                int passiveAttack = unitInfo[passiveId][5];

                // Check damage matrix
                double multiplier = damageMatrix[passiveId][unitId];

                int damagePoints = multiplier * passiveAttack;
                int hp = unit->getHP();
                if (damagePoints > hp)
                {
                    // killed
                    damagePoints *= 2;
                }

                defensivePoints += damagePoints;
            }
        }

        // Check terrain
        defensivePoints *= ((10.0 - terrainInfo[block->getTerrain()][5]) / 10.0);

        // Defence factor
        defensivePoints *= 0.5;

        // Add to blockValue
        blockValue.value -= defensivePoints;
    }
}

void AIProcessor::updateNearbyBlocks(QVector<BlockValue> &accessibleBlocks, Block *block, int unitType, int movement)
{
    int **terrainInfo = m_gameInfo->getTerrainInfo();  // move cost: terrainInfo[terrainId][unitType]

    QVector<BlockValue> allBlocks;
    QVector<BlockValue> remainingBlocks;
    remainingBlocks.push_back({block, nullptr, nullptr, 0, 0});    // the vector only has the origin at first

    while (!remainingBlocks.isEmpty())
    {
        // sort by distance, get the one with the shortest distance
        std::sort(remainingBlocks.begin(), remainingBlocks.end(), [](const BlockValue & v1, const BlockValue & v2)
        {
            return v1.distance > v2.distance;
        });

        const BlockValue temp = remainingBlocks.last();
        remainingBlocks.pop_back();

        QVector<Block *> adjacentBlocks;
        m_map->getAdjacentBlocks(adjacentBlocks, temp.block);
        bool findBuilding = false;

        for (const auto &adjBlock : qAsConst(adjacentBlocks))
        {
            if (adjBlock->getUnit() != nullptr)
            {
                // Check if there's an enemy unit nearby
                if (adjBlock->getUnit()->getSide() != m_nSide)
                {
                    findBuilding = true;
                    break;
                }
                continue;
            }

            int distance = temp.distance + terrainInfo[adjBlock->getTerrain()][unitType];
            bool counted = false;

            // find out whether the block is contained
            for (auto iter = remainingBlocks.begin(); iter != remainingBlocks.end(); iter++)
            {
                if (iter->block == adjBlock)
                {
                    // yes
                    if (iter->distance > distance)
                    {
                        // update distance and route
                        iter->distance = distance;
                        iter->lastBlock = temp.block;
                    }
                    counted = true;
                    break;
                }
            }

            if (!counted)
            {
                // find out whether the block has been submitted
                for (auto iter = allBlocks.begin(); iter != allBlocks.end(); iter++)
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
                    // not considered
                    remainingBlocks.push_back({adjBlock, temp.block, nullptr, distance, 0});
                }
            }
        }

        allBlocks.push_back(temp);

        if (findBuilding)
        {
            break;
        }
    }

    qDebug() << allBlocks.size();

    // Filter the path to the enemy unit
    accessibleBlocks.clear();
    Block *temp = allBlocks.last().block;
    while (temp != nullptr)
    {
        // Find the blockValue related to temp
        BlockValue *blockValue = nullptr;
        for (auto &b : allBlocks)
        {
            if (b.block == temp)
            {
                blockValue = &b;
                break;
            }
        }

        if (blockValue == nullptr)
        {
            qDebug() << "error: nullptr in setting the route";
            break;
        }

        if (blockValue->distance <= movement)
        {
            accessibleBlocks.push_back(*blockValue);
        }

        temp = blockValue->lastBlock;
    }

    accessibleBlocks.first().value = 100;
}

void AIProcessor::moveSingleUnit(Block *block)
{
    // Selecting an ordinary unit
    qDebug() << "AI moving block" << block->getRow() << block->getColumn();

    // Find the best choice
    float **unitInfo = m_gameInfo->getUnitInfo();
    int unitId = block->getUnit()->getId();

    QVector<BlockValue> accessibleBlocks;
    getAccessibleBlocks(accessibleBlocks, block, unitInfo[unitId][0], unitInfo[unitId][1]);

    // update attacking points
    updateAttackPoints(accessibleBlocks, block->getUnit());

    // check if there is no update on attack points (not able to attack)
    bool attackable = false;
    for (const auto &block : qAsConst(accessibleBlocks))
    {
        if (block.value > 0)
        {
            attackable = true;
            break;
        }
    }

    if (attackable)
    {
        qDebug() << "attack";
        // Able to attack: update defensive points
        updateDefencePoints(accessibleBlocks, block->getUnit());
    }
    else
    {
        qDebug() << "not able to attack";
        // Not able to attack: find a path to the nearby enemy
        updateNearbyBlocks(accessibleBlocks, block, unitInfo[unitId][0], unitInfo[unitId][1]);
    }

    // Set route
    BlockValue *idealBlock = std::max_element(accessibleBlocks.begin(), accessibleBlocks.end(),
                             [](const BlockValue & v1, const BlockValue & v2)
    {
        return v1.value < v2.value;
    });

    m_movingRoute.clear();

    Block *temp = idealBlock->block;
    while (temp != nullptr)
    {
        m_movingRoute.push_front(temp);

        // Find the blockValue related to temp
        const BlockValue *blockValue = nullptr;
        for (const auto &b : qAsConst(accessibleBlocks))
        {
            if (b.block == temp)
            {
                blockValue = &b;
                break;
            }
        }

        if (blockValue == nullptr)
        {
            qDebug() << "error: nullptr in setting the route";
            break;
        }
        temp = blockValue->lastBlock;
    }

    // Set confronting block
    m_confrontingBlock = idealBlock->confrontingBlock;

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
    m_confrontingBlock = nullptr;

    QVector<Block *> adjacentBlocks;
    int coins = m_stats->getCoins(m_nSide);
    int unitId = -8;            // do not generate units if not reset
    int innerType = block->getUnit()->getInnerType();

    if (innerType == 1)
    {
        // Choose from soldier (100C) & archer (500C)
        m_map->getAdjacentBlocks(adjacentBlocks, block, 4);

        // Count enemy/uncaptured units
        int count = 0;
        for (const auto &b : qAsConst(adjacentBlocks))
        {
            if (b->getUnit() != nullptr && b->getUnit()->getSide() != m_nSide)
            {
                count++;
            }
        }

        if (count > 0)
        {
            // Enemy nearby: choose soldier
            if (coins >= m_gameInfo->getUnitInfo()[0][9])
            {
                unitId = 0;
            }
        }
        else
        {
            // No enemy nearby: check own units
            m_map->getAdjacentBlocks(adjacentBlocks, block, 6);

            // Count own units
            int count = 0;
            for (const auto &b : qAsConst(adjacentBlocks))
            {
                if (b->getUnit() != nullptr && b->getUnit()->getSide() == m_nSide
                        && b->getUnit()->getId() <= 18)
                {
                    count++;
                }
            }

            if (count > 5)
            {
                // Too many own units: choose trebuchet
                if (coins >= m_gameInfo->getUnitInfo()[9][9])
                {
                    unitId = 9;
                }
            }
            else if (count > 3)
            {
                // Not so many own units: choose archer
                if (coins >= m_gameInfo->getUnitInfo()[4][9])
                {
                    unitId = 4;
                }
            }
            else
            {
                // Not enough own units: choose soldier
                if (coins >= m_gameInfo->getUnitInfo()[0][9])
                {
                    unitId = 0;
                }
            }
        }
    }
    else if (innerType == 2)
    {
        // Choose from sky rider (800C) & dragon (1250C)
        // Count own flying units
        m_map->getAdjacentBlocks(adjacentBlocks, block, 6);
        int count = 0;
        for (const auto &b : qAsConst(adjacentBlocks))
        {
            if (b->getUnit() != nullptr && b->getUnit()->getSide() == m_nSide
                    && b->getUnit()->getId() >= 10 && b->getUnit()->getId() <= 13)
            {
                count++;
            }
        }

        if (count > 2)
        {
            // Too many own units: choose dragon
            if (coins >= m_gameInfo->getUnitInfo()[13][9])
            {
                unitId = 13;
            }
        }
        else
        {
            // Not enough own units: choose sky rider
            if (coins >= m_gameInfo->getUnitInfo()[12][9])
            {
                unitId = 12;
            }
        }
    }
    else if (innerType == 3)
    {
        // Choose from turtle (400C) & warship (900C)
        // Count own water units
        m_map->getAdjacentBlocks(adjacentBlocks, block, 6);
        int count = 0;
        for (const auto &b : qAsConst(adjacentBlocks))
        {
            if (b->getUnit() != nullptr && b->getUnit()->getSide() == m_nSide
                    && b->getUnit()->getId() >= 14 && b->getUnit()->getId() <= 17)
            {
                count++;
            }
        }

        if (count > 2)
        {
            // Too many own units: choose warship
            if (coins >= m_gameInfo->getUnitInfo()[17][9])
            {
                unitId = 17;
            }
        }
        else
        {
            // Not enough own units: choose turtle
            if (coins >= m_gameInfo->getUnitInfo()[15][9])
            {
                unitId = 15;
            }
        }
    }

    if (unitId >= 0)
    {
        // A unit selected
        // Check if there is an empty block for unit generation
        QVector<Block *> nearbyBlocks;
        QVector<Block *> placeableBlocks;
        m_map->getAdjacentBlocks(nearbyBlocks, block);
        int **terrainInfo = m_gameInfo->getTerrainInfo();
        int unitType = m_gameInfo->getUnitInfo()[unitId][0];
        for (const auto &b : qAsConst(nearbyBlocks))
        {
            if (b->getUnit() == nullptr &&
                    terrainInfo[b->getTerrain()][unitType] < 10)
            {
                placeableBlocks.push_back(b);
            }
        }

        if (!placeableBlocks.isEmpty())
        {
            // Can generate a unit
            createUnit(unitId, m_nSide);
            std::random_shuffle(placeableBlocks.begin(), placeableBlocks.end());
            m_confrontingBlock = placeableBlocks[0];
        }
    }

// End operation, begin simulation
// m_movingRoute:      one element - the building
// m_confrontingBlock: where the unit generates
    confrontUnit();                              //  no need to move, go directly to confrontUnit()
}

void AIProcessor::createUnit(int unitId, int side)
{
    if (m_tempUnit != nullptr)
    {
        delete m_tempUnit;
    }
    int maxHP = m_gameInfo->getUnitInfo()[unitId][6];
    m_tempUnit = new Unit(unitId, side, maxHP, m_map);
}

void AIProcessor::activate()
{
    qDebug() << "AI for side" << m_nSide + 1 << "activated";

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

void AIProcessor::nextMove()
{
    if (m_remainingBlocks.size() <= 0)
    {
        finishRound();
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
            m_tempUnit = nullptr;
        }
    }

    // Battle ends (if there is a battle)
    m_confrontingBlock = nullptr;

    if (!battle)
    {
        emit operationFinished();
    }
}

void AIProcessor::finishRound()
{
    // Regenerate buildings, add coins
    for (const auto &unit : m_stats->getUnits(m_nSide))
    {
        // Regenerate buildings
        if (unit->getId() > 18)
        {
            unit->regenerate(0.1);
            if (unit->getInnerType() >= 4 || unit->getInnerType() == 0)
            {
                // Village/Base: coins + 100
                m_stats->addCoins(100, m_nSide);
            }
        }
    }

    qDebug() << "Units remaining" << m_stats->getUnits(m_nSide).size();
    qDebug() << "Coins remaining" << m_stats->getCoins(m_nSide);
    emit finished();
}
