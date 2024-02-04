#include "ui_atmwindow.h"

#include "atmwindow.h"

ATMWindow::ATMWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::ATMWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Банкомат");
    setWindowState(Qt::WindowFullScreen);

    connect(ui->personTransferLineEdit, &QLineEdit::textChanged, this, &ATMWindow::personLineEditChanged);
    connect(ui->personTransferButton, &QPushButton::clicked, this, &ATMWindow::personTransferButtonClicked);

    connect(ui->payTaxesButton, &QPushButton::clicked, this, &ATMWindow::payTaxesButtonClicked);
    connect(ui->companyTaxButton, &QPushButton::clicked, this, &ATMWindow::companyTaxButtonClicked);
    connect(ui->ministerSalaryButton, &QPushButton::clicked, this, &ATMWindow::ministerSalaryButtonClicked);


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



void ATMWindow::getPerson()
{
    if (m_nfcUID.length() == 0) return;

    QJsonObject jsonObject;
    jsonObject["uid"] = m_nfcUID;

    this->setInputsEnabled(false);
    rs->httpPost("atm/get_person", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
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

            m_nfcUID = "";
            this->setInputsEnabled(false);
            ui->personTransferLineEdit->clear();
            ui->personAmountLineEdit->clear();
            ui->atmExitLineEdit->clear();
            ui->companyTaxLineEdit->clear();
            ui->atmAccessButton->setEnabled(true);
            ui->atmExitLineEdit->setEnabled(true);
            ui->exitButton->setEnabled(true);
        }
    });
}




void ATMWindow::atmAccessButtonClicked()
{
    if (m_nfcUID.length() > 0)
    {
        m_nfcUID = "";
        ui->atmAccessButton->setIcon(QIcon(":img/img/atm-denied"));
        ui->personInfoTextBrowser->clear();

        this->setInputsEnabled(false);
        ui->personTransferLineEdit->clear();
        ui->personAmountLineEdit->clear();
        ui->atmExitLineEdit->clear();
        ui->companyTaxLineEdit->clear();
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

    m_nfcUID = nfcMgr->getCardUID();
    getPerson();
}



void ATMWindow::personLineEditChanged()
{
    if (m_nfcUID.length() == 0) return;

    QListWidget *listWidget = ui->personsListWidget;
    QString personName = ui->personTransferLineEdit->text();
    listWidget->clear();

    if (personName.split(" ").size() != 2)
    {
        return;
    }

    QStringList stringList = personName.split(" ");
    QJsonObject jsonData;
    jsonData["firstname"] = stringList[0];
    jsonData["lastname"] = stringList[1];
    jsonData["uid"] = m_nfcUID;

    rs->httpPost("atm/get_transfer_player", jsonData, [listWidget](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();
        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QJsonArray players = jsonData["players"].toArray();
            for (const QJsonValue player : players)
            {
                QString firstname = player[0].toString();
                QString lastname = player[1].toString();
                QString grade = player[2].toString();
                int playerId = player[3].toInt();

                QString playerInfo = QString("%1 %2 | Класс: %3").arg(firstname).arg(lastname).arg(grade);

                QListWidgetItem *item = new QListWidgetItem(playerInfo);
                item->setData(Qt::UserRole, playerId);
                listWidget->addItem(item);
            }
        }
    });
}



void ATMWindow::personTransferButtonClicked()
{
    if (m_nfcUID.length() == 0) return;

    QListWidgetItem *selectedItem = ui->personsListWidget->currentItem();
    if (!selectedItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не выбрали игрока! "
        "Выберите его в таблице, перед этим набрав его имя и фамилию в поле. Пример: \"Иван Иванов\" ");
        return;
    }

    int amount = ui->personAmountLineEdit->text().toInt();
    if (amount < 1)
    {
        QMessageBox::warning(this, "Предупреждение!",
        "В этом поле допустимы только натуральные положительные числа!");
        return;
    }

    QString selectedId = selectedItem->data(Qt::UserRole).toString();
    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;
    jsonData["player_id"] = selectedId;
    jsonData["amount"] = amount;

    this->setInputsEnabled(false);
    rs->httpPost("atm/transfer_player_money", jsonData, [this, amount, selectedId](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            QString("Перевод игроку с player_id %1 перечислено %2 тлц.").arg(selectedId).arg(amount));
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Недостаточно средств для перевода.");
        }
        else
        {
            QMessageBox::critical(this,
            "Ошибка!",
            "Возникла неизвестная ошибка!");
        }

        getPerson();
    });
}



void ATMWindow::payTaxesButtonClicked()
{
    if (m_nfcUID.length() == 0) return;

    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;

    this->setInputsEnabled(false);
    rs->httpPost("bankers/pay_player_taxes", jsonData, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Налоги игрока успешно уплачены.");
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Налоги игрока уже были уплачены за этот период!");
        }
        else
        {
            QMessageBox::critical(this,
            "Ошибка!",
            "Возникла неизвестная ошибка!");
        }

        getPerson();
    });
}



void ATMWindow::companyTaxButtonClicked()
{
    if (m_nfcUID.length() == 0) return;

    int taxAmount = ui->companyTaxLineEdit->text().toInt();
    if (taxAmount < 1)
    {
        QMessageBox::warning(this, "Предупреждение!",
        "В этом поле допустимы только натуральные положительные числа!");
        return;
    }

    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;
    jsonData["tax_amount"] = taxAmount;

    this->setInputsEnabled(false);
    rs->httpPost("atm/pay_company_taxes", jsonData, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
        QString errorReason = jsonData["error"].toString();
        int taxAmount = jsonData["tax_amount"].toInt();

        if (error == QNetworkReply::NoError)
        {
            if (!taxAmount)
            {
                QMessageBox::information(this,
                "Предупреждение!",
                "Налог уже был уплачен ранее! Никаких задолжностей у фирмы больше нет.");
            }
            else {
                QMessageBox::information(this,
                "Успешно!",
                QString("Налог фирмы был уплачен суммой в %1 тлц.").arg(taxAmount));
            }
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            if (errorReason == "founder_not_found")
            {
                QMessageBox::critical(this,
                "Ошибка 404 (ContentNotFoundError)",
                "К сожалению, владелец фирмы не был найден.");
            }
            else if (errorReason == "company_not_found")
            {
                QMessageBox::critical(this,
                "Ошибка 404 (ContentNotFoundError)",
                "К сожалению, такая фирма не была найдена.");
            }
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Недостаточно средств для уплаты налога!");
        }
        else
        {
            QMessageBox::critical(this,
            "Ошибка!",
            "Возникла неизвестная ошибка!");
        }

        getPerson();
    });
}



void ATMWindow::ministerSalaryButtonClicked()
{
    if (m_nfcUID.length() == 0) return;

    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;

    this->setInputsEnabled(false);
    rs->httpPost("atm/pay_minister_salary", jsonData, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Зарплата министра была выплачена.");
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Зарплата уже была выплачена этому министру.");
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(this,
            "Ошибка 404 (ContentNotFoundError)",
            "К сожалению, министр не был найден.");
        }
        else
        {
            QMessageBox::critical(this,
            "Ошибка!",
            "Возникла неизвестная ошибка!");
        }

        getPerson();
    });
}



void ATMWindow::exitATMWindowClicked()
{
    QString code = ui->atmExitLineEdit->text();
    if (code.size() != 10)
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
            getPerson();
        }
    });
}
