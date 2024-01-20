#include "adminwindow.h"
#include "ui_adminwindow.h"

AdminWindow::AdminWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::AdminWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Админ-панелька ;3");

    ui->stackedWidget->setCurrentWidget(ui->registerPage);

    connect(ui->navRegisterButton, &QPushButton::clicked, this, &AdminWindow::navigateBarButtonClicked);
    connect(ui->navChangeButton, &QPushButton::clicked, this, &AdminWindow::navigateBarButtonClicked);


    connect(ui->registerPersonButton, &QPushButton::clicked, this, &AdminWindow::registerPersonButtonClicked);
    connect(ui->registerCompanyButton, &QPushButton::clicked, this, &AdminWindow::registerCompanyButtonClicked);
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
    QString firstname = ui->firstnameLineEdit->text();
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
    jsonObject["firstname"] = firstname;
    jsonObject["lastname"] = lastname;
    jsonObject["grade"] = grade;
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
            ui->firstnameLineEdit->clear();
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
    jsonObject["company_name"] = companyName;

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
            "Фирма не была зарегистрирована, т.к. фирма с таким именем уже присутствует в базе данных.");
        }
    });
}
