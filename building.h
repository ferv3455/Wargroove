#ifndef BUILDING_H
#define BUILDING_H

#include "unit.h"

class Building : public Unit
{
public:
    explicit Building(int unitId, int side, int maxHP, QObject *parent = nullptr, int innerType = 0);
    virtual void paint(QPainter *painter, const QRect &rect, int dynamicsId = 0, int side = -8) const;

    // NEW Getter and setter
    void setSide(int side);
    virtual void getImages(QVector<QImage> *images) const;
    virtual bool isOperable() const;
};

#endif // BUILDING_H
