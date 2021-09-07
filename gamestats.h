#ifndef GAMESTATS_H
#define GAMESTATS_H

#include "unit.h"

#include <QObject>

class GameStats : public QObject
{
    Q_OBJECT
public:
    friend class GameProcessor;
    explicit GameStats(QObject *parent = nullptr, int coins = 100);

    // Getters and setters
    int getCoins() const;
    const QVector<Unit *> &getPlayerUnits() const;
    const QVector<Unit *> &getEnemyUnits() const;

private:
    int m_nCoins;
    QVector<Unit *> m_playerUnits;
    QVector<Unit *> m_enemyUnits;

signals:

};

#endif // GAMESTATS_H
