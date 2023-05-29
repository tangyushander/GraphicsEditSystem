#ifndef DRAW_H
#define DRAW_H
#include <QPen>
#include <QPoint>
#include "math.h"
#include <QPainter>
#include <QMouseEvent>
#endif // DRAW_H


double Distance(int x, int y, int x0, int y0)
{
    return sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
}

class Handle
{
public:
    virtual void mousePressEvent(QMouseEvent *event) = 0;
};

Handle* pt = nullptr;

class PointHandle :public Handle
{
    int curX, curY;
public:
    PointHandle(){}
    virtual void mousePressEvent(QMouseEvent *event)
    {
        QPainter painter;
        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);
        curX = event->pos().x();
        curY = event->pos().y();
        painter.drawPoint(curX,curY);
    };
};

class LineHandle :public Handle
{
    int step;
    int curX, curY;
    int lastX, lastY;
public:
    LineHandle() :step(0) {}
    virtual void mousePressEvent(QMouseEvent *event)
    {
        QPainter painter;
        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);
        if (step == 0)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            lastX = curX;
            lastY = curY;
            step = 1;
        }
        else if (step == 1)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            painter.drawLine(lastX,lastY,curX,curY);
            step = 2;
        }
    };
};


class CircleHandle :public Handle
{
    int step;
    int curX, curY;
    int lastX, lastY;
public:
    CircleHandle() :step(0){}
    virtual void mousePressEvent(QMouseEvent *event)
    {
        QPainter painter;
        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);
        if (step == 0)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            lastX = curX;
            lastY = curY;
            step = 1;
        }
        else if (step == 1)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            double r = Distance(lastX, lastY, curX, curX);
            painter.drawEllipse(curX,curY,r,r);
            step = 2;
        }
    }
};

class RectangleHandle :public Handle
{
    int step;
    int curX, curY;
    int lastX, lastY;
public:
    RectangleHandle() :step(0) {}
    virtual void mousePressEvent(QMouseEvent *event)
    {
        QPainter painter;
        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);
        if (step == 0)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            lastX = curX;
            lastY = curY;
            step = 1;
        }
        else if (step == 1)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawRect(lastX,lastY,curX,curY);
            step = 2;
        }
    }
};

class SectorHandle :public Handle
{
    int step;
    int curX, curY;
public:
    SectorHandle() :step(0) {}
    virtual void mousePressEvent(QMouseEvent *event)
    {
        QPainter painter;
        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);
        if (step == 0)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            step = 1;
        }
        else if (step == 1)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            step = 2;
        }
        else if (step == 2)
        {
            curX = event->pos().x();
            curY = event->pos().y();
            painter.drawPoint(curX,curY);
            step = 3;
        }
    }
};

class PolygonHandle :public Handle
{
    int step;
    int x0, y0;
    int curX, curY;
    int lastX, lastY;
public:
    PolygonHandle() :step(0) {}
    virtual void mousePressEvent(QMouseEvent *event)
    {
        QPainter painter;
        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);
        curX = event->pos().x();
        curY = event->pos().y();
        if (step == 0)
        {
            painter.drawPoint(curX,curY);
            lastX = curX;
            lastY = curY;
            x0 = curX;
            y0 = curY;
            step++;
        }
        else if (curX < x0-5 || curX > x0+5 || curY < y0-5 || curY > y0+5)
        {
            painter.drawLine(lastX,lastY,curX,curY);
            lastX = curX;
            lastY = curY;
            step++;
        }
        else
        {
            painter.drawPoint(curX,curY);
            painter.drawLine(lastX,lastY,curX,curY);
            painter.drawLine(curX,curY,x0,y0);
            step = 0;
        }
    }
};
