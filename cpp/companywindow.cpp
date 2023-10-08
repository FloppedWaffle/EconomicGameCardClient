#include "companywindow.h"
#include "servicepaywindow.h"
#include "ui_companywindow.h"

CompanyWindow::CompanyWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::CompanyWindow),
    m_model(new QStandardItemModel(this))
{
    //TODO: надо доделать ошибки у запросов (посмотреть, какие ошибки могут прилетать с бэка и дописать тут окна ошибок.

    ui->setupUi(this);
    this->setWindowTitle("Фирма");

    connect(ui->founderLineEdit, &QLineEdit::textChanged, this, &CompanyWindow::founderLineEditChanged);
    connect(ui->founderWithdrawButton, &QPushButton::clicked, this, &CompanyWindow::payFounder);

    connect(ui->serviceLineEdit, &QLineEdit::textChanged, this, &CompanyWindow::serviceLineEditChanged);
    connect(ui->payServiceButton, &QPushButton::clicked, this, &CompanyWindow::payService);

    connect(ui->exitButton, &QPushButton::clicked, this, &CustomWindow::closeWindowsOpenAuth);
    connect(ui->refreshButton, &QPushButton::clicked, this, &CompanyWindow::refreshWindow);


    refreshWindow();
}

CompanyWindow::~CompanyWindow()
{
    delete ui;
}



void CompanyWindow::founderLineEditChanged()
{
    QListWidget *listWidget = ui->foundersListWidget;
    QLineEdit *lineEdit = ui->founderLineEdit;

    listWidget->clear();

    if (lineEdit->text() == "" || lineEdit->text().split(" ").size() != 2)
    {
        return;
    }

    QStringList stringList = lineEdit->text().split(" ");
    QJsonObject jsonData;
    jsonData["firstname"] = stringList[0];
    jsonData["lastname"] = stringList[1];

    rs->httpPost("company/get_founders", jsonData, [listWidget](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();
        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QJsonArray founders = jsonData["founders"].toArray();
            for (const QJsonValue founder : founders)
            {
                QString firstname = founder[0].toString();
                QString lastname = founder[1].toString();
                int player_id = founder[2].toInt();

                QString player_info = QString("%1 %2").arg(firstname).arg(lastname);

                QListWidgetItem *item = new QListWidgetItem(player_info);
                item->setData(Qt::UserRole, player_id);
                listWidget->addItem(item);
            }
        }
    });
}



void CompanyWindow::payFounder()
{
    QListWidgetItem *selectedItem = ui->foundersListWidget->currentItem();

    if (!selectedItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не выбрали владельца! "
        "Выберите его в таблице слева, перед этим набрав его имя и фамилию. Пример: \"Иван Иванов\" ");
        return;
    }

    QString selectedId = ui->foundersListWidget->currentItem()->data(Qt::UserRole).toString();


    int withdraw = ui->founderWithdrawLineEdit->text().toInt();
    if (withdraw < 1)
    {
        QMessageBox::warning(this, "Предупреждение!",
        "В этом поле допустимы только натуральные положительные числа!");
        return;
    }

    QJsonObject jsonData;
    jsonData["player_id"] = selectedId;
    jsonData["withdraw"] = withdraw;

    this->setInputsEnabled(false);

    rs->httpPost("company/pay_founder", jsonData, [this, withdraw](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();

        this->setInputsEnabled(true);
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            QString("Сумма в размере %1 тлц успешно выведена на счёт владельца!").arg(withdraw));
            refreshWindow();
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            QMessageBox::critical(this,
            "Ошибка 400 (ProtocolInvalidOperationError)",
            "Недостаточно талиц для вывода! "
            "Введите число меньше.");
        }
        else
        {
            QString errorString = reply->errorString();
            QMessageBox::critical(this,
            errorString,
            "Возникла неизвестная ошибка! Читайте подробнее в названии окна ошибки.");
        }
    });
}



void CompanyWindow::serviceLineEditChanged()
{
    QString lineEditText = ui->serviceLineEdit->text();
    QListView *listView = ui->serviceListView;

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



void CompanyWindow::payService()
{
    QList<ServicePayWindow *> widgetList = this->findChildren<ServicePayWindow *>();
    for (QWidget *widget : widgetList)
    {
        widget->close();
        widget->deleteLater();
    }

    QStandardItemModel *newModel = new QStandardItemModel(this);
    newModel->setColumnCount(2);
    ServicePayWindow *servicePayWindow = new ServicePayWindow(this, newModel);
    newModel->setParent(servicePayWindow);

    for (int row = 0; row < m_model->rowCount(); row++)
    {
        QStandardItem *item = m_model->item(row);
        if (item->checkState() == Qt::Checked)
        {
            QStandardItem *newItemData = new QStandardItem(item->text());
            newItemData->setData(item->data(Qt::UserRole).toInt(), Qt::UserRole);
            newItemData->setData(1, Qt::UserRole + 1);
            newItemData->setData(item->data(Qt::UserRole + 1), Qt::UserRole + 2);
            newItemData->setFlags(newItemData->flags() & ~Qt::ItemIsEditable);

            newModel->appendRow(newItemData);
        }
    }

    if (!newModel->rowCount())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы ничего не выбрали в списке товаров и услуг!");
        servicePayWindow->deleteLater();
        return;
    }

    servicePayWindow->show();

    for (int row = 0; row < m_model->rowCount(); row++)
    {
        QStandardItem *item = m_model->item(row);
        if (item->checkState() == Qt::Checked)
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
}



void CompanyWindow::refreshWindow()
{
    this->setInputsEnabled(false);

    rs->httpGet("company", [this](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();

        this->setInputsEnabled(true);
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QTextBrowser *foundersTextBrowser = ui->foundersNames;

            QString companyName = '"' + jsonData["name"].toString() + '"';
            bool companyIsState = jsonData["is_state"].toBool();

            if (companyIsState)
            {
                foundersTextBrowser->setText("Не предусмотрен, т.к. это гос-ое предприятие");
                ui->companyBalance->setText("Не предусмотрен, т.к. это гос-ое предприятие");
                ui->companyProfit->setText("Не предусмотрен, т.к. это гос-ое предприятие");
                ui->companyTaxes->setText("Не предусмотрены, т.к. это гос-ое предприятие");
                ui->founderLineEdit->setDisabled(true);
                ui->foundersListWidget->setDisabled(true);
                ui->founderWithdrawLineEdit->setDisabled(true);
                ui->founderWithdrawButton->setDisabled(true);
            }
            else {
                QString companyBalance = "Баланс фирмы: " + jsonData["balance"].toString() + " тлц";
                QString companyProfit = "Прибыль фирмы за период: " + jsonData["profit"].toString() + " тлц";
                QString companyTaxes = "Налоги фирмы за прошлый период: " + jsonData["tax_paid"].toString() + " тлц";
                QJsonArray founders = jsonData["founders"].toArray();

                if (ui->companyName->toPlainText() != companyName)
                {
                    ui->companyName->setText(companyName);
                }
                if (ui->companyBalance->text() != companyBalance && !companyIsState)
                {
                    ui->companyBalance->setText(companyBalance);
                }
                if (ui->companyProfit->text() != companyProfit && !companyIsState)
                {
                    ui->companyProfit->setText(companyProfit);
                }
                if (ui->companyTaxes->text() != companyTaxes && !companyIsState)
                {
                    ui->companyTaxes->setText(companyTaxes);
                }

                foundersTextBrowser->setText("| ");
                for (const QJsonValue founder : founders)
                {
                    QString oldText = foundersTextBrowser->toPlainText();
                    QString founderName = founder[0].toString() + " " + founder[1].toString();
                    foundersTextBrowser->setText(oldText + founderName + " | ");
                }
                if (founders.size() > 1) ui->foundersNames_label->setText("Владельцы фирмы: ");
                ui->companyName->setAlignment(Qt::AlignCenter);
            }



            rs->httpGet("/company/get_services", [this](QNetworkReply *reply)
            {
                QNetworkReply::NetworkError error = reply->error();

                if (commonNetworkError(error)) return;

                QListView *listView = ui->serviceListView;

                if (error == QNetworkReply::NoError)
                {
                    m_model->clear();

                    QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
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

                    listView->setModel(m_model);
                    listView->setSelectionMode(QAbstractItemView::NoSelection);
                }
                else
                {
                    QString errorString = reply->errorString();
                    QMessageBox::critical(this,
                    errorString,
                    "Возникла неизвестная ошибка! Читайте подробнее в названии окна ошибки.");
                }
            });
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            if (!this->isVisible()) return;
            QMessageBox::critical(this,
            "Ошибка",
            "Инфомарция о такой фирме не была найдена! "
            "Возможно, ваш токен устарел. Попробуйте войти ещё раз.");
            this->closeWindowsOpenAuth();
        }
        else
        {
            if (!this->isVisible()) return;
            QString errorString = reply->errorString();
            QMessageBox::critical(this,
            errorString,
            "Возникла неизвестная ошибка! Читайте подробнее в названии окна ошибки.");
            this->closeWindowsOpenAuth();
        }
    });
}



void CompanyWindow::keyPressEvent(QKeyEvent *event)
{
    int eventKey = event->key();

    if (eventKey == 16777216)
    {
        this->closeWindowsOpenAuth();
    }
    else if (event->key() == 16777220) // Enter - 16777220
    {
        if (ui->founderLineEdit->hasFocus() ||
            ui->founderWithdrawLineEdit->hasFocus())
        {
            payFounder();
        }
        if (ui->serviceLineEdit->hasFocus())
        {
            payService();
        }
    }
}
