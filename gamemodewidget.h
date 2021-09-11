#ifndef GAMEMODEWIDGET_H
#define GAMEMODEWIDGET_H

#include "settings.h"

#include <QWidget>

namespace Ui
{
class GameModeWidget;
}

class GameModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameModeWidget(Settings *settings, QWidget *parent = nullptr);
    ~GameModeWidget();

public slots:
    void changeMap(int index);

private:
    Ui::GameModeWidget *ui;
    Settings *m_settings;

signals:
    void windowClosed();
    void startGame();
};

#endif // GAMEMODEWIDGET_H
