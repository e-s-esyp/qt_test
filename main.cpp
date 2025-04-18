#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
/*
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "test_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
*/
    QApplication::setStyle("Fusion");
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
    MainWindow window;
    window.resize(800, 600);
    window.show();
    window.plotWidget->fitToView();
    return a.exec();
}
