#ifndef GAMESTATS_H
#define GAMESTATS_H

#include "unit.h"

#include <QObject>

class GameStats : public QObject
{
    Q_OBJECT
public:
    explicit GameStats(QObject *parent = nullptr, int totalSides = 2, int coins = 100);

    // Getters and setters
    int getCoins(int side) const;
    void addCoins(int deltaCoins, int side);
    const QVector<Unit *> &getUnits(int side) const;
    void addUnit(Unit *unit);
    void removeUnit(Unit *unit);

private:
    int m_nTotalSides;
    int *m_nCoins;
    QVector<Unit *> *m_units;

signals:

};

#endif // GAMESTATS_H
