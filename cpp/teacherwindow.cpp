#include "ui_teacherwindow.h"

#include "teacherwindow.h"
#include "authwindow.h"

TeacherWindow::TeacherWindow(QWidget *parent) :
    CustomWindow(parent),
    ui(new Ui::TeacherWindow)
{
    //TODO: надо доделать ошибки у запросов (посмотреть, какие ошибки могут прилетать с бэка и дописать тут окна ошибок.

    ui->setupUi(this);

    this->setWindowTitle("Учитель");
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->navTeacherButton, &QPushButton::clicked, this, &TeacherWindow::navigateBarButtonClicked);
    connect(ui->navMemoButton, &QPushButton::clicked, this, &TeacherWindow::navigateBarButtonClicked);

    connect(ui->studentLineEdit, &QLineEdit::textChanged, this, &TeacherWindow::studentLineEditChanged);

    connect(ui->payFiveButton, &QPushButton::clicked, this, &TeacherWindow::payTalicButtonClicked);
    connect(ui->payTenButton, &QPushButton::clicked, this, &TeacherWindow::payTalicButtonClicked);
    connect(ui->payTwentyButton, &QPushButton::clicked, this, &TeacherWindow::payTalicButtonClicked);
    connect(ui->payThirtyButton, &QPushButton::clicked, this, &TeacherWindow::payTalicButtonClicked);
    connect(ui->payTaxesButton, &QPushButton::clicked, this, &TeacherWindow::payTaxesButtonClicked);

    connect(ui->exitButton, &QPushButton::clicked, this, &CustomWindow::closeWindowsOpenAuth);
    connect(ui->refreshButton, &QPushButton::clicked, this, &TeacherWindow::refreshWindow);


    refreshWindow();
}

TeacherWindow::~TeacherWindow()
{
    delete ui;
}



void TeacherWindow::navigateBarButtonClicked()
{
    QString senderButtonName = qobject_cast<QPushButton*>(sender())->objectName();

    ui->navTeacherButton->setStyleSheet("border-bottom: none;");
    ui->navMemoButton->setStyleSheet("border-bottom: none;");

    QString styleString = "border-bottom: 3px solid black; border-left: 3px solid black; border-right: 3px solid black;";
    if (senderButtonName == "navTeacherButton")
    {
        ui->navTeacherButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->teacherPage);
    }
    else
    {
        ui->navMemoButton->setStyleSheet(styleString);
        ui->stackedWidget->setCurrentWidget(ui->memoPage);
    }
}



void TeacherWindow::studentLineEditChanged()
{
    QListWidget *listWidget = ui->studentsListWidget;
    QString studentName = ui->studentLineEdit->text();

    listWidget->clear();

    if (studentName.split(" ").size() != 2)
    {
        return;
    }

    QStringList stringList = studentName.split(" ");
    QJsonObject jsonObject;
    jsonObject["firstname"] = stringList[0];
    jsonObject["lastname"] = stringList[1];

    rs->httpPost("teachers/get_students", jsonObject, [listWidget](QNetworkReply *reply)
    {
        QNetworkReply::NetworkError error = reply->error();
        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());
            QJsonArray players = jsonData["players"].toArray();
            for (const QJsonValue player : players)
            {
                QString firstname = player[0].toString();
                QString lastname = player[1].toString();
                QString grade = player[2].toString();
                int playerId = player[3].toInt();

                QString playerInfo = QString("%1 %2 | Класс: %3").arg(firstname).arg(lastname).arg(grade);

                QListWidgetItem *item = new QListWidgetItem(playerInfo);
                item->setData(Qt::UserRole, playerId);
                listWidget->addItem(item);
            }
        }
    });
}



void TeacherWindow::payTalicButtonClicked()
{
    QListWidgetItem *selectedItem = ui->studentsListWidget->currentItem();
    if (!selectedItem)
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не выбрали ученика из списка! "
        "Выберите ученика в таблице слева, перед этим набрав его имя и фамилию в поле по этому примеру: \"Иван Иванов\". ");
        return;
    }

    QJsonObject jsonData;

    if (selectedItem)
    {
        QString selectedId = selectedItem->data(Qt::UserRole).toString();
        jsonData["player_id"] = selectedId;
    }

    int salary = 0;
    QString senderButtonName = qobject_cast<QPushButton*>(sender())->objectName();
    if (senderButtonName == "payFiveButton")
    {
        salary = 5;
    }
    else if (senderButtonName == "payTenButton")
    {
        salary = 10;
    }
    else if (senderButtonName == "payTwentyButton")
    {
        salary = 20;
    }
    else if (senderButtonName == "payThirtyButton")
    {
        salary = 30;
    }
    jsonData["salary"] = salary;


    this->setInputsEnabled(false);

    rs->httpPost("teachers/pay_student_salary", jsonData, [this, salary](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            QString("Зарплата в размере %1 тлц успешно зачислена на счёт ученика.").arg(salary));
        }
        else
        {
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Подробности в названии окна ошибки.");
            this->closeWindowsOpenAuth();
        }
    });
}



void TeacherWindow::payTaxesButtonClicked()
{
    QListWidgetItem *selectedItem = ui->studentsListWidget->currentItem();
    if (!selectedItem && !nfcMgr->isCardAttached())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы не выбрали ученика из списка или его карта не была приложена к считывателю! "
        "Выберите ученика в таблице слева, перед этим набрав его имя и фамилию в поле по этому примеру: \"Иван Иванов\". "
        "Иначе просто приложите карту ученика к считывателю, если такой имеется.");
        return;
    }
    if (selectedItem && nfcMgr->isCardAttached())
    {
        QMessageBox::warning(this,
        "Предупреждение!",
        "Вы одновременно приложили карту и выбрали ученика в списке. Выберите что-то одно: уберите карту, чтобы воспользоваться "
        "списком или очистите текстовое поле, чтобы воспользоваться картой.");
        return;
    }


    bool isCard;
    QJsonObject jsonData;

    if (selectedItem)
    {
        QString selectedId = selectedItem->data(Qt::UserRole).toString();
        jsonData["player_id"] = selectedId;
        isCard = false;
    }
    else
    {
        QString uid = nfcMgr->getCardUID();
        jsonData["uid"] = uid;
        isCard = true;
    }
    jsonData["is_card"] = isCard;


    this->setInputsEnabled(false);

    rs->httpPost("teachers/pay_student_taxes", jsonData, [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QMessageBox::information(this,
            "Успешно!",
            "Налоги ученика были уплачены!");
        }
        else
        {
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Подробности в названии окна ошибки.");
            this->closeWindowsOpenAuth();
        }
    });
}



void TeacherWindow::refreshWindow()
{
    this->setInputsEnabled(false);

    rs->httpGet("teachers", [this](QNetworkReply *reply)
    {
        this->setInputsEnabled(true);
        if (!this->isVisible()) return;
        QNetworkReply::NetworkError error = reply->error();
        if (commonNetworkError(error)) return;

        if (error == QNetworkReply::NoError)
        {
            QJsonDocument jsonData = QJsonDocument::fromJson(reply->readAll());

            QString teacherName =  jsonData["name"].toString();
            QString teacherBalance = jsonData["balance"].toString();
            QString teacherSubjectName = jsonData["subject_name"].toString();

            if (QDate::currentDate() > QDate(QDate::currentDate().year(), 2, 12))
            {
                ui->payFiveButton->setDisabled(true);
            }

            if (ui->teacherName->text() != teacherName)
            {
                ui->teacherName->setText("Имя и отчество: " + teacherName);
            }
            if (ui->teacherBalance->text() != teacherBalance)
            {
                ui->teacherBalance->setText("Ваш личный баланс: " + teacherBalance + " тлц");
            }
            if (ui->teacherSubjectName->toPlainText() != teacherSubjectName)
            {
                ui->teacherSubjectName->setPlainText("Название вашего предприятия:");
                ui->teacherSubjectName->setAlignment(Qt::AlignCenter);
                ui->teacherSubjectName->append(teacherSubjectName);
                ui->teacherSubjectName->setAlignment(Qt::AlignCenter);
            }
        }
        else if (error == QNetworkReply::ContentNotFoundError)
        {
            QMessageBox::critical(this,
            "Ошибка 404 (ContentNotFoundError)",
            "Инфомарция о таком учителе не была найдена! "
            "Возможно, ваш токен устарел. Попробуйте войти ещё раз.");
            this->closeWindowsOpenAuth();
        }
        else
        {
            QString errorString = reply->errorString();
            QMessageBox::critical(this, errorString,
            "Возникла неизвестная ошибка! Подробности в названии окна ошибки.");
            this->closeWindowsOpenAuth();
        }
    });
}
