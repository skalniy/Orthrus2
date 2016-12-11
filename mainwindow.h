#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <utility>
#include "core/router_controller.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QString hostname;

    std::unique_ptr<Orthrus::RouterController> rc;

signals:
    void connectClicked(std::string addr);

public slots:
    void msgReceivedSlot(std::string nick, std::string msg);
    void error_handler(std::exception& e);


private slots:
    void on_sendButton_clicked();
    void on_pushButton_clicked(); // config & run
    void on_connectButton_clicked();
};

#endif // MAINWINDOW_H
