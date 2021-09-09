#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "settings.h"

#include <QWidget>

namespace Ui
{
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(Settings *settings, QWidget *parent = nullptr);
    ~SettingsWidget();

    virtual void closeEvent(QCloseEvent *event);

public slots:
    void changeMap(int);

private:
    Ui::SettingsWidget *ui;
    Settings *m_settings;

signals:
    void windowClosed();
};

#endif // SETTINGSWIDGET_H
