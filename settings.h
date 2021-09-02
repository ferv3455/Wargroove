#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSize>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);

public:
    int m_nRefreshTime;         // milliseconds
    int m_nMoveTime;            // milliseconds

    QString m_mapFileName;
    QSize m_mapSize;
    int m_nBlockSize;
    int m_nZoomScale;

    QString m_backgroundMusic;
    int m_nVolume;
};

#endif // SETTINGS_H