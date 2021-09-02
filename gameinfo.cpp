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

    qDebug() << "Terrain info loaded";
    data.close();

    // Unit info
    QFile data2(":/gameinfo/gameinfo/unit_info.txt");
    if (!data2.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    in.setDevice(&data2);

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

    qDebug() << "Unit info loaded";
    data2.close();

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

    qDebug() << "Damage matrix loaded";
    data.close();
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
