#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) try :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<std::string>("std::string");
    ui->setupUi(this);
}
catch (std::exception& e) { error_handler(e); }

MainWindow::~MainWindow()
{ delete ui; }

void MainWindow::msgReceivedSlot(std::string nick, std::string msg) try
{
    ui->msgView->append(QString::fromStdString("["+nick+"] " + msg));
}
catch (std::exception& e) { error_handler(e); }

void MainWindow::on_sendButton_clicked() try
{
    std::string msg = ui->msgEdit->toPlainText().toStdString();
    rc->get_router().send(msg);
    ui->msgView->append(QString::fromStdString("["+hostname.toStdString()+" (you)] ")
                        + ui->msgEdit->toPlainText());
    ui->msgEdit->clear();
}
catch (std::exception& e) { error_handler(e); }

void MainWindow::on_pushButton_clicked() try
{
    ui->peerIPEdit->setEnabled(false);
    ui->peerPortBox->setEnabled(false);
    ui->connectButton->setEnabled(false);
    hostname = ui->hostnameEdit->text();
    rc.reset(nullptr);
    rc.reset(new Orthrus::RouterController(hostname.toStdString(),
                                           ui->hostPortBox->value(),
                                           boost::bind(&MainWindow::error_handler, this, _1)));
    rc->get_router().set_read_msg_cb(boost::bind(&MainWindow::msgReceivedSlot, this, _1, _2));
    rc->start();
    ui->peerIPEdit->setEnabled(true);
    ui->peerPortBox->setEnabled(true);
    ui->connectButton->setEnabled(true);
}
catch (std::exception& e) { error_handler(e); }

void MainWindow::on_connectButton_clicked() try
{ rc->get_router().connect(ui->peerIPEdit->text().toStdString() + ':' + std::to_string(ui->peerPortBox->value())); }
catch (std::exception& e) { error_handler(e); }


void MainWindow::error_handler(std::exception& e)
{ QMessageBox::critical(this, QString::fromStdString("Ошибка"), QString::fromStdString(e.what())); }
