// plotwidget.cpp
#include "plotwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QtMath>

PlotWidget::PlotWidget(QWidget *parent)
    : QWidget(parent), scale(1.0), offset(0, 0), dragging(false)
{
    for (int i = 0; i <= 100; ++i) {
        double x = i / 10.0;
        data.append(QPointF(x, qSin(x)));
    }
    minx = 0;
    maxx = 100 / 10.0;
    scale = 50;
    history.push(data);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    //figureConstr =
}

void PlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    drawGrid(painter);
    drawAxes(painter);
    drawFunction(painter);
    drawCursorInfo(painter, mapFromGlobal(QCursor::pos()));
    drawMeasurement(painter);
}

void PlotWidget::drawAxes(QPainter &painter)
{
    QPen graphPen(Qt::gray, 2);
    painter.setPen(graphPen);

    painter.drawLine(0, offset.y(), width(), offset.y());
    painter.drawLine(offset.x(), 0, offset.x(), height());
}

void PlotWidget::drawGrid(QPainter &painter)
{
    painter.setPen(QColor(220, 220, 220)); // светло-серый
    const double step = 1.0;

    double xMin = -offset.x() / scale;
    double xMax = (width()  - offset.x()) / scale;
    for (int i = std::ceil(xMin); i <= std::floor(xMax); ++i) {
        double x = i * scale + offset.x();
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    }

    double yMin = -offset.y() / scale;
    double yMax = (height() - offset.y()) / scale;
    for (int j = std::ceil(yMin); j <= std::floor(yMax); ++j) {
        double y = j * scale + offset.y();
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    }

    painter.setPen(QColor(100, 100, 100)); // светло-серый

    QPointF topLeft = screenToWorld(QPoint(0, 0));
    QPointF bottomRight = screenToWorld(QPoint(width(), height()));

    int xStart = std::floor(topLeft.x());
    int xEnd = std::ceil(bottomRight.x());
    int yStart = std::floor(bottomRight.y());  // нижняя граница
    int yEnd = std::ceil(topLeft.y());         // верхняя граница

    // Подписи по X (вдоль нижней границы)
    int yPos = height() - 2; // немного выше нижнего края
    for (int x = xStart; x <= xEnd; ++x) {
        QPointF screen = worldToScreen(QPointF(x, 0));
        if (screen.x() >= 0 && screen.x() <= width())
            painter.drawText(screen.x() + 2, yPos, QString::number(x));
    }

    // Подписи по Y (вдоль левой границы)
    int xPos = 2; // немного правее левого края
    for (int y = yStart; y <= yEnd; ++y) {
        QPointF screen = worldToScreen(QPointF(0, y));
        if (screen.y() >= 0 && screen.y() <= height())
            painter.drawText(xPos, screen.y() - 2, QString::number(y));
    }}

double PlotWidget::evalFunc(double x) const {
    if (x < minx) {
        x = minx;
    }
    if (x > maxx) {
        x = maxx;
    }
    int x1 = std::floor(x * 10);
    int x2 = std::ceil(x * 10);
    double t = x * 10 - std::floor(x * 10);
    if (x2 < 0 || x1 > data.size() - 1){
        return NAN;
    }
    if (x1 < 0) {
        return data[x2].y();
    }
    if (x2 > data.size() - 1) {
        return data[x1].y();
    }
    return data[x1].y() * (1 - t) + data[x2].y() * t;
}

void PlotWidget::drawFunction(QPainter &painter)
{
    QPen graphPen(Qt::blue, 2);  // синий цвет, толщина 2
    painter.setPen(graphPen);
    for (int i = 1; i < data.size(); ++i) {
        QPointF p1 = data[i - 1];
        QPointF p2 = data[i];
        QPoint screenP1 = QPoint((p1.x() * scale) + offset.x(),  -(p1.y() * scale) + offset.y());
        QPoint screenP2 = QPoint((p2.x() * scale) + offset.x(),  -(p2.y() * scale) + offset.y());
        painter.drawLine(screenP1, screenP2);
    }
}

void PlotWidget::fitToView()
{
    if (data.isEmpty())
        return;

    // 1. Найти границы графика
    double minX = data[0].x(), maxX = data[0].x();
    double minY = data[0].y(), maxY = data[0].y();

    for (const QPointF& p : data) {
        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }

    // 2. Размер графика в мировых координатах
    double worldWidth = (maxX - minX);
    double worldHeight = (maxY - minY);

    if (worldWidth == 0) worldWidth = 1;
    if (worldHeight == 0) worldHeight = 1;

    // 3. Задать небольшой отступ от краёв
    double margin = 40.0;

    // 4. Рассчитать масштаб
    double scaleX = (width() - margin * 2) / worldWidth;
    double scaleY = (height() - margin * 2) / worldHeight;
    scale = std::min(scaleX, scaleY);

    // 5. Центр графика (в мировых координатах)
    QPointF worldCenter((minX + maxX) / 2.0, (minY + maxY) / 2.0);

    // 6. Центр экрана (в пикселях)
    QPointF screenCenter(width() / 2.0, height() / 2.0);

    // 7. Смещение: куда сдвинуть, чтобы центр мира оказался в центре экрана
    offset = screenCenter - QPointF(worldCenter.x() * scale, -worldCenter.y() * scale);

    update();
}

void PlotWidget::drawCursorInfo(QPainter &painter, const QPoint &pos)
{
    double x = getWorldX(pos.x());
    if (x < minx) {
        x = minx;
    }
    if (x > maxx) {
        x = maxx;
    }
    double v = evalFunc(x);
    QString text = QString("(%1, %2)")
                       .arg(x, 0, 'f', 2)
                       .arg(v, 0, 'f', 2);
    painter.setPen(Qt::black);
    painter.drawText(pos.x(), pos.y() + 35, text);
    // text = QString("(%1, %2)")
    //                    .arg(pos.x())
    //                    .arg(pos.y());
    // painter.setPen(Qt::black);
    // painter.drawText(10, 20, text);
    painter.setBrush(Qt::red);
    painter.setPen(Qt::black);
    if (abs(v) < 1000000) {
        lastFigurePoint = worldToScreen(QPointF(x, v));
    }
    int radius = 5;
    painter.drawEllipse(lastFigurePoint, radius, radius);
    // text = QString("(%1, %2)")
    //            .arg(lastFigurePoint.x())
    //            .arg(lastFigurePoint.y());
    // painter.setPen(Qt::black);
    // painter.drawText(10, 40, text);
}

void PlotWidget::setMeasuring(bool enabled) {
    measuring = enabled;
    hasFirstPoint = false;
    update();
}

QPoint PlotWidget::worldToScreen(const QPointF& world) const {
    double x = world.x() * scale + offset.x();
    double y = -world.y() * scale + offset.y();
    return QPoint(static_cast<int>(x), static_cast<int>(y));
}

QPointF PlotWidget::screenToWorld(const QPoint &pos) const
{
    double x = (pos.x() - offset.x()) / scale;
    double y = -(pos.y() - offset.y()) / scale;
    return QPointF(x, y);
}

double PlotWidget::getWorldX(double x) const {
    return (x - offset.x()) / scale;
}

void PlotWidget::drawMeasurement(QPainter &painter)
{
    if (measuring) {
        for (QPointF p : measurePoints) {
            painter.setBrush(Qt::yellow);
            painter.setPen(Qt::black);
            QPoint center = worldToScreen(p);
            int radius = 5;
            painter.drawEllipse(center, radius, radius);
        }
        if (measurePoints.size() == 2) {
            QPoint p1 = worldToScreen(measurePoints[0]);
            QPoint p2 = worldToScreen(measurePoints[1]);
            QPen graphPen(Qt::red, 2);
            painter.setPen(graphPen);
            painter.drawLine(p1, p2);

            double dx = measurePoints[1].x() - measurePoints[0].x();
            double dy = measurePoints[1].y() - measurePoints[0].y();
            double dist = std::sqrt(dx * dx + dy * dy);
            painter.setPen(Qt::black);
            QFont font = painter.font();
            font.setPointSize(14);
            painter.setFont(font);
            QString distText = QString("Distance: %1").arg(dist, 0, 'f', 2);
            painter.drawText((p1 + p2) / 2, distText);
        }
    }
}

void PlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        offset += event->pos() - lastMousePos;
    }
    lastMousePos = event->pos();
    update();
}

void PlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        lastMousePos = event->pos();
    } else if (event->button() == Qt::RightButton && measuring) {
        if (measurePoints.size() == 2) {
            measurePoints.clear();
        }
        double x = getWorldX(event->pos().x());
        double y = evalFunc(x);
        if (y != NAN) {
            measurePoints.push_back(QPointF(x, y));
            if (measurePoints.size() == 2) {
                update();
            }
        }
    }
}

void PlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
    }
    ensureVisible(lastFigurePoint);
    update();
}

void PlotWidget::zoomAt(const QPoint& screenPos, double zoomFactor)
{
    // 1. Переводим позицию курсора из экрана в мир ДО изменения масштаба
    QPointF beforeZoomWorld = screenToWorld(screenPos);

    // 2. Меняем масштаб
    scale *= zoomFactor;

    // 3. Переводим ту же экранную точку ПОСЛЕ изменения масштаба
    QPointF afterZoomWorld = worldToScreen(beforeZoomWorld);

    // 4. Компенсируем смещением: offset += (before - after)
    offset += screenPos - afterZoomWorld;
    update();
}

void PlotWidget::wheelEvent(QWheelEvent *event)
{
    zoomAt(lastMousePos, (event->angleDelta().y() > 0) ? 1.1 : 0.90909);
    ensureVisible(lastFigurePoint);
    update();
}

void PlotWidget::keyPressEvent(QKeyEvent *event)
{
    int delta = 10;
    switch (event->key()) {
    case Qt::Key_Left:
        offset.rx() += delta;
        break;
    case Qt::Key_Right:
        offset.rx() -= delta;
        break;
    case Qt::Key_Up:
        offset.ry() += delta;
        break;
    case Qt::Key_Down:
        offset.ry() -= delta;
        break;
    case Qt::Key_Escape:
        emit escPressed();
        break;
    }
    ensureVisible(lastFigurePoint);
    update();
}

void PlotWidget::ensureVisible(const QPointF& screenPoint)
{
    const int margin = 40; // небольшой отступ от границ окна

    // Смещения, которые нужно применить, чтобы точка оказалась в пределах виджета
    double dx = 0;
    double dy = 0;

    if (screenPoint.x() < margin)
        dx = margin - screenPoint.x();
    else if (screenPoint.x() > width() - margin)
        dx = width() - margin - screenPoint.x();

    if (screenPoint.y() < margin)
        dy = margin - screenPoint.y();
    else if (screenPoint.y() > height() - margin)
        dy = height() - margin - screenPoint.y();

    offset += QPointF(dx, dy);
}

void PlotWidget::applyAlgorithm(const std::function<void(QVector<QPointF>&)> &algorithm)
{
    measurePoints.clear();
    history.push(data);
    algorithm(data);
    fitToView();
    update();
}

void PlotWidget::undo()
{
    if (history.size() > 1) {
        measurePoints.clear();
        data = history.pop();
        fitToView();
        update();
    }
}
