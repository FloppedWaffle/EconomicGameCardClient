#ifndef TEACHERWINDOW_H
#define TEACHERWINDOW_H


#include "customwindow.h"


namespace Ui
{
    class TeacherWindow;
}



class TeacherWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit TeacherWindow(QWidget *parent = nullptr);
    ~TeacherWindow() override;

    void navigateBarButtonClicked();

    void studentLineEditChanged();

    void payTalicButtonClicked();
    void payTaxesButtonClicked();

    void refreshWindow();



private:
    Ui::TeacherWindow *ui;

};

#endif // TEACHERWINDOW_H
