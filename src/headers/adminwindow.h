#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include "customwindow.h"

#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QStandardItemModel>

namespace Ui {
class AdminWindow;
}

class AdminWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

    void navigateBarButtonClicked();


    void registerPersonButtonClicked();
    void registerCompanyButtonClicked();
    void registerTeacherButtonClicked();


    void personLineEditChanged();
    void personListWidgetClicked();

    void addToCompanyButtonClicked();
    void removeFromCompanyButtonClicked();
    void removePersonButtonClicked();
    void removeCompanyButtonClicked();

    void companyLineEditChanged();
    void companiesListWidgetClicked();

    void serviceSearchLineEditChanged();
    void servicesListViewClicked();

    void addServiceButtonClicked();
    void removeServiceButtonClicked();




private:
    Ui::AdminWindow *ui;
    QStandardItemModel *m_model;
};

#endif // ADMINWINDOW_H
