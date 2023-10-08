#include "companywindow.h"
#include "servicepaywindow.h"
#include "ui_servicepaywindow.h"


ServicePayWindow::ServicePayWindow(QWidget *parent, QStandardItemModel *model) :
    CustomWindow(parent),
    ui(new Ui::ServicePayWindow),
    m_model(model)
{
    ui->setupUi(this);
    this->setWindowTitle("Оплата товаров и услуг");

    QListView *listView = ui->serviceListView;


    connect(listView, &QListView::clicked, this, &ServicePayWindow::serviceListViewClicked);

    connect(ui->servicePlusButton, &QPushButton::clicked, this, &ServicePayWindow::servicePlusButtonClicked);
    connect(ui->serviceMinusButton, &QPushButton::clicked, this, &ServicePayWindow::serviceMinusButtonClicked);

    connect(ui->servicePayButton, &QPushButton::clicked, this, &ServicePayWindow::servicePayButtonClicked);

    connect(ui->exitButton, &QPushButton::clicked, this, &ServicePayWindow::exitServicePayButtonClicked);


    listView->setModel(m_model);
}

ServicePayWindow::~ServicePayWindow()
{
    delete ui;
    delete m_model;
}



void ServicePayWindow::serviceListViewClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->serviceListView->currentIndex());
    QString quantity = item->data(Qt::UserRole + 1).toString();

    ui->serviceLineEdit->setText(quantity + " шт");

    int allCost = 0;
    for (int row = 0; row < m_model->rowCount(); row++)
    {
        QStandardItem *item = m_model->item(row);
        int serviceQuantity = item->data(Qt::UserRole + 1).toInt();
        int serviceCost = item->data(Qt::UserRole + 2).toInt();
        allCost += serviceCost * serviceQuantity;
    }

    ui->serviceAllCostLineEdit->setText("Итог: " + QString::number(allCost) + " тлц");
}



void ServicePayWindow::servicePlusButtonClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->serviceListView->currentIndex());
    int serviceQuantity = item->data(Qt::UserRole + 1).toInt();

    item->setData(serviceQuantity + 1, Qt::UserRole + 1);
    serviceListViewClicked();
}



void ServicePayWindow::serviceMinusButtonClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->serviceListView->currentIndex());
    int serviceQuantity = item->data(Qt::UserRole + 1).toInt();

    if (serviceQuantity <= 1)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Нельзя ставить количество товаров/услуг меньше единицы!");
        return;
    }

    item->setData(serviceQuantity - 1, Qt::UserRole + 1);
    serviceListViewClicked();
}



void ServicePayWindow::servicePayButtonClicked()
{
    bool isCard = ui->serviceCardRadio->isChecked();
    if (isCard && !nfcMgr->isCardAttached())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Карта не приложена к считывателю!");
        return;
    }

    QJsonObject jsonData;
    QJsonArray pairsArray;
    jsonData["uid"] = nfcMgr->getCardUID();

    for (int row = 0; row < m_model->rowCount(); row++)
    {
        QStandardItem *item = m_model->item(row);

        int serviceId = item->data(Qt::UserRole).toInt();
        int serviceQuantity = item->data(Qt::UserRole + 1).toInt();

        QJsonObject pairJsonObject;
        pairJsonObject["service_id"] = serviceId;
        pairJsonObject["quantity"] = serviceQuantity;

        pairsArray.append(pairJsonObject);
    }

    jsonData["services"] = pairsArray;

    this->setInputsEnabled(false);

    rs->httpPost("company/pay_services", jsonData, [this](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();
        QString errorReason = QJsonDocument::fromJson(reply->readAll())["error"].toString();
        this->setInputsEnabled(true);
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            if (!this->isVisible()) return;
            QMessageBox::information(this,
            "Успешно! ",
            "Оплата завершена корректно.");
            this->close();

            CompanyWindow *companyWindow = qobject_cast<CompanyWindow*>(parent());
            companyWindow->refreshWindow();
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            if (!this->isVisible()) return;
            QMessageBox::critical(this,
            "Ошибка 404 (ContentNotFoundError)",
            "Информация о таком покупателе не была найдена!");
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            if (errorReason == "no_enough_quantity")
            {
                if (!this->isVisible()) return;
                QMessageBox::critical(this,
                "Ошибка 400 (ProtocolInvalidOperationError)",
                "Недостаточно товаров/услуг для оплаты!");
            }
            else if (errorReason == "not_enough_money")
            {
                if (!this->isVisible()) return;
                QMessageBox::critical(this,
                "Ошибка 400 (ProtocolInvalidOperationError)",
                "Недостаточно средств для оплаты!");
            }
        }
        else
        {
            if (!this->isVisible()) return;
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Читайте подробнее в названии окна ошибки.");
        }
    });
}



void ServicePayWindow::exitServicePayButtonClicked()
{
    this->close();
    this->deleteLater();
}



void ServicePayWindow::keyPressEvent(QKeyEvent *event)
{
    int eventKey = event->key();

    if (eventKey == 16777216)
    {
        this->close();
    }
    else if (ui->serviceListView->hasFocus())
    {
        if (eventKey == 16777235)   // Enter
        {
            serviceListViewClicked();
        }
        else if (eventKey == 61)    // CTRL + (+)
        {
            servicePlusButtonClicked();
        }
        else if (eventKey == 45)    // CTRL + (-)
        {
            serviceMinusButtonClicked();
        }

        this->setFocus();
        ui->serviceListView->setFocus();
    }
}
