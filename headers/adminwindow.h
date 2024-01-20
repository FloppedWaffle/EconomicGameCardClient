#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include "customwindow.h"

#include <QRandomGenerator>
#include <QCryptographicHash>

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


    void personLineEditChanged();
    void personListWidgetClicked();

    void addToCompanyButtonClicked();
    void removeFromCompanyButtonClicked();
    void removePersonButtonClicked();
    void removeCompanyButtonClicked();

    void companyLineEditChanged();
    void companiesListWidgetClicked();

    void addServiceButtonClicked();




private:
    Ui::AdminWindow *ui;
};

#endif // ADMINWINDOW_H
