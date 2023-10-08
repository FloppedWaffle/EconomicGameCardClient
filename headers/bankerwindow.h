#ifndef BANKERWINDOW_H
#define BANKERWINDOW_H


#include  "customwindow.h"


namespace Ui {
class BankerWindow;
}

class BankerWindow : public CustomWindow
{
    Q_OBJECT

public:
    explicit BankerWindow(QWidget *parent = nullptr);
    ~BankerWindow();

    void nfcReaderButtonClicked();



private:
    Ui::BankerWindow *ui;
};

#endif // BANKERWINDOW_H
