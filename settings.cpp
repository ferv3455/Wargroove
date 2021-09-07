#include "settings.h"

Settings::Settings(QObject *parent)
    : QObject(parent),
      m_nRefreshTime(5),                           // refresh time
      m_nMoveSteps(6),                             // unit move steps

      m_mapTerrainFileName(":/maps/terrain/river_delta"),  // map terrain filename
      m_mapUnitsFileName(":/maps/units/river_delta"),      // map units filename
      m_mapSize(50, 59),                           // width, height
      m_nBlockSize(30),                            // initial size of a block
      m_nZoomScale(10),                            // scale change per scroll

      m_backgroundMusic("./music/bgm05.mp3"),      // bgm filename
      m_nVolume(5)                                 // music volume
{

}
