#ifndef SERVICEPAYWINDOW_H
#define SERVICEPAYWINDOW_H



#include <QStandardItemModel>
#include <QKeyEvent>

#include "customwindow.h"
#include "nfcmanager.h"



namespace Ui
{
    class ServicePayWindow;
}



class ServicePayWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit ServicePayWindow(QWidget *parent = nullptr, QStandardItemModel *model = nullptr);
    ~ServicePayWindow() override;

    void setAllCost(const QStandardItemModel *model);

    void servicesListViewClicked();
    void servicePlusButtonClicked();
    void serviceMinusButtonClicked();

    void servicePayButtonClicked();

    void exitServicePayButtonClicked();

    void keyPressEvent(QKeyEvent *event) override;



private:
    Ui::ServicePayWindow *ui;
    QStandardItemModel *m_model;
};

#endif // SERVICEPAYWINDOW_H
