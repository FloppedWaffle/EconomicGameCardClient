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

    QListView *listView = ui->servicesListView;


    connect(listView, &QListView::clicked, this, &ServicePayWindow::servicesListViewClicked);

    connect(ui->servicePlusButton, &QPushButton::clicked, this, &ServicePayWindow::servicePlusButtonClicked);
    connect(ui->serviceMinusButton, &QPushButton::clicked, this, &ServicePayWindow::serviceMinusButtonClicked);

    connect(ui->servicePayButton, &QPushButton::clicked, this, &ServicePayWindow::servicePayButtonClicked);

    connect(ui->exitButton, &QPushButton::clicked, this, &ServicePayWindow::exitServicePayButtonClicked);

    listView->setModel(m_model);

    QTimer::singleShot(0, [this]()
    {
        this->setAllCost(m_model);
    });
}

ServicePayWindow::~ServicePayWindow()
{
    delete ui;
    delete m_model;
}



void ServicePayWindow::setAllCost(const QStandardItemModel *model)
{
    int allCost = 0;
    for (int row = 0; row < model->rowCount(); row++)
    {
        QStandardItem *item = model->item(row);
        int serviceQuantity = item->data(Qt::UserRole + 1).toInt();
        int serviceCost = item->data(Qt::UserRole + 2).toInt();
        allCost += serviceCost * serviceQuantity;
    }

    ui->serviceAllCostLineEdit->setText("Итог: " + QString::number(allCost) + " тлц");
}



void ServicePayWindow::servicesListViewClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->servicesListView->currentIndex());
    QString quantity = item->data(Qt::UserRole + 1).toString();

    ui->serviceLineEdit->setText(quantity + " шт");
}



void ServicePayWindow::servicePlusButtonClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->servicesListView->currentIndex());
    int serviceQuantity = item->data(Qt::UserRole + 1).toInt();

    item->setData(serviceQuantity + 1, Qt::UserRole + 1);

    this->servicesListViewClicked();
    this->setAllCost(m_model);
}



void ServicePayWindow::serviceMinusButtonClicked()
{
    QStandardItem *item = m_model->itemFromIndex(ui->servicesListView->currentIndex());
    int serviceQuantity = item->data(Qt::UserRole + 1).toInt();

    if (serviceQuantity <= 1) return;

    item->setData(serviceQuantity - 1, Qt::UserRole + 1);

    this->servicesListViewClicked();
    this->setAllCost(m_model);
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
    rs->httpPost("companies/pay_services", jsonData, [this](QNetworkReply *reply)
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
            QMessageBox::critical(this,
            "Ошибка!",
            "Возникла неизвестная ошибка!");
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

    if (ui->servicesListView->hasFocus())
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
            QModelIndex curIndex = ui->servicesListView->currentIndex();
            QModelIndex prevIndex = curIndex.sibling(curIndex.row() - 1, curIndex.column());

            if (prevIndex.isValid())
            {
                ui->servicesListView->setCurrentIndex(prevIndex);
            }

            servicesListViewClicked();
        }
        else if (eventKey == 16777236) // RIGHT ARROW
        {
            QModelIndex curIndex = ui->servicesListView->currentIndex();
            QModelIndex nextIndex = curIndex.sibling(curIndex.row() + 1, curIndex.column());

            if (nextIndex.isValid())
            {
                ui->servicesListView->setCurrentIndex(nextIndex);
            }

            servicesListViewClicked();
        }
    }
}
