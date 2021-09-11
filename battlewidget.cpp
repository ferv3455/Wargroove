#include "battlewidget.h"
#include "ui_battlewidget.h"

#include <QPropertyAnimation>

BattleWidget::BattleWidget(Block *activeBlock, Block *passiveBlock,
                           GameInfo *gameInfo, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::BattleWidget),
      m_gameInfo(gameInfo),

      m_activeImages(),
      m_passiveImages(),
      m_nDynamicsId(0),

      m_activeBlock(activeBlock),
      m_passiveBlock(passiveBlock),
      m_nActiveDamage(0),
      m_nPassiveDamage(0),
      m_bActiveCritical(false),
      m_bActiveKilled(false),
      m_bPassiveKilled(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->imageWidget->setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(900, 400);
    m_activeLabel = new QLabel(ui->imageWidget);
    m_passiveLabel = new QLabel(ui->imageWidget);
    m_damageLabel = new QLabel(ui->imageWidget);

    m_damageLabel->setGeometry(-100, -100, 300, 50);

    // Initialize images
    QVector<QImage> v;

    m_activeBlock->getUnit()->getImages(&v);
    for (const auto &img : qAsConst(v))
    {
        m_activeImages.push_back(QPixmap::fromImage(img.scaledToHeight(150)));
    }
    m_activeLabel->setPixmap(m_activeImages[m_nDynamicsId]);

    m_passiveBlock->getUnit()->getImages(&v);
    for (const auto &img : qAsConst(v))
    {
        m_passiveImages.push_back(QPixmap::fromImage(img.scaledToHeight(150).mirrored(true, false)));
    }
    m_passiveLabel->setPixmap(m_passiveImages[m_nDynamicsId]);

    // Initialize labels
    ui->activeAttackLabel->setText(
        QString::number(static_cast<int>(m_gameInfo->getUnitInfo()[m_activeBlock->getUnit()->getId()][5])));
    ui->passiveAttackLabel->setText(
        QString::number(static_cast<int>(m_gameInfo->getUnitInfo()[m_passiveBlock->getUnit()->getId()][5])));
    ui->activeShieldLabel->setText(
        QString::number(m_gameInfo->getTerrainInfo()[m_passiveBlock->getTerrain()][5]));
    ui->passiveShieldLabel->setText(
        QString::number(m_gameInfo->getTerrainInfo()[m_passiveBlock->getTerrain()][5]));
    ui->activeHPBar->setValue(100 * m_activeBlock->getUnit()->getHPPercentage());
    ui->activeHPLabel->setText(QString::number(m_activeBlock->getUnit()->getHP()));
    ui->passiveHPBar->setValue(100 * m_passiveBlock->getUnit()->getHPPercentage());
    ui->passiveHPLabel->setText(QString::number(m_passiveBlock->getUnit()->getHP()));

    // Load background picture
    QImage img;
    Unit *unit = m_passiveBlock->getUnit();
    if (unit != nullptr)
    {
        int unitId = unit->getId();
        if (unitId >= 10 && unitId <= 13)
        {
            // Air unit
            img = QImage(":/image/background/0");
        }
        else
        {
            img = QImage(":/image/background/" + QString::number(m_passiveBlock->getTerrain()));
        }
    }
    else
    {
        img = QImage(":/image/background/" + QString::number(m_passiveBlock->getTerrain()));
    }

    show();
    // Set background picture
    QPalette palette = ui->imageWidget->palette();
    palette.setBrush(ui->imageWidget->backgroundRole(),
                     QBrush(img.scaled(ui->imageWidget->size(),
                                       Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    ui->imageWidget->setPalette(palette);

    // Initialize timer
    m_dynamicsTimer = new QTimer(this);
    m_dynamicsTimer->start(300);

    // Connect all animations
    connect(m_dynamicsTimer, &QTimer::timeout, this, &BattleWidget::updateDynamics);
}

BattleWidget::~BattleWidget()
{
    delete ui;
}

void BattleWidget::setActiveAttack(int damage, bool killed, bool critical)
{
    m_nActiveDamage = damage;
    m_bActiveCritical = critical;
    m_bActiveKilled = killed;
}

void BattleWidget::setPassiveAttack(int damage, bool killed)
{
    m_nPassiveDamage = damage;
    m_bPassiveKilled = killed;
}

void BattleWidget::adjustScreen()
{
    // Reset size
    int parentWidth = static_cast<QWidget *>(parent())->width();
    int parentHeight = static_cast<QWidget *>(parent())->height();
    setGeometry(parentWidth / 2 - 450, parentHeight / 2 - 200, 900, 400);
}

void BattleWidget::begin()
{
    show();
    adjustScreen();
    goOnStage();
}

void BattleWidget::updateDynamics()
{
    m_nDynamicsId = 1 - m_nDynamicsId;
    m_activeLabel->setPixmap(m_activeImages[m_nDynamicsId]);
    m_passiveLabel->setPixmap(m_passiveImages[m_nDynamicsId]);
}

void BattleWidget::goOnStage()
{
    QPropertyAnimation *activeAnimation = new QPropertyAnimation(m_activeLabel, "pos", this);
    activeAnimation->setDuration(500);
    activeAnimation->setStartValue(QPoint(-300, 70));
    activeAnimation->setEndValue(QPoint(100, 70));
    activeAnimation->setEasingCurve(QEasingCurve::Linear);

    QPropertyAnimation *passiveAnimation = new QPropertyAnimation(m_passiveLabel, "pos", this);
    passiveAnimation->setDuration(500);
    passiveAnimation->setStartValue(QPoint(1050, 70));
    passiveAnimation->setEndValue(QPoint(650, 70));
    passiveAnimation->setEasingCurve(QEasingCurve::Linear);

    m_activeLabel->show();
    m_passiveLabel->show();
    activeAnimation->start();
    passiveAnimation->start();

    connect(passiveAnimation, &QPropertyAnimation::finished, this, &BattleWidget::activeAttack);
}

void BattleWidget::activeAttack()
{
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(1000);

    QPropertyAnimation *activeAttackAnimation = new QPropertyAnimation(m_activeLabel, "pos", this);
    activeAttackAnimation->setDuration(200);
    activeAttackAnimation->setStartValue(QPoint(100, 70));
    activeAttackAnimation->setEndValue(QPoint(1000, 70));
    activeAttackAnimation->setEasingCurve(QEasingCurve::Linear);

    QPropertyAnimation *activeReturnAnimation = new QPropertyAnimation(m_activeLabel, "pos", this);
    activeReturnAnimation->setDuration(500);
    activeReturnAnimation->setStartValue(QPoint(-300, 70));
    activeReturnAnimation->setEndValue(QPoint(100, 70));
    activeReturnAnimation->setEasingCurve(QEasingCurve::Linear);

    m_damageLabel->setText((m_bActiveCritical ? "CRIT. " : " ") + QString::number(-m_nActiveDamage));

    QPropertyAnimation *activeDamageAnimation = new QPropertyAnimation(m_damageLabel, "pos", this);
    activeDamageAnimation->setDuration(1000);
    activeDamageAnimation->setStartValue(QPoint(700, 100));
    activeDamageAnimation->setEndValue(QPoint(700, -50));
    activeDamageAnimation->setEasingCurve(QEasingCurve::Linear);

    // start() has a default parameter, so I have to use SIGNAL(), SLOT()
    connect(activeAttackAnimation, SIGNAL(finished()), activeReturnAnimation, SLOT(start()));
    connect(activeAttackAnimation, SIGNAL(finished()), activeDamageAnimation, SLOT(start()));

    if (m_bActiveKilled)
    {
        QPropertyAnimation *passiveKilledAnimation = new QPropertyAnimation(m_passiveLabel, "pos", this);
        passiveKilledAnimation->setDuration(800);
        passiveKilledAnimation->setStartValue(QPoint(650, 70));
        passiveKilledAnimation->setEndValue(QPoint(650, 400));
        passiveKilledAnimation->setEasingCurve(QEasingCurve::Linear);

        connect(activeAttackAnimation, SIGNAL(finished()), passiveKilledAnimation, SLOT(start()));
        connect(activeAttackAnimation, &QPropertyAnimation::finished, this, [ = ]()
        {
            ui->passiveHPBar->setValue(0);
            ui->passiveHPLabel->setText(QString::number(0));
        });
        connect(activeDamageAnimation, &QPropertyAnimation::finished, this, &BattleWidget::ending);
    }
    else
    {
        connect(activeAttackAnimation, &QPropertyAnimation::finished, this, [ = ]()
        {
            ui->passiveHPBar->setValue(m_passiveBlock->getUnit()->getHPPercentage() * 100);
            ui->passiveHPLabel->setText(QString::number(m_passiveBlock->getUnit()->getHP()));
        });

        if (m_passiveBlock->getUnit() != nullptr && m_nPassiveDamage == 0)
        {
            connect(activeDamageAnimation, &QPropertyAnimation::finished, this, &BattleWidget::ending);
        }
        else
        {
            connect(activeDamageAnimation, &QPropertyAnimation::finished, this, &BattleWidget::passiveAttack);
        }
    }

    connect(timer, SIGNAL(timeout()), activeAttackAnimation, SLOT(start()));
    timer->start(1000);
}

void BattleWidget::passiveAttack()
{
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(1000);

    QPropertyAnimation *passiveAttackAnimation = new QPropertyAnimation(m_passiveLabel, "pos", this);
    passiveAttackAnimation->setDuration(200);
    passiveAttackAnimation->setStartValue(QPoint(650, 70));
    passiveAttackAnimation->setEndValue(QPoint(-250, 70));
    passiveAttackAnimation->setEasingCurve(QEasingCurve::Linear);

    QPropertyAnimation *passiveReturnAnimation = new QPropertyAnimation(m_passiveLabel, "pos", this);
    passiveReturnAnimation->setDuration(500);
    passiveReturnAnimation->setStartValue(QPoint(1050, 70));
    passiveReturnAnimation->setEndValue(QPoint(650, 70));
    passiveReturnAnimation->setEasingCurve(QEasingCurve::Linear);

    m_damageLabel->setText(QString::number(-m_nPassiveDamage));

    QPropertyAnimation *passiveDamageAnimation = new QPropertyAnimation(m_damageLabel, "pos", this);
    passiveDamageAnimation->setDuration(1000);
    passiveDamageAnimation->setStartValue(QPoint(150, 100));
    passiveDamageAnimation->setEndValue(QPoint(150, -50));
    passiveDamageAnimation->setEasingCurve(QEasingCurve::Linear);

    // start() has a default parameter, so I have to use SIGNAL(), SLOT()
    connect(passiveAttackAnimation, SIGNAL(finished()), passiveReturnAnimation, SLOT(start()));
    connect(passiveAttackAnimation, SIGNAL(finished()), passiveDamageAnimation, SLOT(start()));

    if (m_bPassiveKilled)
    {
        QPropertyAnimation *activeKilledAnimation = new QPropertyAnimation(m_activeLabel, "pos", this);
        activeKilledAnimation->setDuration(800);
        activeKilledAnimation->setStartValue(QPoint(100, 70));
        activeKilledAnimation->setEndValue(QPoint(100, 400));
        activeKilledAnimation->setEasingCurve(QEasingCurve::Linear);

        connect(passiveAttackAnimation, SIGNAL(finished()), activeKilledAnimation, SLOT(start()));
        connect(passiveAttackAnimation, &QPropertyAnimation::finished, this, [ = ]()
        {
            ui->activeHPBar->setValue(0);
            ui->activeHPLabel->setText(QString::number(0));
        });
    }
    else
    {
        connect(passiveAttackAnimation, &QPropertyAnimation::finished, this, [ = ]()
        {
            ui->activeHPBar->setValue(m_activeBlock->getUnit()->getHPPercentage() * 100);
            ui->activeHPLabel->setText(QString::number(m_activeBlock->getUnit()->getHP()));
        });
    }

    connect(passiveDamageAnimation, &QPropertyAnimation::finished, this, &BattleWidget::ending);
    connect(timer, SIGNAL(timeout()), passiveAttackAnimation, SLOT(start()));
    timer->start(1000);
}

void BattleWidget::ending()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &BattleWidget::end);
    timer->setSingleShot(true);
    timer->start(1000);
}
