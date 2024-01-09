#include <QApplication>

#include "authwindow.h"
#include "teacherwindow.h"
#include "companywindow.h"
#include "servicepaywindow.h"
#include "bankerwindow.h"
#include "atmwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/img/img/economicgame.png"));

    QSettings settings("EconomicGame", "AuthSettings");
    QString role = settings.value("role").toString();

    QMainWindow *mainWindow;
    if (role == "teacher")
    {
        mainWindow = new TeacherWindow(nullptr);
    }
    else if (role == "company")
    {
        mainWindow = new CompanyWindow(nullptr);
    }
    else if (role == "banker")
    {
        mainWindow = new BankerWindow(nullptr);
    }
    else if (role == "atm")
    {
        mainWindow = new ATMWindow(nullptr);
    }
    else
    {
        mainWindow = new AuthWindow(nullptr);
    }
    mainWindow->show();

    return a.exec();
}
