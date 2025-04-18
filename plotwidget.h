// plotwidget.h
#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QStack>
#include <functional>

class PlotWidget : public QWidget
{
    Q_OBJECT
signals:
    void escPressed();

public:
    PlotWidget(QWidget *parent = nullptr);
    void applyAlgorithm(const std::function<void(QVector<QPointF>&)>& algorithm);
    void undo();
    void setMeasuring(bool enabled);
    void fitToView();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVector<QPointF> data;
    QStack<QVector<QPointF>> history;
    QPoint lastMousePos;
    QPoint lastFigurePoint;
    double scale;
    double minx;
    double maxx;
    QPointF offset;
    bool dragging;
    bool measuring = false;
    QVector<QPointF> measurePoints;
    bool hasFirstPoint = false;

    double evalFunc(double x) const;
    QPoint worldToScreen(const QPointF& world) const;
    QPointF screenToWorld(const QPoint &pos) const;
    double getWorldX(double x) const;
    void drawAxes(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawFunction(QPainter &painter);
    void drawCursorInfo(QPainter &painter, const QPoint &pos);
    void drawMeasurement(QPainter &painter);
    void zoomAt(const QPoint& screenPos, double zoomFactor);
    void ensureVisible(const QPointF& worldPoint);
};

#endif // PLOTWIDGET_H
