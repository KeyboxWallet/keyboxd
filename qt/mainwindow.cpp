#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QTimer>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mProcess(parent),
    ui(new Ui::MainWindow)
{
	auto appPath = QCoreApplication::applicationDirPath();
#ifdef _WIN32
	QIcon icon(appPath + "/icon.ico");
	setWindowIcon(icon);
#endif
    ui->setupUi(this);
    connect(&mProcess, &QProcess::stateChanged, this, &MainWindow::stateChanged);
    connect(ui->startStopButton, &QPushButton::clicked, this, &MainWindow::startOrStopDaemon);
    startOrStopDaemon();
}

MainWindow::~MainWindow()
{
    if( mProcess.state() == QProcess::Running) {
        mProcess.kill();
    }
}

void MainWindow::startOrStopDaemon()
{
    if( mProcess.state() == QProcess::Running) {
        mProcess.kill();
    }
    //
    else if(mProcess.state() == QProcess::NotRunning){
		auto appPath = QCoreApplication::applicationDirPath();
        mProcess.setProgram(appPath + "/keyboxd");
        mProcess.setWorkingDirectory(qApp->applicationDirPath());
        mProcess.start();
    }
}

void MainWindow::stateChanged(QProcess::ProcessState nState)
{
    if(nState == QProcess::NotRunning) {
        ui->startStopButton->setStyleSheet("color: green");
        ui->startStopButton->setText(tr("start daemon"));
        ui->statusBar->showMessage(tr("daemon not running"));
        ui->statusBar->setStyleSheet("color: red");

    }
    if(nState == QProcess::Running) {
        ui->startStopButton->setStyleSheet("color: red");
        ui->startStopButton->setText(tr("stop daemon"));
        ui->statusBar->showMessage(tr("daemon running ok."));
        ui->statusBar->setStyleSheet("color: green");
        auto win = this;
        QTimer::singleShot(2000, this, &MainWindow::minimizeWindow);
    }
}

void MainWindow::minimizeWindow()
{
    this->setWindowState(Qt::WindowMinimized);
}