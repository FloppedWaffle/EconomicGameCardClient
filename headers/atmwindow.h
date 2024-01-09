#ifndef ATMWINDOW_H
#define ATMWINDOW_H

#include "customwindow.h"

namespace Ui
{
    class ATMWindow;
}



class ATMWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit ATMWindow(QWidget *parent = nullptr);
    ~ATMWindow() override;

    void checkIsUidSame();

    void atmAccessButtonClicked();

    void personLineEditChanged();
    void personTransferButtonClicked();

    void payTaxesButtonClicked();
    void companyTaxButtonClicked();
    void ministerSalaryButtonClicked();

    void exitATMWindowClicked();



private:
    Ui::ATMWindow *ui;
    QString m_nfcUID;
};

#endif // ATMWINDOW_H
