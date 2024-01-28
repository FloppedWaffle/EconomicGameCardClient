#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H


#include <QKeyEvent>
#include <QToolButton>
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
    ~AuthWindow() override;

    void navigateBarButtonClicked();
    void togglePasswordVisibilityPressed();
    void togglePasswordVisibilityReleased();
    void authRequestButtonClicked();

    void keyPressEvent(QKeyEvent *event) override;



private:
    Ui::AuthWindow *ui;
    QToolButton *eyeButton;
};

#endif // AUTHWINDOW_H
