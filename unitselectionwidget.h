#ifndef UNITSELECTIONWIDGET_H
#define UNITSELECTIONWIDGET_H

#include "descriptionwidget.h"

#include <QWidget>
#include <QLabel>
#include <QListWidget>

namespace Ui
{
class UnitSelectionWidget;
}

class UnitSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UnitSelectionWidget(GameInfo *gameInfo, QWidget *parent = nullptr);
    ~UnitSelectionWidget();

    // The widget used in each list widget item
    class ItemWidget : public QWidget
    {
    public:
        explicit ItemWidget(const QString &name, int cost,
                            const QPixmap &image, QWidget *parent = nullptr);
    public:
        int m_cost;
        QLabel *m_costLabel;
        void setValidity(bool valid);
    };

    // Load all units
    void loadUnits(const QStringList &unitNames, float **unitInfo);

    // Show the widget to select from units of the given kind
    void showUnits(int unitKind, int coins);

public slots:
    void selected();
    void updateButtonValidity();
    void adjustSize();

private:
    Ui::UnitSelectionWidget *ui;
    DescriptionWidget *m_descriptionWidget;
    QListWidget *m_listWidget;
    QList<QListWidgetItem *> m_listWidgetItems;
    GameInfo *m_gameInfo;

    int m_nKind;

signals:
    void confirm(int);
    void cancel();
};

#endif // UNITSELECTIONWIDGET_H
