#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "gamewidget.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    GameWidget *m_gameWidget;

};
#endif // MAINWIDGET_H
