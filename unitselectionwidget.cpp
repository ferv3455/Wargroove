#include "unitselectionwidget.h"
#include "ui_unitselectionwidget.h"

#include <QPushButton>
#include <QDebug>

UnitSelectionWidget::UnitSelectionWidget(GameInfo *gameInfo, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UnitSelectionWidget),
    m_listWidgetItems(),
    m_gameInfo(gameInfo),
    m_nKind(0)
{
    ui->setupUi(this);

    m_descriptionWidget = new DescriptionWidget(m_gameInfo, this, true);
    ui->horizontalLayout->addWidget(m_descriptionWidget);
    m_descriptionWidget->setUnit(nullptr, 0);
    m_descriptionWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

    m_listWidget = ui->listWidget;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    setAttribute(Qt::WA_StyledBackground, true);
    hide();

    connect(m_listWidget, &QListWidget::currentItemChanged, this, &UnitSelectionWidget::updateButtonValidity);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &UnitSelectionWidget::selected);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &UnitSelectionWidget::cancel);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &UnitSelectionWidget::hide);
}

UnitSelectionWidget::~UnitSelectionWidget()
{
    delete ui;
}

void UnitSelectionWidget::loadUnits(const QStringList &unitNames, float **unitInfo)
{
    for (int i = 0; i < 18; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(m_listWidget, unitInfo[i][10]);
        item->setSizeHint(QSize(100, 80));

        ItemWidget *w = new ItemWidget(unitNames[i], unitInfo[i][9],
                                       QPixmap(":/image/unit/" + QString::number(i)).copy(0, 0, 64, 64),
                                       m_listWidget);
        m_listWidget->setItemWidget(item, w);
        m_listWidgetItems.push_back(item);
    }
}

void UnitSelectionWidget::showUnits(int unitKind, int coins)
{
    // Set kind
    static int kindList[3] = {1, 0, 7};
    m_nKind = kindList[unitKind];
    m_descriptionWidget->setUnit(nullptr, m_nKind);

    // Set coins
    ui->currentCoinsLabel->setText(QString::number(coins));

    // Hide other items
    for (const auto &item : qAsConst(m_listWidgetItems))
    {
        if (item->type() == unitKind)
        {
            item->setHidden(false);
            ItemWidget *w = static_cast<ItemWidget *>(m_listWidget->itemWidget(item));
            if (w->m_cost <= coins)
            {
                w->setValidity(true);
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
            }
            else
            {
                w->setValidity(false);
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            }
        }
        else
        {
            item->setHidden(true);
        }
    }

    // Adjust screen size
    adjustSize();

    // Reset selection
    m_listWidget->setCurrentItem(nullptr);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    show();
    m_descriptionWidget->show();
    m_descriptionWidget->adjustBackground();
}

void UnitSelectionWidget::adjustSize()
{
    int parentWidth = static_cast<QWidget *>(parent())->width();
    int parentHeight = static_cast<QWidget *>(parent())->height();
    setGeometry(parentWidth / 6, parentHeight / 6, parentWidth * 2 / 3, parentHeight * 2 / 3);
    m_descriptionWidget->show();
    m_descriptionWidget->adjustBackground();
}

void UnitSelectionWidget::selected()
{
    if (m_listWidget->currentItem() == nullptr)
    {
        return;
    }

    int selectedId = m_listWidgetItems.indexOf(m_listWidget->currentItem());
    emit confirm(selectedId);
    hide();
}

void UnitSelectionWidget::updateButtonValidity()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(item != nullptr);
    if (item == nullptr)
    {
        m_descriptionWidget->setUnit(nullptr, m_nKind);
    }
    else
    {
        int selectedId = m_listWidgetItems.indexOf(item);
        Unit unit(selectedId, 0, m_gameInfo->getUnitInfo()[selectedId][6], this);
        m_descriptionWidget->setUnit(&unit, m_nKind);
    }
    m_descriptionWidget->adjustBackground();
}

UnitSelectionWidget::ItemWidget::ItemWidget(const QString &name, int cost,
        const QPixmap &image, QWidget *parent) : QWidget(parent), m_cost(cost)
{
    QHBoxLayout *widgetLayout = new QHBoxLayout(this);
    widgetLayout->setContentsMargins(20, 0, 20, 0);
    QLabel *pic = new QLabel(this);
    pic->setPixmap(image);
    widgetLayout->addWidget(pic);
    QLabel *nameLabel = new QLabel(name, this);
    nameLabel->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    widgetLayout->addWidget(nameLabel);
    QLabel *icon = new QLabel(this);
    icon->setPixmap(QPixmap(":/image/icon/coin"));
    widgetLayout->addWidget(icon);
    m_costLabel = new QLabel(QString("%1").arg(cost, 4), this);
    widgetLayout->addWidget(m_costLabel);
    this->setLayout(widgetLayout);
}

void UnitSelectionWidget::ItemWidget::setValidity(bool valid)
{
    if (!valid)
    {
        m_costLabel->setStyleSheet("color: #b34446");
    }
    else
    {
        m_costLabel->setStyleSheet("color: #3d4265");
    }
}

