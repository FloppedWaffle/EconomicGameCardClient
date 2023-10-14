#include "banknfcwindow.h"
#include "ui_banknfcwindow.h"


BankNFCWindow::BankNFCWindow(QWidget *parent, const QString &uid) :
    CustomWindow(parent),
    ui(new Ui::BankNFCWindow),
    m_nfcUID(uid)
{
    ui->setupUi(this);
    this->setWindowTitle("Работа с картой");


    connect(ui->depositButton, &QPushButton::clicked, this, &BankNFCWindow::transferMoney);
    connect(ui->withdrawButton, &QPushButton::clicked, this, &BankNFCWindow::transferMoney);

    connect(ui->personTransferLineEdit, &QLineEdit::textChanged, this, &BankNFCWindow::personLineEditChanged);
    connect(ui->personTransferButton, &QPushButton::clicked, this, &BankNFCWindow::personTransferButtonClicked);

    connect(ui->payTaxesButton, &QPushButton::clicked, this, &BankNFCWindow::payTaxesButtonClicked);
    connect(ui->companyTaxButton, &QPushButton::clicked, this, &BankNFCWindow::companyTaxButtonClicked);
    connect(ui->ministerSalaryButton, &QPushButton::clicked, this, &BankNFCWindow::ministerSalaryButtonClicked);


    connect(ui->refreshButton, &QPushButton::clicked, this, &BankNFCWindow::refreshWindow);
    connect(ui->exitButton, &QPushButton::clicked, this, &BankNFCWindow::exitBankNFCButtonClicked);


    qDebug() << "NFC is: " + m_nfcUID;


    refreshWindow();
}

BankNFCWindow::~BankNFCWindow()
{
    delete ui;
}



void BankNFCWindow::checkIsUidSame()
{
    if (!nfcMgr->isCardAttached())
    {
        QMessageBox::critical(this,
        "Ошибка!",
        "Карта не приложена к считывателю! "
        "Карта должна быть всегда приложена к считывателю до окончания работы с ней.");
        this->close();
        this->deleteLater();
        return;
    }

    if (nfcMgr->getCardUID() != m_nfcUID)
    {
        QMessageBox::critical(this,
        "Ошибка!",
        "Несоответствующая карта! "
        "Вы приложили другую карту вместо той, чтобы изначально.");
        this->close();
        this->deleteLater();
        return;
    }
}



void BankNFCWindow::transferMoney()
{
    checkIsUidSame();

    int transferAmount = ui->transferAmountLineEdit->text().toInt();
    if (transferAmount < 1)
    {
        QMessageBox::warning(this, "Предупреждение!",
        "В этом поле допустимы только натуральные положительные числа!");
        return;
    }

    QString senderButtonName = qobject_cast<QPushButton*>(sender())->objectName();
    QString transferAction = senderButtonName == "depositButton" ? "deposit" : "withdraw";

    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;
    jsonData["amount"] = transferAmount;
    jsonData["transfer_action"] = transferAction;

    this->setInputsEnabled(false);

    rs->httpPost("banker/transfer_money", jsonData, [this, transferAction, transferAmount](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;


        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            QString("%1 средств в размере %2 тлц было выполнено.")
            .arg(transferAction == "deposit" ? "Зачисление" : "Вывод").arg(transferAmount));
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Недостаточно талиц для вывода! "
            "Введите число меньше.");
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(nullptr,
            "Ошибка 404 (ContentNotFoundError)",
            "Инфомарция о таком человеке не была найдена! ");
            this->close();
            this->deleteLater();
        }
        else
        {
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Подробности в названии окна ошибки.");
            this->close();
            this->deleteLater();
        }

        refreshWindow();
    });
}



void BankNFCWindow::personLineEditChanged()
{
    QListWidget *listWidget = ui->personsListWidget;
    QString personName = ui->personTransferLineEdit->text();

    listWidget->clear();

    if (personName.split(" ").size() != 2)
    {
        return;
    }

    QStringList stringList = personName.split(" ");
    QJsonObject jsonObject;
    jsonObject["firstname"] = stringList[0];
    jsonObject["lastname"] = stringList[1];

    rs->httpPost("banker/get_transfer_player", jsonObject, [listWidget](QNetworkReply *reply)
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



void BankNFCWindow::personTransferButtonClicked()
{
    checkIsUidSame();

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

    rs->httpPost("banker/transfer_player_money", jsonData, [this, amount, selectedId](QNetworkReply *reply)
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
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Подробности в названии окна ошибки.");
            this->close();
            this->deleteLater();
        }

        refreshWindow();
    });
}



void BankNFCWindow::payTaxesButtonClicked()
{
    checkIsUidSame();

    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;
    jsonData["is_card"] = ui->cardRadio->isChecked();

    this->setInputsEnabled(false);

    rs->httpPost("banker/pay_player_taxes", jsonData, [this](QNetworkReply *reply)
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
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Подробности в названии окна ошибки.");
            this->close();
            this->deleteLater();
        }

        refreshWindow();
    });
}



void BankNFCWindow::companyTaxButtonClicked()
{
    checkIsUidSame();

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
    jsonData["is_card"] = ui->cardRadio->isChecked();

    this->setInputsEnabled(false);

    rs->httpPost("banker/pay_company_taxes", jsonData, [this](QNetworkReply *reply)
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

        refreshWindow();
    });
}



void BankNFCWindow::ministerSalaryButtonClicked()
{
    checkIsUidSame();

    QJsonObject jsonData;
    jsonData["uid"] = m_nfcUID;
    jsonData["is_card"] = ui->cardRadio->isChecked();

    this->setInputsEnabled(false);

    rs->httpPost("banker/pay_minister_salary", jsonData, [this](QNetworkReply *reply)
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

        refreshWindow();
    });
}



void BankNFCWindow::refreshWindow()
{
    checkIsUidSame();

    QJsonObject jsonObject;
    jsonObject["uid"] = m_nfcUID;

    this->setInputsEnabled(false);

    rs->httpPost("banker/get_person", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
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
                    personInfo = personInfo +
                    "Является владельцем фирмы" + "\n";
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

                ui->cardRadio->setDisabled(true);
                ui->cashRadio->setDisabled(true);
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



void BankNFCWindow::exitBankNFCButtonClicked()
{
    this->close();
    this->deleteLater();
}

