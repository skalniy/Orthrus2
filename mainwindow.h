#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <utility>
#include "routercontroller.h"

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

    std::unique_ptr<RouterController> router;

    QMenu* routerMenu;
    QAction* startAct;

signals:
    void msgSend(std::string);
    void connectClicked(std::string addr);

public slots:
    void msgReceivedSlot(std::string nick, std::string msg);
    void error_handler(std::string, std::string);

private slots:
    void on_sendButton_clicked();
    void on_pushButton_clicked();
    void on_connectButton_clicked();
};

#endif // MAINWINDOW_H
