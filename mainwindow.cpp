#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) try :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<std::string>("std::string");
    ui->setupUi(this);
} catch (boost::system::system_error& e) {
    error_handler("Router", e.what());
}

MainWindow::~MainWindow() try
{  
    delete ui;
} catch (boost::system::system_error& e) {
    error_handler("Router", e.what());
}

void MainWindow::msgReceivedSlot(std::string nick, std::string msg) try
{
    ui->msgView->append(QString::fromStdString("["+nick+"] " + msg));
} catch (boost::system::system_error& e) {
    error_handler("Router", e.what());
}

void MainWindow::on_sendButton_clicked() try
{
    std::string msg = ui->msgEdit->toPlainText().toStdString();
    emit msgSend(msg);
    ui->msgView->append(QString::fromStdString("["+router->get_hostname()+" (you)] ")
                        + ui->msgEdit->toPlainText());
    ui->msgEdit->clear();
} catch (boost::system::system_error& e) {
    error_handler("Router", e.what());
}

void MainWindow::on_pushButton_clicked() try
{
    ui->peerIPEdit->setEnabled(false);
    ui->peerPortBox->setEnabled(false);
    ui->connectButton->setEnabled(false);
    router.reset(nullptr);
    router.reset(new RouterController(this, ui->hostPortBox->value(),
                                      ui->hostnameEdit->text().toStdString()));
    router->start();
    QObject::connect(router.get(), &RouterController::error, this, &MainWindow::error_handler);
    QObject::connect(router.get(), &RouterController::msgReceived, this, &MainWindow::msgReceivedSlot);
    QObject::connect(this, &MainWindow::msgSend, router.get(), &RouterController::msgSend);
    QObject::connect(this, &MainWindow::connectClicked, router.get(), &RouterController::connectClicked);
    ui->peerIPEdit->setEnabled(true);
    ui->peerPortBox->setEnabled(true);
    ui->connectButton->setEnabled(true);
} catch (boost::system::system_error& e) {
    error_handler("Router", e.what());
}


void MainWindow::on_connectButton_clicked() try
{
    emit connectClicked(ui->peerIPEdit->text().toStdString() + ':' + std::to_string(ui->peerPortBox->value()));
} catch (boost::system::system_error& e) {
    error_handler("Router", e.what());
}


void MainWindow::error_handler(std::string name, std::string what)
{
    QMessageBox::critical(this, QString::fromStdString(name), QString::fromStdString(what));
}
// нет проверки на правильность введенных данных
