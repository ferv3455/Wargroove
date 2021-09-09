#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <QObject>

class GameInfo : public QObject
{
    Q_OBJECT
public:
    explicit GameInfo(QObject *parent = nullptr);
    ~GameInfo();
    void loadFile();

    int **getTerrainInfo() const;
    float **getUnitInfo() const;
    float **getDamageMatrix() const;

    const QStringList &getUnitNames() const;
    const QStringList &getCommanderNames() const;
    const QStringList &getBuildingNames() const;
    const QStringList &getTerrainNames() const;

private:
    int **m_terrainInfo;
    float **m_unitInfo;
    float **m_damageMatrix;

    int m_nTerrainNumber;
    int m_nUnitType;
    int m_nUnitNumber;

    QStringList m_sUnitNames;
    QStringList m_sCommanderNames;
    QStringList m_sBuildingNames;
    QStringList m_sTerrainNames;
};

#endif // GAMEINFO_H
