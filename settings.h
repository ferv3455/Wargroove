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
    int m_nMoveSteps;

    QStringList m_mapNames;
    QList<QSize> m_mapSizes;
    int m_nCurrentMap;

    int m_nBlockSize;
    int m_nZoomScale;

    QString m_backgroundMusic;
    int m_nVolume;
    int m_nSEVolume;
};

#endif // SETTINGS_H
