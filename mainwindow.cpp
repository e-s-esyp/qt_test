// mainwindow.cpp
#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QSvgRenderer>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    plotWidget = new PlotWidget(this);
    setCentralWidget(plotWidget);

    QMenu *functionMenu = menuBar()->addMenu(tr("Функция"));
    undoAction = new QAction(QIcon("icons/undo.svg"), tr("Вернуть"), this);
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    functionMenu->addAction(undoAction);
    QMenu* undoMenu = new QMenu("Вернуть к", this);
    connect(undoMenu, &QMenu::aboutToShow, this, [=]() {
        undoMenu->clear();

        if (operationHistory.isEmpty()) {
            QAction* empty = new QAction("Нет действий для отмены", undoMenu);
            empty->setEnabled(false);
            undoMenu->addAction(empty);
            return;
        }

        // От последнего действия к первому
        for (int i = operationHistory.size() - 1; i >= 0; --i) {
            QString label = QString("%1. %2")
                                .arg(i + 1)
                                .arg(operationHistory[i]);

            QAction* action = new QAction(label, undoMenu);
            connect(action, &QAction::triggered, this, [=]() {
                while (operationHistory.size() > i + 1)
                    undo();
            });

            undoMenu->addAction(action);
        }

        // Начальное состояние — всегда внизу
        QAction* initialAction = new QAction("0. Начальное состояние", undoMenu);
        connect(initialAction, &QAction::triggered, this, [=]() {
            undoToBase();
        });
        undoMenu->addSeparator();
        undoMenu->addAction(initialAction);
    });
    functionMenu->addMenu(undoMenu);


    QAction* subtractFiveAction = new QAction(QIcon("icons/minus.svg"), tr("Вычесть 5"), this);
    connect(subtractFiveAction, &QAction::triggered, this, [=]() {
        MainWindow::applyOffset(-5);
        operationHistory.append("Вычесть 5 из всех точек");
    });
    functionMenu->addAction(subtractFiveAction);
    QAction* addFiveAction = new QAction(QIcon("icons/plus.svg"), tr("Прибавить 5"), this);
    connect(addFiveAction, &QAction::triggered, this, [=]() {
        MainWindow::applyOffset(5);
        operationHistory.append("Прибавить 5 для всех точек");
    });
    functionMenu->addAction(addFiveAction);

    QMenu *additionalMenu = menuBar()->addMenu(tr("Дополнительно"));
    QAction* fullView = new QAction(QIcon("icons/align.svg"), tr("Вписать в окно"), this);
    connect(fullView, &QAction::triggered, this, [=]() {
        plotWidget->fitToView();
    });
    additionalMenu->addAction(fullView);
    QAction* measureAction = new QAction(QIcon("icons/measurement.svg"), "Измерить расстояние", this);
    measureAction->setCheckable(true);
    connect(measureAction, &QAction::toggled, this, [=](bool checked) {
        plotWidget->setMeasuring(checked);
    });
    connect(plotWidget, &PlotWidget::escPressed, this, [=]() {
        if (measureAction->isChecked())
            measureAction->setChecked(false);
    });
    additionalMenu->addAction(measureAction);

    QAction* helpAction = new QAction(QIcon("icons/help.svg"), tr("Помощь"), this);
    connect(helpAction, &QAction::triggered, this, [=]() {
        QMessageBox::information(
            this,
            "Справка",
            "Описание горячих клавиш и возможностей графика:\n\n"
            "↑ ↓ ← → или драг/дроп мышью — перемещение графика.\n"
            "Колесо мыши — масштаб.\n"
            "При движениях и масштабировании часть графика всегда остается в области видимости.\n"
            "Если будем оставлять весь график в области видимости, то нельзя будет увеличить фрагмнет графика.\n"
            "Под курсором отображается координата x и значение функции.\n"
            "Правая кнопка мыши — измерение расстояний.\n"
            "Меню — применение алгоритмов к функции.\n"
            "ESC — выход из режима измерения."
            ,
            QMessageBox::Ok
            );
    });
    additionalMenu->addAction(helpAction);
    helpAction->setIconVisibleInMenu(true);

    QToolBar* toolbar = addToolBar("Основная панель");
    toolbar->addAction(subtractFiveAction);
    toolbar->addAction(addFiveAction);
    toolbar->addAction(undoAction);
    toolbar->addAction(fullView);
    toolbar->addAction(measureAction);
    toolbar->addAction(helpAction);
    toolbar->setIconSize(QSize(32, 32));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void MainWindow::applyOffset(double offset)
{
    undoAction->setEnabled(true);
    plotWidget->applyAlgorithm([offset](QVector<QPointF> &data) {
        for (QPointF &point : data)
            point.setY(point.y() + offset);
    });
}

void MainWindow::undo()
{
    if(!operationHistory.empty()) {
        plotWidget->undo();
        operationHistory.removeLast();
    }
    if(operationHistory.empty()) {
        undoAction->setEnabled(false);
    }
}

void MainWindow::undoToBase()
{
    undoAction->setEnabled(false);
    while(!operationHistory.empty()) {
        plotWidget->undo();
        operationHistory.removeLast();
    }
}
