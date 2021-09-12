#include "settings.h"

Settings::Settings(QObject *parent)
    : QObject(parent),
      m_nRefreshTime(15),                          // refresh time  WARNING: game might crash if too low
      m_nMoveSteps(4),                             // unit move steps

      m_mapNames{"ai_test", "standard", "lake", "mountain", "river_delta"},           // map names
      m_mapSizes{QSize(20, 20), QSize(20, 20), QSize(20, 20), QSize(20, 20), QSize(50, 59)}, // map sizes (width, height)
      m_nCurrentMap(0),                            // current map

      m_nBlockSize(30),                            // initial size of a block
      m_nZoomScale(10),                            // scale change per scroll

      m_bFogMode(true),                            // fog mode
      m_bAI(false),                                // use AI

      m_backgroundMusic("./music/bgm05.mp3"),      // bgm filename
      m_nVolume(5),                                // music volume
      m_nSEVolume(15)                              // sound effect volume
{

}
