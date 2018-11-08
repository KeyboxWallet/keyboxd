#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QtDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    auto appPath = QCoreApplication::applicationDirPath();
    auto macPath = appPath + "/../Resources";
    QString translateFileName = "keyboxd_ui_la";
    auto result = translator.load(QLocale(), translateFileName, ".", appPath) ||
        translator.load(QLocale(), translateFileName, ".", macPath);
    a.installTranslator(&translator);
    MainWindow w;
    w.show();

    return a.exec();
}
