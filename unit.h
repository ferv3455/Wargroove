#ifndef UNIT_H
#define UNIT_H

#include <QObject>
#include <QPainter>
#include <QImage>

class Unit : public QObject
{
    Q_OBJECT

public:
    explicit Unit(int unitId, int side, int maxHP, QObject *parent = nullptr,
                  int innerType = 0, float HPPercentage = 1.0);
    virtual void paint(QPainter *painter, const QRect &rect, int dynamicsId = 0, int side = -8) const;

    // Getters and setters
    int getId() const;
    int getInnerType() const;
    int getSide() const;
    bool getActivity() const;
    int getHP() const;
    int getMaxHP() const;
    float getHPPercentage() const;
    Unit *getCarrier() const;
    virtual void getImages(QVector<QImage> *images) const;

    virtual bool isOperable() const;
    bool isCarrier() const;

    void setDirection(int direction);
    void setActivity(bool active);
    void setCarrier(Unit *unit);

    // Game-related
    bool injured(int damage);           // return true if killed
    bool checkCritical() const;         // check if reach critical-hit criteria
    void regenerate(double ratio);      // recover by ratio (maxHP)

    // Convert Image color to gray
    static QImage grayImage(const QImage *image);

protected:
    // Attributes
    int m_nId;               // value: 0~20
    int m_nInnerType;        // inner type of the unit (e.g. three commanders)
    QImage m_images[4];      // images: 2 facing right, 2 left

    // Properties
    int m_nSide;             // side of the unit: 0~1 (0-player)
    int m_nDirection;        // value: 0-right, 1-left
    bool m_bActive;          // can be moved in this round
    Unit *m_carryingUnit;    // (just for wagons, balloons and barges) the unit they carry

    // Game info
    int m_nHealthPoint;      // HP
    int m_nMaxHealthPoint;   // max HP

signals:

};

#endif // UNIT_H
