#include "bankerwindow.h"
#include "ui_bankerwindow.h"

#include "banknfcwindow.h"


BankerWindow::BankerWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::BankerWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Банк");


    connect(ui->exitButton, &QPushButton::clicked, this, &CustomWindow::closeWindowsOpenAuth);
    connect(ui->nfcReaderButton, &QPushButton::clicked, this, &BankerWindow::nfcReaderButtonClicked);


    this->setInputsEnabled(false);
    rs->httpGet("bankers", [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(this,
            "Ошибка 404 (ContentNotFoundError)",
            "Инфомарция о таком банкире не была найдена! "
            "Возможно, ваш токен устарел. Попробуйте войти ещё раз.");
            this->closeWindowsOpenAuth();
        }
    });
}

BankerWindow::~BankerWindow()
{
    delete ui;
}




void BankerWindow::nfcReaderButtonClicked()
{
    QList<BankNFCWindow *> widgetList = this->findChildren<BankNFCWindow *>();
    for (QWidget *widget : widgetList)
    {
        widget->close();
        widget->deleteLater();
    }


    if (!nfcMgr->isCardAttached())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Карта не приложена к считывателю!");
        return;
    }

    QString uid = nfcMgr->getCardUID();
    QJsonObject jsonObject;
    jsonObject["uid"] = uid;

    this->setInputsEnabled(false);
    rs->httpPost("bankers/get_person", jsonObject, [this, uid](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            BankNFCWindow *nfcWindow = new BankNFCWindow(this, uid);
            nfcWindow->show();
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(nullptr,
            "Ошибка 404 (ContentNotFoundError)",
            "Инфомарция о таком человеке не была найдена! ");
        }
        else
        {
            QMessageBox::critical(this,
            "Ошибка!",
            "Возникла неизвестная ошибка!");
        }
    });
}




