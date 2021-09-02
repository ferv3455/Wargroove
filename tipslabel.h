#ifndef TIPSLABEL_H
#define TIPSLABEL_H

#include <QLabel>
#include <QTimer>

class TipsLabel : public QLabel
{
public:
    TipsLabel(const QString &text, QWidget *parent = nullptr);
    void popup(const QString &text);

public slots:
    void updateBackground();

private:
    QTimer *m_timer;
    int m_nOpacity;
};

#endif // TIPSLABEL_H
