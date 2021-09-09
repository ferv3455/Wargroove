#include "settings.h"

Settings::Settings(QObject *parent)
    : QObject(parent),
      m_nRefreshTime(5),                           // refresh time
      m_nMoveSteps(6),                             // unit move steps

      m_mapNames{"standard", "river_delta", "tiny"},            // map names
      m_mapSizes{QSize(20, 20), QSize(50, 59), QSize(10, 11)},  // map sizes (width, height)
      m_nCurrentMap(0),                                         // current map

      m_nBlockSize(30),                            // initial size of a block
      m_nZoomScale(10),                            // scale change per scroll

      m_backgroundMusic("./music/bgm05.mp3"),      // bgm filename
      m_nVolume(5),                                // music volume
      m_nSEVolume(15)                                 // sound effect volume
{

}
