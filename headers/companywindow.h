#ifndef COMPANYWINDOW_H
#define COMPANYWINDOW_H

#include <QStandardItemModel>
#include <QKeyEvent>

#include "customwindow.h"



namespace Ui
{
    class CompanyWindow;
}



class CompanyWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit CompanyWindow(QWidget *parent = nullptr);
    ~CompanyWindow() override;

    void navigateBarButtonClicked();

    void founderLineEditChanged();
    void payFounder();

    void serviceLineEditChanged();
    void payService();

    void refreshWindow();

    void keyPressEvent(QKeyEvent *event) override;



private:
    Ui::CompanyWindow *ui;
    QStandardItemModel *m_model;

};

#endif // COMPANYWINDOW_H
