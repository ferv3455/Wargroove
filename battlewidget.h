#ifndef BATTLEWIDGET_H
#define BATTLEWIDGET_H

#include "block.h"
#include "gameinfo.h"

#include <QWidget>
#include <QLabel>
#include <QTimer>

namespace Ui
{
class BattleWidget;
}

class BattleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BattleWidget(Block *activeBlock, Block *passiveBlock,
                          GameInfo *gameInfo, QWidget *parent = nullptr);
    ~BattleWidget();

    void setActiveAttack(int damage, bool killed);
    void setPassiveAttack(int damage, bool killed);

    void adjustScreen();
    void begin();

public slots:
    void updateDynamics();

    void goOnStage();
    void activeAttack();
    void passiveAttack();
    void ending();

private:
    Ui::BattleWidget *ui;

    // Data
    GameInfo *m_gameInfo;

    // Widgets
    QLabel *m_activeLabel;
    QLabel *m_passiveLabel;
    QLabel *m_damageLabel;

    // Images
    QVector<QPixmap> m_activeImages;
    QVector<QPixmap> m_passiveImages;
    int m_nDynamicsId = 0;

    // Timer
    QTimer *m_dynamicsTimer;

    // Data
    Block *m_activeBlock;
    Block *m_passiveBlock;
    int m_nActiveDamage;
    int m_nPassiveDamage;
    bool m_bActiveKilled;
    bool m_bPassiveKilled;

signals:
    void end();
};

#endif // BATTLEWIDGET_H
