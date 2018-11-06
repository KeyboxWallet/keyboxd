#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mProcess(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    connect(&mProcess, &QProcess::stateChanged, this, &MainWindow::stateChanged);
    connect(ui->startStopButton, &QPushButton::clicked, this, &MainWindow::startOrStopDaemon);
    startOrStopDaemon();
}

MainWindow::~MainWindow()
{
}

void MainWindow::startOrStopDaemon()
{
    if( mProcess.state() == QProcess::Running) {
        mProcess.kill();
    }
    //
    else if(mProcess.state() == QProcess::NotRunning){
        mProcess.setProgram("./keyboxd");
        mProcess.setWorkingDirectory(qApp->applicationDirPath());
        mProcess.start();
    }
}

void MainWindow::stateChanged(QProcess::ProcessState nState)
{
    if(nState == QProcess::NotRunning) {
        ui->startStopButton->setStyleSheet("color: green");
        ui->startStopButton->setText("  start daemon ");
        ui->statusBar->showMessage("daemon not running");
        ui->statusBar->setStyleSheet("color: red");

    }
    if(nState == QProcess::Running) {
        ui->startStopButton->setStyleSheet("color: red");
        ui->startStopButton->setText("  stop daemon");
        ui->statusBar->showMessage("daemon running ok.");
        ui->statusBar->setStyleSheet("color: green");
    }
}
