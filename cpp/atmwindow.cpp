#include "ui_atmwindow.h"

#include "atmwindow.h"

ATMWindow::ATMWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::ATMWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Банкомат");

//    connect(ui->personTransferLineEdit, &QLineEdit::textChanged, this, &ATMWindow::personLineEditChanged);
//    connect(ui->personTransferButton, &QPushButton::clicked, this, &ATMWindow::personTransferButtonClicked);

//    connect(ui->payTaxesButton, &QPushButton::clicked, this, &ATMWindow::payTaxesButtonClicked);
//    connect(ui->companyTaxButton, &QPushButton::clicked, this, &ATMWindow::companyTaxButtonClicked);
//    connect(ui->ministerSalaryButton, &QPushButton::clicked, this, &ATMWindow::ministerSalaryButtonClicked);


    connect(ui->exitButton, &QPushButton::clicked, this, &ATMWindow::exitATMWindowClicked);
    connect(ui->atmAccessButton, &QToolButton::clicked, this, &ATMWindow::atmAccessButtonClicked);


    this->setInputsEnabled(false);
    rs->httpGet("atm", [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            this->setInputsEnabled(false);
            ui->atmAccessButton->setEnabled(true);
            ui->atmExitLineEdit->setEnabled(true);
            ui->exitButton->setEnabled(true);
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(this,
            "Ошибка 404 (ContentNotFoundError)",
            "Инфомарция о таком терминале не была найдена! "
            "Возможно, ваш токен устарел. Попробуйте войти ещё раз.");
            this->closeWindowsOpenAuth();
        }
    });
}

ATMWindow::~ATMWindow()
{
    delete ui;
}


void ATMWindow::atmAccessButtonClicked()
{
    if (m_nfcUID.length() > 0)
    {
        m_nfcUID = "";
        ui->atmAccessButton->setIcon(QIcon(":img/img/atm-denied"));
        ui->personInfoTextBrowser->clear();

        this->setInputsEnabled(false);
        ui->atmAccessButton->setEnabled(true);
        ui->atmExitLineEdit->setEnabled(true);
        ui->exitButton->setEnabled(true);

        return;
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
    rs->httpPost("atm/get_person", jsonObject, [this, uid](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            m_nfcUID = uid;
            ui->atmAccessButton->setIcon(QIcon(":img/img/atm-success"));

            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());

            QTextBrowser *textBrowser = ui->personInfoTextBrowser;
            QString role = jsonData["role"].toString();
            QJsonArray jsonPerson = jsonData["person"].toArray();

            textBrowser->clear();

            QString personInfo;
            if (role == "players")
            {
                QString firstname = jsonPerson[0].toString();
                QString lastname = jsonPerson[1].toString();
                QString grade = jsonPerson[2].toString();
                int balance = jsonPerson[3].toInt();
                bool taxPaid = bool(jsonPerson[4].toInt());
                bool isFounder = bool(jsonPerson[5].toInt());
                bool isMinister = bool(jsonPerson[6].toInt());
                bool isMinisterPaid = bool(jsonPerson[7].toInt());

                personInfo =
                "Имя и фамилия: " + firstname + " " + lastname + "\n"
                "Класс: " + grade + "\n"
                "Баланс: " + QString::number(balance) + " тлц" + "\n"
                "Налоги: " + (taxPaid ? "уплачены" : "неуплачены") + "\n";

                if (isFounder)
                {
                    int companyTaxes = jsonData["company_taxes"].toInt();

                    personInfo = personInfo +
                    "Является владельцем фирмы" + "\n"
                    "Налоги фирмы: " + QString::number(companyTaxes) + " тлц";
                    ui->payTaxesButton->setDisabled(true);
                    ui->ministerSalaryButton->setDisabled(true);
                }
                else if (isMinister)
                {
                    personInfo = personInfo +
                    "Является министром" + "\n"
                    "Зарплата " + (isMinisterPaid ? "выплачена" : "не выплачена") + "\n";
                    ui->payTaxesButton->setDisabled(true);
                    ui->companyTaxLineEdit->setDisabled(true);
                    ui->companyTaxButton->setDisabled(true);
                }
                else
                {
                    ui->companyTaxLineEdit->setDisabled(true);
                    ui->companyTaxButton->setDisabled(true);
                    ui->ministerSalaryButton->setDisabled(true);
                }
            }
            else if (role == "teachers")
            {
                QString firstname = jsonPerson[0].toString();
                QString middlename = jsonPerson[1].toString();
                int balance = jsonPerson[2].toInt();

                personInfo =
                "Имя и Отчество: " + firstname + " " + middlename + "\n"
                "Баланс: " + QString::number(balance) + " тлц" + "\n";

                ui->personTransferLineEdit->setDisabled(true);
                ui->personsListWidget->setDisabled(true);
                ui->personAmountLineEdit->setDisabled(true);
                ui->personTransferButton->setDisabled(true);
                ui->payTaxesButton->setDisabled(true);
                ui->companyTaxLineEdit->setDisabled(true);
                ui->companyTaxButton->setDisabled(true);
                ui->ministerSalaryButton->setDisabled(true);
            }

            textBrowser->setPlainText(personInfo);
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(nullptr,
            "Ошибка 404 (ContentNotFoundError)",
            "Инфомарция о таком человеке не была найдена! ");
            this->close();
            this->deleteLater();
        }
    });
}


void ATMWindow::exitATMWindowClicked()
{
    QString code = ui->atmExitLineEdit->text();
    if (code.size() < 10)
    {
        return;
    }

    QString hashedCode = QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex();
    QJsonObject jsonObject;
    jsonObject["exit_password"] = hashedCode;

    rs->httpPost("atm/exit", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            this->closeWindowsOpenAuth();
        }
        else
        {
            this->setInputsEnabled(false);
            ui->atmAccessButton->setEnabled(true);
            ui->atmExitLineEdit->setEnabled(true);
            ui->exitButton->setEnabled(true);
        }
    });
}
