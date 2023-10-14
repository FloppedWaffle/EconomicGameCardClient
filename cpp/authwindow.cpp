#include "ui_authwindow.h"


#include "authwindow.h"
#include "teacherwindow.h"
#include "companywindow.h"
#include "bankerwindow.h"



AuthWindow::AuthWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::AuthWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);
    this->setWindowTitle("Добро пожаловать!");
    QTimer::singleShot(1500, [this]()
    {
        this->setWindowTitle("Главная страница");
    });
    ui->authLineEdit->setFocus();

    connect(ui->navAuthButton, &QPushButton::clicked, this, &AuthWindow::navigateBarButtonClicked);
    connect(ui->navAboutButton, &QPushButton::clicked, this, &AuthWindow::navigateBarButtonClicked);
    connect(ui->navRulesButton, &QPushButton::clicked, this, &AuthWindow::navigateBarButtonClicked);
    connect(ui->navFaqButton, &QPushButton::clicked, this, &AuthWindow::navigateBarButtonClicked);

    connect(ui->authRequestButton, &QPushButton::clicked, this, &AuthWindow::authRequestButtonClicked);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}



void AuthWindow::navigateBarButtonClicked()
{
    QString senderButtonName = qobject_cast<QPushButton*>(sender())->objectName();

    ui->navAuthButton->setStyleSheet("border-bottom: none;");
    ui->navAboutButton->setStyleSheet("border-bottom: none;");
    ui->navRulesButton->setStyleSheet("border-bottom: none;");
    ui->navFaqButton->setStyleSheet("border-bottom: none;");

    QString styleString = "border-bottom: 3px solid black; border-left: 3px solid black; border-right: 3px solid black;";
    if (senderButtonName == "navAuthButton")
    {
        ui->navAuthButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->authPage);
        ui->authLineEdit->setFocus();
    }
    else if (senderButtonName == "navAboutButton")
    {
        ui->navAboutButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->aboutPage);
    }
    else if (senderButtonName == "navRulesButton")
    {
        ui->navRulesButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->rulesPage);
    }
    else
    {
        ui->navFaqButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->faqPage);
    }
}



void AuthWindow::authRequestButtonClicked()
{
    QString code = ui->authLineEdit->text();
    if (code.size() < 10)
    {
        QMessageBox::critical(this, "Ошибка", "Недостаточно символов! Необходимо ввести 10 символов.");
        return;
    }

    p_settings.clear();

    QString hashedCode = QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex();
    QJsonObject jsonObject;
    jsonObject["auth_password"] = hashedCode;

    this->setInputsEnabled(false);

    rs->httpPost("auth", jsonObject, [this](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();
        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QString token = jsonData["auth_token"].toString();
            QString role = jsonData["role"].toString();

            p_settings.setValue("token", token);
            p_settings.setValue("role", role);

            QMainWindow *mainWindow;
            if (role == "teacher")
            {
                this->close();
                mainWindow = new TeacherWindow(nullptr);
                this->deleteLater();
            }
            else if (role == "company")
            {
                this->close();
                mainWindow = new CompanyWindow(nullptr);
                this->deleteLater();
            }
            else
            {
                this->close();
                mainWindow = new BankerWindow(nullptr);
                this->deleteLater();
            }

            mainWindow->show();
            return;
        }
        else if (error == QNetworkReply::AuthenticationRequiredError)
        {
            QMessageBox::critical(this,
            "Ошибка 401 (AuthenticationRequiredError)",
            "Неправильный код авторизации! "
            "Попробуйте проверить корректность вводимого вами кода.");
        }
        else if (error == QNetworkReply::ConnectionRefusedError)
        {
            QMessageBox::critical(this,
            "Ошибка (ConnectionRefusedError)",
            "Сервер отклонил запрос! "
            "Возможно, сервер не был запущен или только-только настраивается. Ожидайте!");
        }
        else if (error == QNetworkReply::TimeoutError ||
                 error == QNetworkReply::OperationCanceledError)
        {
            QMessageBox::critical(this,
            "Ошибка (TimeoutError или OperationCanceledError)",
            "В данный момент сервер недоступен! "
            "Попробуйте авторизоваться позже.");
        }
        else
        {
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Читайте подробнее в названии окна ошибки.");
        }

        this->setInputsEnabled(true);
        this->setFocus();
        ui->authLineEdit->setFocus();
    });
}



void AuthWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == 16777220) // Enter - 16777220
    {
        if (ui->authLineEdit->hasFocus())
        {
            authRequestButtonClicked();
        }
    }
}
