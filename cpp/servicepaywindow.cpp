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

    if (serviceQuantity <= 1) return;

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
    jsonData["uid"] = isCard ? nfcMgr->getCardUID() : "";

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
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;
        QString errorReason = QJsonDocument::fromJson(reply->readAll())["error"].toString();

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно! ",
            "Оплата завершена корректно.");
            this->close();
            this->deleteLater();

            CompanyWindow *companyWindow = qobject_cast<CompanyWindow*>(parent());
            companyWindow->refreshWindow();
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(this,
            "Ошибка 404 (ContentNotFoundError)",
            "Информация о таком покупателе не была найдена!");
        }
        else if (error == QNetworkReply::ProtocolInvalidOperationError)
        {
            if (errorReason == "no_enough_quantity")
            {
                QMessageBox::critical(this,
                "Ошибка 400 (ProtocolInvalidOperationError)",
                "Недостаточно товаров/услуг для оплаты!");
            }
            else if (errorReason == "not_enough_money")
            {
                QMessageBox::critical(this,
                "Ошибка 400 (ProtocolInvalidOperationError)",
                "Недостаточно средств для оплаты!");
            }
        }
        else
        {
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
    qDebug() << eventKey;

    if (eventKey == 16777216) // ESC
    {
        this->close();
        this->deleteLater();
    }
    else if (ui->serviceListView->hasFocus())
    {
        if (eventKey == 61)    // CTRL + (+)
        {
            servicePlusButtonClicked();
        }
        else if (eventKey == 45)    // CTRL + (-)
        {
            serviceMinusButtonClicked();
        }
        else if (eventKey == 16777234) // LEFT ARROW
        {
            QModelIndex curIndex = ui->serviceListView->currentIndex();
            QModelIndex prevIndex = curIndex.sibling(curIndex.row() - 1, curIndex.column());

            if (prevIndex.isValid())
            {
                ui->serviceListView->setCurrentIndex(prevIndex);
            }

            serviceListViewClicked();
        }
        else if (eventKey == 16777236) // RIGHT ARROW
        {
            QModelIndex curIndex = ui->serviceListView->currentIndex();
            QModelIndex nextIndex = curIndex.sibling(curIndex.row() + 1, curIndex.column());

            if (nextIndex.isValid())
            {
                ui->serviceListView->setCurrentIndex(nextIndex);
            }

            serviceListViewClicked();
        }

//        this->setFocus();
//        ui->serviceListView->setFocus();
    }
}
