#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H


#include <QKeyEvent>
#include <QCryptographicHash>
#include <QtNetwork>
#include <functional>

#include "customwindow.h"



namespace Ui
{
    class AuthWindow;
}



class AuthWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

    void navigateBarButtonClicked();

    void authRequestButtonClicked();

    void keyPressEvent(QKeyEvent *event) override;



private:
    Ui::AuthWindow *ui;
};

#endif // AUTHWINDOW_H
