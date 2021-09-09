#ifndef DESCRIPTIONWIDGET_H
#define DESCRIPTIONWIDGET_H

#include "block.h"
#include "gameinfo.h"

#include <QWidget>
#include <QLabel>
#include <QTimer>

namespace Ui
{
class DescriptionWidget;
}

class DescriptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DescriptionWidget(GameInfo *gameInfo, QWidget *parent = nullptr, bool selectUnits = false);
    ~DescriptionWidget();

    void setUnit(const Unit *unit, int terrain);
    void setBlock(const Block *block);

    void adjustBackground();
    void updateScreen();

    void closeWindow();

public slots:
    void updateUnitPicture();

private:
    Ui::DescriptionWidget *ui;
    GameInfo *m_gameInfo;
    Block *m_displayingBlock;

    bool m_bSelectUnits;

    QTimer *m_refreshTimer;

    QLabel ***m_interunitLabels;
    QLabel *m_unitPictureLabel;
    QImage *m_unitImages;
    int m_nDynamicsId;
};

#endif // DESCRIPTIONWIDGET_H
