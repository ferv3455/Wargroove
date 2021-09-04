#ifndef UNIT_H
#define UNIT_H

#include <QObject>
#include <QPainter>
#include <QImage>

class Unit : public QObject
{
    Q_OBJECT
public:
    explicit Unit(int unitId, int side, QObject *parent = nullptr);
    void paint(QPainter *painter, const QRect &rect, int dynamicsId) const;

    // Getters and setters
    int getId() const;
    int getSide() const;
    bool getActivity() const;
    void setDirection(int direction);
    void setActivity(bool active);

    // Convert Image color to gray
    static QImage grayImage(const QImage *image);

private:
    int m_nId;               // value: 0~20
    int m_nSide;             // side of the unit: 0~1 (0-player)
    QImage m_images[4];      // images: 2 facing right, 2 left
    int m_nDirection;        // value: 0-right, 1-left
    bool m_bActive;          // can be moved in this round

signals:

};

#endif // UNIT_H
