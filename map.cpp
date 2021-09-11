#include "map.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

Map::Map(QSize size, QObject *parent, int blockSize, QPoint offset)
    : QObject(parent),
      m_size(size),
      m_pOffset(offset),
      m_nDynamicsId(0),
      m_nBlockSize(blockSize),
      m_nScale(100)
{
    m_matrix = new Block **[m_size.height()];
    for (int i = 0; i < m_size.height(); i++)
    {
        m_matrix[i] = new Block *[m_size.width()]();
    }
}

Map::~Map()
{
    for (int i = 0; i < m_size.height(); i++)
    {
        delete m_matrix[i];
    }

    delete m_matrix;
}

void Map::loadTerrain(const QString &filename, bool fogMode)
{
    QFile data(filename);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&data);

    QStringList sizeStr = in.readLine().split(" ");
    if (sizeStr[0].toInt() != m_size.height() || sizeStr[1].toInt() != m_size.width())
    {
        qDebug() << "Map size doesn't match";
        return;
    }

    for (int i = 0; i < m_size.height(); i++)
    {
        QStringList rowStr = in.readLine().simplified().split(" ");

        for (int j = 0; j < m_size.width(); j++)
        {
            if (j >= rowStr.size())
            {
                qDebug() << "Map incomplete";
                break;
            }

            int id = rowStr[j].toInt();
            if (id == 0)
            {
                m_matrix[i][j] = nullptr;
            }
            else
            {
                m_matrix[i][j] = new Block(id, i, j, this, fogMode);
            }
        }
    }

    qDebug() << "Map" << filename << "loaded";
    data.close();

    updateAllBlocks();
}

void Map::loadUnits(const QString &filename, GameInfo *gameInfo, GameStats *stats)
{
    QFile data(filename);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&data);
    in.readLine();          // skip comments

    float **unitInfo = gameInfo->getUnitInfo();

    while (!in.atEnd())
    {
        QStringList rowStr = in.readLine().simplified().split(" ");
        if (rowStr.size() == 5)
        {
            Block *block = getBlock(rowStr[0].toInt(), rowStr[1].toInt());
            if (block != nullptr && block->getUnit() == nullptr)
            {
                int unitId = rowStr[3].toInt();
                int side = rowStr[2].toInt();
                block->setUnit(unitId, side, unitInfo[unitId][6], rowStr[4].toInt());
                if (side >= 0)
                {
                    stats->addUnit(block->getUnit());
                }
            }
        }
    }

    qDebug() << "Units" << filename << "loaded";
    data.close();
}

Block *Map::getBlock(int row, int col) const
{
    if (row < 0 || row >= m_size.height() || col < 0 || col >= m_size.width())
        return nullptr;

    return m_matrix[row][col];
}

Block *Map::getBlock(QPoint position) const
{
    int newBlockSize = m_nBlockSize * m_nScale / 100;
    int deltaX = newBlockSize * 2;
    int deltaY = newBlockSize * 1.8;

    int row = (position.y() - m_pOffset.y() + deltaY) / deltaY;
    if (row < 0 || row > m_size.height())
    {
        return nullptr;         // outside the map
    }
    int col = (position.x() - m_pOffset.x() + deltaX / 2) / deltaX;
    if (col < 0 || col > m_size.width())
    {
        return nullptr;         // outside the map
    }

    // Narrow down to two columns and two rows: row-1~row, col-1~col
    for (int i = row - 1; i <= row; i++)
    {
        if (i < 0 || i >= m_size.height())
        {
            continue;
        }

        for (int j = col - 1; j <= col; j++)
        {
            if (j < 0 || j >= m_size.width())
            {
                continue;
            }

            if (m_matrix[i][j]->getArea()->containsPoint(position, Qt::OddEvenFill))
            {
                return m_matrix[i][j];
            }
        }
    }

    return nullptr;
}

QSize Map::getSize() const
{
    return m_size;
}

int Map::getDynamicsId() const
{
    return m_nDynamicsId;
}

QPoint Map::getCenterPosition(const Block *block) const
{
    int row = block->getRow();
    int col = block->getColumn();

    int newBlockSize = m_nBlockSize * m_nScale / 100;
    int deltaX = newBlockSize * 2;
    int deltaY = newBlockSize * 1.8;

    QPoint pos = m_pOffset + QPoint((row % 2 ? newBlockSize : 0) + col * deltaX,
                                    row * deltaY);
    return pos;
}

int Map::getBlockSize() const
{
    return m_nBlockSize * m_nScale / 100;
}

int Map::getScale() const
{
    return m_nScale;
}

void Map::getAdjacentBlocks(QVector<Block *> &blockVector, Block *block) const
{
    if (block == nullptr)
    {
        return;
    }

    static int deltaCoordinates[2][6][2] =
    {
        {{-1, -1}, {-1, 0}, {0, -1}, {0, 1}, {1, -1}, {1, 0}},        // even rows
        {{-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, 0}, {1, 1}}         // odd rows
    };          // index delta of blocks that are adjacent to the selected block

    blockVector.clear();
    int row = block->getRow();
    int col = block->getColumn();
    int parity = row % 2;

    for (int i = 0; i < 6; i++)
    {
        Block *temp = getBlock(row + deltaCoordinates[parity][i][0], col + deltaCoordinates[parity][i][1]);
        if (temp != nullptr)
        {
            blockVector.push_back(temp);
        }
    }
}

void Map::getAdjacentBlocks(QVector<Block *> &blockVector, Block *block, int rangeHigh, int rangeLow) const
{
    if (block == nullptr)
    {
        return;
    }

    struct Vertex
    {
        Block *block;
        int distance;
    };

    QVector<Vertex> remainingBlocks;
    QVector<Vertex> countedBlocks;
    remainingBlocks.push_back({block, 0});    // the vector only has the origin at first

    while (!remainingBlocks.isEmpty())
    {
        // sort by distance, get the one with the shortest distance
        std::sort(remainingBlocks.begin(), remainingBlocks.end(), [](const Vertex & v1, const Vertex & v2)
        {
            return v1.distance > v2.distance;
        });

        const Vertex temp = remainingBlocks.last();
        remainingBlocks.pop_back();
        if (temp.distance > rangeHigh)           // other blocks are all out of reach
        {
            break;
        }

        QVector<Block *> adjacentBlocks;
        getAdjacentBlocks(adjacentBlocks, temp.block);

        for (const auto &adjBlock : qAsConst(adjacentBlocks))
        {
            int distance = temp.distance + 1;
            bool counted = false;

            // find out whether the block is counted
            for (auto iter = remainingBlocks.begin(); iter != remainingBlocks.end(); iter++)
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
                for (auto iter = countedBlocks.begin(); iter != countedBlocks.end(); iter++)
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
                    // not counted
                    remainingBlocks.push_back({adjBlock, distance});
                }
            }
        }

        countedBlocks.push_back(temp);
    }

    blockVector.clear();
    for (const auto &vertex : qAsConst(countedBlocks))
    {
        if (vertex.distance >= rangeLow)
        {
            blockVector.append(vertex.block);
        }
    }
}

void Map::getAllBlocks(QVector<Block *> &blockVector, int side) const
{
    blockVector.clear();
    int rows = m_size.height();
    int cols = m_size.width();

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Block *block = getBlock(i, j);
            if (block != nullptr && block->getUnit() != nullptr && block->getUnit()->getSide() == side)
            {
                blockVector.push_back(block);
            }
        }
    }
}

void Map::adjustOffset(QPoint deltaPos)
{
    m_pOffset += deltaPos;
    updateAllBlocks();
}

void Map::adjustScale(int deltaScale, QPointF center)
{
    if ((m_nScale <= 30 && deltaScale < 0) || (m_nScale >= 150 && deltaScale > 0))
    {
        return;
    }

    center -= m_pOffset;            // relative position
    QPointF deltaOffset = (deltaScale * 1.0 / m_nScale) * center;
    m_pOffset -= deltaOffset.toPoint();
    m_nScale += deltaScale;
    updateAllBlocks();
}

void Map::updateAllBlocks() const
{
    int rows = m_size.height();
    int cols = m_size.width();

    int newBlockSize = m_nBlockSize * m_nScale / 100;
    int deltaX = newBlockSize * 2;
    int deltaY = newBlockSize * 1.8;
    QPoint pos = m_pOffset;

    for (int i = 0; i < rows; i++)
    {
        pos.setX(m_pOffset.x() + (i % 2 ? newBlockSize : 0));

        for (int j = 0; j < cols; j++)
        {
            Block *block = getBlock(i, j);
            if (block != nullptr)
            {
                block->updateArea(pos, newBlockSize);
            }

            pos.setX(pos.x() + deltaX);
        }

        pos.setY(pos.y() + deltaY);
    }
}

void Map::paint(QPainter *painter, int part) const
{
    int rows = m_size.height();
    int cols = m_size.width();
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Block *block = getBlock(i, j);
            if (block != nullptr)
            {
                block->paint(painter, part, m_nDynamicsId);
            }
        }
    }
}

void Map::updateDynamics()
{
    m_nDynamicsId = 1 - m_nDynamicsId;
}

