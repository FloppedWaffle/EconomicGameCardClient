#include <QApplication>
#include <QFontDatabase>

#include "authwindow.h"
#include "teacherwindow.h"
#include "companywindow.h"
#include "servicepaywindow.h"
#include "bankerwindow.h"
#include "atmwindow.h"
#include "adminwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    a.setWindowIcon(QIcon(":/img/img/economicgame.png"));

    QFile fontFile(":/fonts/fonts/FiraSansCondensed-Medium.ttf");
    fontFile.open(QIODevice::ReadOnly);
    QByteArray fontData = fontFile.readAll();
    fontFile.close();

    int fontId = QFontDatabase::addApplicationFontFromData(fontData);
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);

    if (!fontFamilies.isEmpty()) {
        QFont font(fontFamilies.at(0));
        a.setFont(font);
    }


    QSettings settings("EconomicGame", "AuthSettings");
    QString role = settings.value("role").toString();

    QMainWindow *mainWindow;
    if (role == "teachers")
    {
        mainWindow = new TeacherWindow(nullptr);
    }
    else if (role == "companies")
    {
        mainWindow = new CompanyWindow(nullptr);
    }
    else if (role == "bankers")
    {
        mainWindow = new BankerWindow(nullptr);
    }
    else if (role == "atm")
    {
        mainWindow = new ATMWindow(nullptr);
    }
    else if (role == "admin")
    {
        mainWindow = new AdminWindow(nullptr);
    }
    else
    {
        mainWindow = new AuthWindow(nullptr);
    }
    mainWindow->show();

    return a.exec();
}
