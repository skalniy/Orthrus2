#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<std::string>("std::string");

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{  
    delete ui;
}


void MainWindow::msgReceivedSlot(std::string nick, std::string msg)
{
    ui->msgView->append(QString::fromStdString("["+nick+"] " + msg));
}

void MainWindow::on_sendButton_clicked()
{
    std::string msg = ui->msgEdit->toPlainText().toStdString();
    emit msgSend(msg);
    ui->msgView->append(QString::fromStdString("["+router->get_hostname()+" (you)] ")
                        + ui->msgEdit->toPlainText());
    ui->msgEdit->clear();
}

void MainWindow::on_pushButton_clicked()
{
    router.reset(nullptr);
    router.reset(new RouterController(this, ui->hostPortBox->value(),
                                      ui->hostnameEdit->text().toStdString()));
    router->start();
    QObject::connect(router.get(), &RouterController::msgReceived, this, &MainWindow::msgReceivedSlot);
    QObject::connect(this, &MainWindow::msgSend, router.get(), &RouterController::msgSend);
    QObject::connect(this, &MainWindow::connectClicked, router.get(), &RouterController::connectClicked);
}



void MainWindow::on_connectButton_clicked()
{
    emit connectClicked(ui->peerIPEdit->text().toStdString() + ':' + std::to_string(ui->peerPortBox->value()));
}
