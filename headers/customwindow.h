#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStyleFactory>
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListView>
#include <QDebug>


#include "requestsender.h"
#include "nfcmanager.h"



class CustomWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CustomWindow(QWidget *parent = nullptr);
    virtual ~CustomWindow();

    virtual void setInputsEnabled(const bool &boolean);
    virtual bool commonNetworkError(const QNetworkReply::NetworkError &error);
    virtual void closeWindowsOpenAuth();



protected:
    RequestSender *rs;
    NFCManager *nfcMgr;
    QSettings p_settings;
};

#endif // CUSTOMWINDOW_H
