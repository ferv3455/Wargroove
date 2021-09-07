#include "unitselectionwidget.h"
#include "ui_unitselectionwidget.h"

#include <QPushButton>

UnitSelectionWidget::UnitSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UnitSelectionWidget),
    m_listWidgetItems()
{
    ui->setupUi(this);
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
//        QListWidgetItem *item =
//            new QListWidgetItem(unitNames[i] + QString(" (%1 C)").arg(unitInfo[i][9]),
//                                m_listWidget, unitInfo[i][10]);
//        item->setIcon(QPixmap(":/image/unit/" + QString::number(i)).copy(0, 0, 64, 64));
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
}

void UnitSelectionWidget::adjustSize()
{
    int parentWidth = static_cast<QWidget *>(parent())->width();
    int parentHeight = static_cast<QWidget *>(parent())->height();
    setGeometry(parentWidth / 4, parentHeight / 4, parentWidth / 2, parentHeight / 2);
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
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_listWidget->currentItem() != nullptr);
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

