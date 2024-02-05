#include "adminwindow.h"
#include "ui_adminwindow.h"

AdminWindow::AdminWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::AdminWindow),
    m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Админ-панель");

    ui->stackedWidget->setCurrentWidget(ui->registerPage);

    connect(ui->navRegisterButton, &QPushButton::clicked, this, &AdminWindow::navigateBarButtonClicked);
    connect(ui->navChangeButton, &QPushButton::clicked, this, &AdminWindow::navigateBarButtonClicked);


    connect(ui->registerPersonButton, &QPushButton::clicked, this, &AdminWindow::registerPersonButtonClicked);
    connect(ui->registerCompanyButton, &QPushButton::clicked, this, &AdminWindow::registerCompanyButtonClicked);
    connect(ui->registerTeacherButton, &QPushButton::clicked, this, &AdminWindow::registerTeacherButtonClicked);


    connect(ui->personLineEdit, &QLineEdit::textChanged, this, &AdminWindow::personLineEditChanged);
    connect(ui->personsListWidget, &QListWidget::clicked, this, &AdminWindow::personListWidgetClicked);


    connect(ui->companyLineEdit, &QLineEdit::textChanged, this, &AdminWindow::companyLineEditChanged);
    connect(ui->companiesListWidget, &QListWidget::clicked, this, &AdminWindow::companiesListWidgetClicked);


    connect(ui->addToCompanyButton, &QPushButton::clicked, this, &AdminWindow::addToCompanyButtonClicked);
    connect(ui->removeFromCompanyButton, &QPushButton::clicked, this, &AdminWindow::removeFromCompanyButtonClicked);
    connect(ui->removePersonButton, &QPushButton::clicked, this, &AdminWindow::removePersonButtonClicked);
    connect(ui->removeCompanyButton, &QPushButton::clicked, this, &AdminWindow::removeCompanyButtonClicked);


    connect(ui->serviceSearchLineEdit, &QLineEdit::textChanged, this, &AdminWindow::serviceSearchLineEditChanged);
    connect(ui->servicesListView, &QListView::clicked, this, &AdminWindow::servicesListViewClicked);

    connect(ui->addServiceButton, &QPushButton::clicked, this, &AdminWindow::addServiceButtonClicked);
    connect(ui->removeServiceButton, &QPushButton::clicked, this, &AdminWindow::removeServiceButtonClicked);


    connect(ui->exitButton, &QPushButton::clicked, this, &CustomWindow::closeWindowsOpenAuth);
}

AdminWindow::~AdminWindow()
{
    delete ui;
}



void AdminWindow::navigateBarButtonClicked()
{
    QString senderButtonName = qobject_cast<QPushButton*>(sender())->objectName();

    ui->navRegisterButton->setStyleSheet("border-bottom: none;");
    ui->navChangeButton->setStyleSheet("border-bottom: none;");

    QString styleString = "border-bottom: 3px solid black; border-left: 3px solid black; border-right: 3px solid black;";
    if (senderButtonName == "navRegisterButton")
    {
        ui->navRegisterButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->registerPage);
    }
    else if (senderButtonName == "navChangeButton")
    {
        ui->navChangeButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->changePage);
    }
}



void AdminWindow::registerPersonButtonClicked()
{
    QString firstname = ui->personFirstnameLineEdit->text();
    QString lastname = ui->lastnameLineEdit->text();
    QString grade = ui->gradeLineEdit->text();
    bool isMinister = ui->isMinisterCheckBox->checkState();

    if (!firstname.length() || !lastname.length() || !grade.length() || !nfcMgr->isCardAttached())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Некоторые поля выше не были заполнены или карта не была приложена к считывателю!");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["firstname"] = firstname.trimmed();
    jsonObject["lastname"] = lastname.trimmed();
    jsonObject["grade"] = grade.trimmed();
    jsonObject["is_minister"] = isMinister;
    jsonObject["uid"] = nfcMgr->getCardUID();


    this->setInputsEnabled(false);
    rs->httpPost("admin/register_person", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Игрок был успешно зарегистрирован!");
            ui->personFirstnameLineEdit->clear();
            ui->lastnameLineEdit->clear();
            ui->gradeLineEdit->clear();
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Игрок не был зарегистрирован, т.к. человек с такими именем и фамилией или его карта уже присутствуют в базе данных.");
        }
    });
}



void AdminWindow::registerCompanyButtonClicked()
{
    QString companyName = ui->companyNameLineEdit->text();

    if (!companyName.length())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Поле с именем фирмы не было заполнено!");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["company_name"] = companyName.trimmed();

    const QString possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString randomPassword;
    for (int i = 0; i < 10; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        randomPassword.append(possibleCharacters.at(index));
    }
    QString hashedCode = QCryptographicHash::hash(randomPassword.toUtf8(), QCryptographicHash::Sha256).toHex();
    jsonObject["password"] = hashedCode;


    this->setInputsEnabled(false);
    rs->httpPost("admin/register_company", jsonObject, [this, randomPassword](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            QString("Фирма была успешно зарегистрирована! Код фирмы: %1").arg(randomPassword));
            ui->companyNameLineEdit->clear();
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Фирма не была зарегистрирована, т.к. фирма с таким именем или паролем уже присутствует в базе данных.");
        }
    });
}



void AdminWindow::registerTeacherButtonClicked()
{
    QString firstname = ui->teacherFirstnameLineEdit->text();
    QString middlename = ui->middlenameLineEdit->text();
    QString subjectName = ui->subjectNameLineEdit->text();

    if (!firstname.length() || !middlename.length() || !subjectName.length() || !nfcMgr->isCardAttached())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Некоторые поля выше не были заполнены или карта не была приложена к считывателю!");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["firstname"] = firstname.trimmed();
    jsonObject["middlename"] = middlename.trimmed();
    jsonObject["subject_name"] = subjectName.trimmed();
    jsonObject["uid"] = nfcMgr->getCardUID();

    const QString possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString randomPassword;
    for (int i = 0; i < 10; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        randomPassword.append(possibleCharacters.at(index));
    }
    QString hashedCode = QCryptographicHash::hash(randomPassword.toUtf8(), QCryptographicHash::Sha256).toHex();
    jsonObject["password"] = hashedCode;


    this->setInputsEnabled(false);
    rs->httpPost("admin/register_teacher", jsonObject, [this, randomPassword](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            QString("Учитель был успешно зарегистрирован! Код учителя: %1").arg(randomPassword));
            ui->teacherFirstnameLineEdit->clear();
            ui->middlenameLineEdit->clear();
            ui->subjectNameLineEdit->clear();
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Учитель не был зарегистрирован, т.к. человек с такими именем и фамилией или его карта уже присутствуют в базе данных.");
        }
    });
}



void AdminWindow::personLineEditChanged()
{
    QListWidget *listWidget = ui->personsListWidget;
    QString personName = ui->personLineEdit->text();

    listWidget->clear();
    ui->personBalanceLabel->setText("Баланс игрока:");

    if (personName.split(" ").size() != 2)
    {
        return;
    }

    QStringList stringList = personName.split(" ");
    QJsonObject jsonObject;
    jsonObject["firstname"] = stringList[0];
    jsonObject["lastname"] = stringList[1];


    rs->httpPost("admin/get_persons", jsonObject, [listWidget](QNetworkReply *reply)
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
                bool isMinister = player[4].toInt();
                qDebug() << player[4];

                QString playerInfo = QString("%1 %2 | Класс: %3 | Роль: %4")
                .arg(firstname)
                .arg(lastname)
                .arg(grade)
                .arg(isMinister ? "госслужащий" : "обычный игрок");

                QListWidgetItem *item = new QListWidgetItem(playerInfo);
                item->setData(Qt::UserRole, playerId);
                listWidget->addItem(item);
            }
        }
    });
}



void AdminWindow::personListWidgetClicked()
{
    QListWidgetItem *playerItem = ui->personsListWidget->currentItem();
    QJsonObject jsonObject;
    QString selectedId = playerItem->data(Qt::UserRole).toString();
    jsonObject["player_id"] = selectedId;


    this->setInputsEnabled(false);
    rs->httpPost("admin/get_person_balance", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QString balanceText = "Баланс игрока: " + QString::number(jsonData["balance"].toInt()) + " тлц";
            ui->personBalanceLabel->setText(balanceText);
        }
    });
}



void AdminWindow::companyLineEditChanged()
{
    QListWidget *listWidget = ui->companiesListWidget;
    listWidget->clear();
    m_model->clear();

    QString companyName = ui->companyLineEdit->text().trimmed();
    QJsonObject jsonObject;
    jsonObject["company_name"] = companyName;


    rs->httpPost("admin/get_companies", jsonObject, [listWidget](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();

        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QJsonArray companies = jsonData["companies"].toArray();
            for (const QJsonValue company : companies)
            {
                QString companyName = company[0].toString();
                int companyId = company[1].toInt();

                QListWidgetItem *item = new QListWidgetItem(companyName);
                item->setData(Qt::UserRole, companyId);
                listWidget->addItem(item);
            }
        }
    });
}



void AdminWindow::companiesListWidgetClicked()
{
    QListWidgetItem *companyItem = ui->companiesListWidget->currentItem();
    QJsonObject jsonObject;
    QString selectedId = companyItem->data(Qt::UserRole).toString();
    jsonObject["company_id"] = selectedId;


    this->setInputsEnabled(false);
    rs->httpPost("admin/get_company_info", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QString balanceText = "Баланс фирмы: " + QString::number(jsonData["balance"].toInt()) + " тлц";
            ui->companyBalanceLabel->setText(balanceText);


            m_model->clear();
            QJsonArray services = jsonData["services"].toArray();
            for (const QJsonValue service : services)
            {
                int serviceId = service[0].toInt();
                QString serviceName = service[1].toString();
                int serviceCost = service[2].toInt();
                int serviceQuantity = service[3].toInt();

                QString serviceInfo = QString("%1 | %2 тлц | %3 шт осталось").arg(serviceName).arg(serviceCost).arg(serviceQuantity);

                QStandardItem *item = new QStandardItem(serviceInfo);
                item->setData(serviceId, Qt::UserRole);
                item->setData(serviceCost, Qt::UserRole + 1);
                item->setCheckable(true);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setCheckState(Qt::Unchecked);

                m_model->appendRow(item);
            }

            QListView *listView = ui->servicesListView;

            listView->setModel(m_model);
            listView->setSelectionMode(QAbstractItemView::NoSelection);
        }
    });
}



void AdminWindow::addToCompanyButtonClicked()
{
    QListWidgetItem *playerItem = ui->personsListWidget->currentItem();
    QListWidgetItem *companyItem = ui->companiesListWidget->currentItem();

    if (!playerItem || !companyItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не указали что-то из двух первых столбцов. Для того, чтобы добавить человека в фирму, "
        "нужно указать его в первом столбце и фирму в правом.");
        return;
    }


    QJsonObject jsonObject;
    jsonObject["player_id"] = playerItem->data(Qt::UserRole).toInt();
    jsonObject["company_id"] = companyItem->data(Qt::UserRole).toInt();
    jsonObject["is_founder"] = ui->isFounderCheckBox->checkState();


    this->setInputsEnabled(false);
    rs->httpPost("admin/add_person_to_company", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Игрок был успешно добавлен в фирму.");
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Игрок не был добавлен в фирму, так как уже состоит в этой или другой фирме или является госслужащим.");
        }
    });
}



void AdminWindow::removeFromCompanyButtonClicked()
{
    QListWidgetItem *playerItem = ui->personsListWidget->currentItem();

    if (!playerItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не указали человека в первом столбце. Для того, чтобы удалить человека из фирмы, "
        "нужно указать его в первом столбце.");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["player_id"] = playerItem->data(Qt::UserRole).toInt();


    this->setInputsEnabled(false);
    rs->httpPost("admin/remove_person_from_company", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Игрок был успешно удалён из фирмы.");
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Игрок не был удалён из фирмы, так как изначально он не состоял ни в какой фирме.");
        }
    });
}



void AdminWindow::removePersonButtonClicked()
{
    QListWidgetItem *playerItem = ui->personsListWidget->currentItem();

    if (!playerItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не указали человека в первом столбце. Для того, чтобы удалить человека, "
        "нужно указать его в первом столбце.");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["player_id"] = playerItem->data(Qt::UserRole).toInt();


    this->setInputsEnabled(false);
    rs->httpPost("admin/remove_person", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Игрок был успешно удалён.");
            this->personLineEditChanged();
        }
    });
}



void AdminWindow::removeCompanyButtonClicked()
{
    QListWidgetItem *companyItem = ui->companiesListWidget->currentItem();

    if (!companyItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не указали фирму во втором столбце. Для того, чтобы удалить фирму, "
        "нужно указать её во втором столбце.");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["company_id"] = companyItem->data(Qt::UserRole).toInt();


    this->setInputsEnabled(false);
    rs->httpPost("admin/remove_company", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Фирма была успешно удалёна.");
            this->companyLineEditChanged();
        }
    });
}



void AdminWindow::serviceSearchLineEditChanged()
{
    QString lineEditText = ui->serviceSearchLineEdit->text();
    QListView *listView = ui->servicesListView;

    for (int row = 0; row < m_model->rowCount(); row++)
    {
        QStandardItem *item = m_model->item(row);
        QString itemText = item->text();

        if (itemText.startsWith(lineEditText) || lineEditText == "")
        {
            listView->setRowHidden(row, false);
        }
        else
        {
            listView->setRowHidden(row, true);
        }
    }
}



void AdminWindow::servicesListViewClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->servicesListView->currentIndex());
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}



void AdminWindow::addServiceButtonClicked()
{
    QListWidgetItem *companyItem = ui->companiesListWidget->currentItem();
    QString serviceName = ui->serviceNameLineEdit->text();
    QString serviceCost = ui->serviceCostLineEdit->text();
    QString serviceQuantity = ui->serviceQuantityLineEdit->text();

    if (!companyItem || !serviceName.length() || !serviceCost.length() || !serviceQuantity.length())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Некоторые поля выше не были заполнены или не была выбрана фирма из второго столбца!");
        return;
    }

    QJsonObject jsonObject;
    jsonObject["company_id"] = companyItem->data(Qt::UserRole).toInt();
    jsonObject["service_name"] = serviceName;
    jsonObject["service_cost"] = serviceCost;
    jsonObject["service_quantity"] = serviceQuantity;


    this->setInputsEnabled(false);
    rs->httpPost("admin/add_company_service", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Товар/услуга добавлен/добавлена.");
            this->companiesListWidgetClicked();
        }
    });
}



void AdminWindow::removeServiceButtonClicked()
{
    QListWidgetItem *companyItem = ui->companiesListWidget->currentItem();

    if (!companyItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не выбрали фирму из второго столбца!");
        return;
    }

    QJsonObject jsonObject;
    QJsonArray jsonArray;
    for (int row = 0; row < m_model->rowCount(); row++)
    {
        QStandardItem *item = m_model->item(row);
        if (item->checkState() == Qt::Checked)
        {
            int serviceId = item->data(Qt::UserRole).toInt();
            jsonArray.append(serviceId);
        }
    }
    if (!jsonArray.size())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не выбрали ни одного/одной товара/услуги из третьего столбца!");
        return;
    }

    jsonObject["player_id"] = companyItem->data(Qt::UserRole).toInt();
    jsonObject["services"] = jsonArray;


    this->setInputsEnabled(false);
    rs->httpPost("admin/remove_company_service", jsonObject, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (this->commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Выбранные товары/услуги были удалены.");
            this->companiesListWidgetClicked();
        }
    });
}
