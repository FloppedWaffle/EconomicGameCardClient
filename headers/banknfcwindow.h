#ifndef BANKNFCWINDOW_H
#define BANKNFCWINDOW_H


#include "customwindow.h"



namespace Ui
{
class BankNFCWindow;
}



class BankNFCWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit BankNFCWindow(QWidget *parent = nullptr, const QString &uid = nullptr);
    ~BankNFCWindow() override;


    bool checkIsUidSame();

    void transferMoney();

    void personLineEditChanged();
    void personTransferButtonClicked();

    void payTaxesButtonClicked();
    void companyTaxButtonClicked();
    void ministerSalaryButtonClicked();

    void exitBankNFCButtonClicked();
    void refreshWindow();



private:
    Ui::BankNFCWindow *ui;
    QString m_nfcUID;
};

#endif // BANKNFCWINDOW_H
