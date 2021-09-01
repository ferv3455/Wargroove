#include "settings.h"

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_nRefreshTime(10)                         // refresh time
    , m_nMoveTime(200)                           // unit move time

    , m_mapFileName(":/maps/river_delta")        // map name
    , m_mapSize(50, 59)                          // width, height
    , m_nBlockSize(30)                           // initial size of a block
    , m_nZoomScale(10)                           // scale change per scroll

    , m_backgroundMusic("./music/bgm05.mp3")     // bgm filename
    , m_nVolume(10)                              // music volume
{

}
