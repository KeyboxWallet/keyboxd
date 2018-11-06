#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void startOrStopDaemon();
    void stateChanged(QProcess::ProcessState nState);
    void minimizeWindow();
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    QProcess mProcess;
    Ui::MainWindow *ui;
    // bool processStarted;
};

#endif // MAINWINDOW_H
