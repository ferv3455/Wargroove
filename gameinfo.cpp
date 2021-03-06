#include "gameinfo.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

GameInfo::GameInfo(QObject *parent)
    : QObject(parent),
      m_nTerrainNumber(9),
      m_nUnitType(5),
      m_nUnitNumber(21)
{
    // Initialize names
    m_sUnitNames = QStringList
    {
        tr("Soldier"),      // 0
        tr("Dog"),          // 1
        tr("Spearman"),     // 2
        tr("Mage"),         // 3
        tr("Archer"),       // 4
        tr("Giant"),        // 5
        tr("Cavalry"),      // 6
        tr("Wagon"),        // 7
        tr("Ballista"),     // 8
        tr("Trebuchet"),    // 9
        tr("Balloon"),      // 10
        tr("Aeronaut"),     // 11
        tr("Sky Rider"),    // 12
        tr("Dragon"),       // 13
        tr("Barge"),        // 14
        tr("Turtle"),       // 15
        tr("Harpoon Ship"), // 16
        tr("Warship")       // 17
    };
    m_sCommanderNames = QStringList
    {
        tr("Mercia"),       // 18_1
        tr("Emeric"),       // 18_2
        tr("Caesar"),       // 18_3
        tr("Sigrid"),       // 18_1
        tr("Valder"),       // 18_2
        tr("Ragna")         // 18_3
    };
    m_sBuildingNames = QStringList
    {
        tr("Stronghold"),   // 19_0
        tr("Barrack"),      // 19_1
        tr("Tower"),        // 19_2
        tr("Port"),         // 19_3
        tr("Village"),      // 19_4
        tr("Water Village") // 19_5
    };
    m_sTerrainNames = QStringList
    {
        tr("Air"),          // 0
        tr("Road"),         // 1
        tr("Plain"),        // 2
        tr("Forest"),       // 3
        tr("Mountain"),     // 4
        tr("Beach"),        // 5
        tr("Shallow Sea"),  // 6
        tr("Deep Sea"),     // 7
        tr("Bridge"),       // 8
        tr("Reef")          // 9
    };

    // Initialize descriptions
    m_sUnitDescription = QStringList
    {
        tr("Basic infantry, useful for capturing structures."),
        tr("A quick unit."),
        tr("Slower, more powerful infantry."),
        tr("A powerful unit built to combat air threats."),
        tr("Ranged ground units able to move and attack in the same turn."),
        tr("An immensely powerful unit, especially during critical hits."),
        tr("Powerful, mobile ground unit."),
        tr("Capable of quickly transporting units that would otherwise travel by foot."),
        tr("Ranged ground to air unit, unable to move and attack in the same turn."),
        tr("Powerful long range unit, unable to move and attack in the same turn."),

        tr("Aircraft able to transport all ground units."),
        tr("Immensely mobile units, useful for crossing enemy lines."),
        tr("Air-to-air unit, able to create huge damage during critical hits."),
        tr("Incredibly powerful air-to-ground unit, especially during critical hits."),

        tr("Boats able to transport all ground units across water."),
        tr("Powerful naval unit, built to conquer the water."),
        tr("Ranged water unit, able to attack air and sea."),
        tr("An immensely powerful long range unit.")
    };

    m_sCommanderDescription = QStringList
    {
        tr("Most powerful ground unit type. Having no commanders may result in a loss.")
    };

    m_sBuildingDescription = QStringList
    {
        tr("Special buildings which cannot be captured. Losing it may result in a loss."),
        tr("Produces Land Units."),
        tr("Produces Air Units."),
        tr("Produces Naval Units."),
        tr("The primary building type, able to generate income for players."),
        tr("Functionally identical to Villages, but are positioned within water tiles.")
    };

    // Initialize terrain info
    m_terrainInfo = new int *[m_nTerrainNumber + 1];
    for (int i = 0; i <= m_nTerrainNumber; i++)
    {
        m_terrainInfo[i] = new int[m_nUnitType + 2];
    }

    // Initialize unit info
    m_unitInfo = new float *[m_nUnitNumber];
    for (int i = 0; i < m_nUnitNumber; i++)
    {
        m_unitInfo[i] = new float[12];
    }

    // Initialize unit info
    m_damageMatrix = new float *[m_nUnitNumber];
    for (int i = 0; i < m_nUnitNumber; i++)
    {
        m_damageMatrix[i] = new float[m_nUnitNumber];
    }

    loadFile();
}

GameInfo::~GameInfo()
{
    for (int i = 0; i <= m_nTerrainNumber; i++)
    {
        delete m_terrainInfo[i];
    }

    for (int i = 0; i < m_nUnitNumber; i++)
    {
        delete m_unitInfo[i];
        delete m_damageMatrix[i];
    }

    delete m_terrainInfo;
    delete m_unitInfo;
    delete m_damageMatrix;
}

void GameInfo::loadFile()
{
    // Terrain info
    QFile data(":/gameinfo/gameinfo/terrain_info.txt");
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&data);

    QStringList sizeStr = in.readLine().split(" ");
    if (sizeStr[0].toInt() != m_nTerrainNumber || sizeStr[1].toInt() != m_nUnitType)
    {
        qDebug() << "Size doesn't match";
        return;
    }

    for (int i = 1; i <= m_nTerrainNumber; i++)
    {
        QStringList rowStr = in.readLine().simplified().split(" ");
        for (int j = 1; j <= m_nUnitType + 2; j++)
        {
            if (j >= rowStr.size())
            {
                qDebug() << "Map incomplete";
                break;
            }
            m_terrainInfo[i][j - 1] = rowStr[j].toInt();
        }
    }

    data.close();

    // Unit info
    data.setFileName(":/gameinfo/gameinfo/unit_info.txt");
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    in.setDevice(&data);

    sizeStr = in.readLine().split(" ");
    if (sizeStr[0].toInt() != m_nUnitNumber)
    {
        qDebug() << "Size doesn't match";
        return;
    }

    for (int i = 0; i < m_nUnitNumber; i++)
    {
        QStringList rowStr = in.readLine().simplified().split(" ");
        for (int j = 1; j <= 12; j++)
        {
            if (j >= rowStr.size())
            {
                qDebug() << "Map incomplete";
                break;
            }
            m_unitInfo[i][j - 1] = rowStr[j].toFloat();
        }
    }

    data.close();

    // Damage matrix
    data.setFileName(":/gameinfo/gameinfo/damage_matrix.txt");
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    in.setDevice(&data);

    sizeStr = in.readLine().split(" ");
    if (sizeStr[0].toInt() != m_nUnitNumber)
    {
        qDebug() << "Size doesn't match";
        return;
    }

    for (int i = 0; i < m_nUnitNumber; i++)
    {
        QStringList rowStr = in.readLine().simplified().split(" ");
        for (int j = 1; j <= m_nUnitNumber; j++)
        {
            if (j >= rowStr.size())
            {
                qDebug() << "Map incomplete";
                break;
            }
            m_damageMatrix[i][j - 1] = rowStr[j].toFloat();
        }
    }

    data.close();

    qDebug() << "Game Info loaded";
}

int **GameInfo::getTerrainInfo() const
{
    return m_terrainInfo;
}

float **GameInfo::getUnitInfo() const
{
    return m_unitInfo;
}

float **GameInfo::getDamageMatrix() const
{
    return m_damageMatrix;
}

const QStringList &GameInfo::getUnitNames() const
{
    return m_sUnitNames;
}

const QStringList &GameInfo::getCommanderNames() const
{
    return m_sCommanderNames;
}

const QStringList &GameInfo::getBuildingNames() const
{
    return m_sBuildingNames;
}

const QStringList &GameInfo::getTerrainNames() const
{
    return m_sTerrainNames;
}

const QStringList &GameInfo::getUnitDescription() const
{
    return m_sUnitDescription;
}

const QStringList &GameInfo::getCommanderDescription() const
{
    return m_sCommanderDescription;
}

const QStringList &GameInfo::getBuildingDescription() const
{
    return m_sBuildingDescription;
}
