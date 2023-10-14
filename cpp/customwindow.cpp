#include "customwindow.h"
#include "authwindow.h"



CustomWindow::CustomWindow(QWidget *parent) :
    QMainWindow{parent},
//    rs(new RequestSender(QUrl("http://31.129.111.182:5000"), this)),
    nfcMgr(new NFCManager()),
    p_settings("EconomicGame", "AuthSettings")
{
    this->setWindowIcon(QIcon(":/img/img/economicgame.png"));

    bool prod = false;
    rs = new RequestSender(QUrl(prod ? "http://31.129.111.182:5000" : "http://192.168.1.187:5000"), this);
}


CustomWindow::~CustomWindow()
{
    delete rs;
    delete nfcMgr;
}



void CustomWindow::setInputsEnabled(const bool &boolean)
{
    QList<QPushButton*> allPushButtons = this->findChildren<QPushButton*>();
    QList<QRadioButton*> allRadioButtons = this->findChildren<QRadioButton*>();
    QList<QLineEdit*> allLines = this->findChildren<QLineEdit*>();
    QList<QListWidget*> allWidgets = this->findChildren<QListWidget*>();
    QList<QListView*> allViews = this->findChildren<QListView*>();

    if (boolean)
    {
        for (QPushButton *pushButton : allPushButtons)
        {
            pushButton->setEnabled(true);
        }
        for (QRadioButton *radioButton : allRadioButtons)
        {
            radioButton->setEnabled(true);
        }
        for (QLineEdit *line : allLines)
        {
            line->setEnabled(true);
        }
        for (QListWidget *widget : allWidgets)
        {
            widget->setEnabled(true);
        }
        for (QListView *view : allViews)
        {
            view->setEnabled(true);
        }
    }
    else
    {
        for (QPushButton *pushButton : allPushButtons)
        {
            pushButton->setDisabled(true);
        }
        for (QRadioButton *radioButton : allRadioButtons)
        {
            radioButton->setDisabled(true);
        }
        for (QLineEdit *line : allLines)
        {
            line->setDisabled(true);
        }
        for (QListWidget *widget : allWidgets)
        {
            widget->setDisabled(true);
        }
        for (QListView *view : allViews)
        {
            view->setDisabled(true);
        }
    }
}



bool CustomWindow::commonNetworkError(const QNetworkReply::NetworkError &error)
{
    if (!this->isVisible()) return false;

    if (error == QNetworkReply::AuthenticationRequiredError)
    {
        QMessageBox::critical(this,
        "Ошибка 401 (AuthenticationRequiredError)",
        "Вы неавторизованы! "
        "Попробуйте пройти авторизацию заново.");
        this->closeWindowsOpenAuth();
        return true;
    }
    else if (error == QNetworkReply::ConnectionRefusedError)
    {
        QMessageBox::critical(this,
        "Ошибка (ConnectionRefusedError)",
        "Сервер отклонил запрос! "
        "Возможно, сервер не был запущен или только-только настраивается. Ожидайте!");
        this->closeWindowsOpenAuth();
        return true;
    }
    else if (error == QNetworkReply::TimeoutError ||
             error == QNetworkReply::OperationCanceledError)
    {
        QMessageBox::critical(this,
        "Ошибка (TimeoutError или OperationCanceledError)",
        "В данный момент сервер недоступен!");
        this->closeWindowsOpenAuth();
        return true;
    }

    return false;
}



void CustomWindow::closeWindowsOpenAuth()
{
    if (!this->isVisible()) return;
    p_settings.clear();

    QList<QWidget *> topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget *widget : topLevelWidgets)
    {
        widget->close();
        widget->deleteLater();
    }


    AuthWindow *authWindow = new AuthWindow(nullptr);
    authWindow->show();
}
