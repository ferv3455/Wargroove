#include "gameprocessor.h"

GameProcessor::GameProcessor(Settings *settings,
                             GameInfo *gameInfo,
                             Map *map,
                             TipsLabel *tipsLabel,
                             QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_gameInfo(gameInfo),
      m_map(map),
      m_tipsLabel(tipsLabel),
      m_nStage(0)
{
    m_unitMover = new UnitMover(m_settings, m_map, this);
}

void GameProcessor::paint(QPainter *painter)
{
    m_unitMover->paint(painter);
}

void GameProcessor::zoomMap(int direction, QPointF point)
{
    m_map->adjustScale(direction * m_settings->m_nZoomScale, point);
    m_tipsLabel->popup(tr("Map Scale: ") + QString::number(m_map->getScale()) + "\%");
}

