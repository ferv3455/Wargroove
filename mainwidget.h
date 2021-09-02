#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "gamewidget.h"

#include <QWidget>
#include <QPushButton>
#include <QTranslator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

public slots:
    void retranslate();
    void showGame();

private:
    Ui::MainWidget *ui;
    GameWidget *m_gameWidget;

    QPushButton *m_pushButton_1;
    QPushButton *m_pushButton_2;

    QTranslator m_translators[5];
    int m_nTranslatorId;
    int m_nTranslatorNumber;
};
#endif // MAINWIDGET_H
