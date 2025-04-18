// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "plotwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    PlotWidget *plotWidget;
    QVector<QString> operationHistory;

private:
    QAction* undoAction;

private slots:
    void applyOffset(double offset);
    void undo();
    void undoToBase();
};

#endif // MAINWINDOW_H
