#ifndef GAMEPROCESSOR_H
#define GAMEPROCESSOR_H

#include <QObject>

class GameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit GameProcessor(QObject *parent = nullptr);

signals:

};

#endif // GAMEPROCESSOR_H
