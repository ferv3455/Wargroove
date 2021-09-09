#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "gamewidget.h"
#include "settingswidget.h"

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

    virtual void resizeEvent(QResizeEvent *event);

public slots:
    void retranslate();
    void showGame();
    void showSettings();
    void returnToMenu();
    void resetMedia(QMediaPlayer::State);
    void setMediaVolume(int);

private:
    Ui::MainWidget *ui;
    GameWidget *m_gameWidget;
    SettingsWidget *m_settingsWidget;

    QPushButton *m_pushButton_1;
    QPushButton *m_pushButton_2;

    QTranslator m_translators[5];
    int m_nTranslatorId;
    int m_nTranslatorNumber;

    QMediaPlayer *m_bgMusic;
    bool m_bStopped;

    // Game settings
    Settings *m_settings;
};
#endif // MAINWIDGET_H
