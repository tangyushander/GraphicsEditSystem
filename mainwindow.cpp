#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStatusBar>
#include <QLabel>
#include <QPoint>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsItemGroup>
#include <QLine>
#include <QGraphicsView>
#include <QColor>
#include <QColorDialog>
#include <QMessageBox>
#include <QVector>
#include <QPointF>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDataStream>
#include <QList>
#include <QMap>
#include <QToolBar>
#include <math.h>

#define PI 3.1415926


template<class T>
void setBrushColor(T *item)
{//函数模板
    QColor color = item->brush().color();
    color = QColorDialog::getColor(color,NULL,"选择填充颜色");
    if (color.isValid())
        item->setBrush(QBrush(color));
}

//计算任意多边形的面积，顶点按照顺时针或者逆时针方向排列
double ComputePolygonArea(const QVector<QPointF> &points)
{
    int point_num = points.size();
    if (point_num < 3) return 0.0;
    double s = points[0].y() * (points[point_num-1].x() - points[1].x());
    for(int i = 1; i < point_num; ++i)
        s += points[i].y() * (points[i-1].x() - points[(i+1)%point_num].x());
    return fabs(s/2.0);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置初始状态
    cur_status = Files;
    isSaved = false;
    // 设置状态栏
    QStatusBar *sBar = statusBar();
    coordinate_label.setParent(this);
    coordinate_label.setMinimumWidth(150);
    shapeinfo_label.setParent(this);
    shapeinfo_label.setMinimumWidth(200);
    sBar->addPermanentWidget(&coordinate_label);
    sBar->addPermanentWidget(&shapeinfo_label);

    // 设置工具栏
    QToolBar *tBar1 = addToolBar("文件工具栏");
    tBar1->addAction(ui->actionNew);
    tBar1->addSeparator();
    tBar1->addAction(ui->actionOpen);
    tBar1->addSeparator();
    tBar1->addAction(ui->actionSave);

    QToolBar *tBar2 = addToolBar("绘图工具栏");
    tBar2->addSeparator();
    tBar2->addAction(ui->actionLine);
    tBar2->addSeparator();
    tBar2->addAction(ui->actionRectangle);
    tBar2->addSeparator();
    tBar2->addAction(ui->actionEllipse);
    tBar2->addSeparator();
    tBar2->addAction(ui->actionPolygon);
    tBar2->addSeparator();
    tBar2->addAction(ui->actionCurve);
    tBar2->addSeparator();
    tBar2->addAction(ui->action_broken_line);
    QToolBar *tBar3 = addToolBar("编辑工具栏");
    tBar3->addAction(ui->actionSelect);
    tBar3->addSeparator();
    tBar3->addAction(ui->actionFill);
    tBar3->addSeparator();
    tBar3->addAction(ui->actionTranslate);
    tBar3->addSeparator();
    tBar3->addAction(ui->actionZoomin);
    tBar3->addSeparator();
    tBar3->addAction(ui->actionZoomout);
    tBar3->addSeparator();
    tBar3->addAction(ui->actionDelete);


    ui->actionLine->setIcon(QIcon(":/image/images/line.png"));
    ui->actionCurve->setIcon(QIcon(":/image/images/pen.png"));
    ui->actionEllipse->setIcon(QIcon(":/image/images/oval.png"));
    ui->actionRectangle->setIcon(QIcon(":/image/images/rect.png"));
    ui->actionPolygon->setIcon(QIcon(":/image/images/poly.png"));
    ui->action_broken_line->setIcon(QIcon(":/image/images/brokenline.jpeg"));
    ui->actionSelect->setIcon(QIcon(":/image/images/select.jpeg"));
    ui->actionTranslate->setIcon(QIcon(":/image/images/move.png"));
    ui->actionFill->setIcon(QIcon(":/image/images/fill.png"));
    ui->actionDelete->setIcon(QIcon(":/image/images/delete.png"));
    ui->actionNew->setIcon(QIcon(":/image/images/new.jpeg"));
    ui->actionOpen->setIcon(QIcon(":/image/images/open.jpeg"));
    ui->actionSave->setIcon(QIcon(":/image/images/save.jpeg"));
    ui->actionZoomin->setIcon(QIcon(":/image/images/widen.png"));
    ui->actionZoomout->setIcon(QIcon(":/image/images/narrow.png"));
    // 创建场景
    scene = new QGraphicsScene(-300,-200,600,200);
    ui->graphicsView->setScene(scene);

    // 设置鼠标样式
    ui->graphicsView->setMouseTracking(true);

    // 信号与槽
    connect(ui->graphicsView, &MyGraphicsview::mouseMove, this, &MainWindow::mouseMove_slot);
    connect(ui->graphicsView, &MyGraphicsview::mousePressed, this, &MainWindow::mousePressed_slot);
    connect(ui->graphicsView, &MyGraphicsview::mouseReleased, this, &MainWindow::mouseReleased_slot);
    connect(ui->graphicsView, &MyGraphicsview::mouseDoubleClick, this, &MainWindow::mouseDoubleClick_slot);

    connect(ui->actionLine, &QAction::triggered,
            [=]()
            {
                cur_status = DrawLine;
                shapeinfo_label.setText("");
                start_point = end_point = QPointF();
            }
            );
    connect(ui->actionLine, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Draw);
            }
            );
    connect(ui->actionRectangle, &QAction::triggered,
            [=]()
            {
                cur_status = DrawRect;
                shapeinfo_label.setText("");
                start_point = end_point = QPointF();
            }
            );
    connect(ui->actionRectangle, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Draw);
            }
            );
    connect(ui->actionEllipse, &QAction::triggered,
            [=]()
            {
                cur_status = DrawEllipse;
                shapeinfo_label.setText("");
                start_point = end_point = QPointF();
            }
            );
    connect(ui->actionEllipse, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Draw);
            }
            );
    connect(ui->actionPolygon, &QAction::triggered,
            [=]()
            {
                cur_status = DrawPolygon;
                shapeinfo_label.setText("");
                start_point = end_point = QPointF();
                points.clear();
            }
            );
    connect(ui->actionPolygon, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Draw);
            }
            );
    connect(ui->actionCurve, &QAction::triggered,
            [=]()
            {
                cur_status = DrawCurve;
                shapeinfo_label.setText("");
                start_point = end_point = QPointF();
            }
            );
    connect(ui->actionCurve, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Draw);
            }
            );

    connect(ui->action_broken_line, &QAction::triggered,
            [=]()
            {
                cur_status = DrawBrokenLine;
                shapeinfo_label.setText("");
                start_point = end_point = QPointF();
                points.clear();
            }
            );
    connect(ui->action_broken_line, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Draw);
            }
            );

    connect(ui->actionSelect, &QAction::triggered,
            [=]()
            {
                cur_status = Select;
            }
            );
    connect(ui->actionSelect, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Edit);
            }
            );
    connect(ui->actionFill, &QAction::triggered,
            [=]()
            {
                cur_status = Fill;
            }
            );
    connect(ui->actionFill, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Edit);
            }
            );
    connect(ui->actionTranslate, &QAction::triggered,
            [=]()
            {
                cur_status = Translate;
            }
            );
    connect(ui->actionTranslate, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Edit);
            }
            );
    connect(ui->actionDelete, &QAction::triggered,
            [=]()
            {
                cur_status = Delete;
            }
            );
    connect(ui->actionDelete, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(Edit);
            }
            );

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::actNew_triggered_slot);
    connect(ui->actionNew, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(File);
            }
            );
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::actSave_triggered_slot);
    connect(ui->actionSave, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(File);
            }
            );
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::actOpen_triggered_slot);
    connect(ui->actionOpen, &QAction::triggered,
            ui->graphicsView,
            [=]()
            {
                ui->graphicsView->set_cur_status(File);
            }
            );
}

MainWindow::~MainWindow()
{
    delete group;
    delete scene;
    delete ui;
}

void MainWindow::mousePressed_slot(QMouseEvent *e1) {

    if (e1->button() == Qt::LeftButton) {
        // 绘图时
        if (cur_status == DrawLine
                || cur_status == DrawRect
                || cur_status == DrawEllipse
                || cur_status == DrawPolygon
                || cur_status == DrawCurve
                || cur_status == DrawBrokenLine)
        {
            isSaved = false;
            start_point = ui->graphicsView->mapToScene(e1->pos());
            if (cur_status == DrawPolygon) {
                points.append(start_point);
            }
            if (cur_status == DrawCurve) { // 曲线
                group = new QGraphicsItemGroup; // 创建一个组合
                scene->addItem(group);
                group->setData(areaORlen_key, QVariant(0.0));
                group->setData(shape_key, "曲线");
                group->setData(group_id_key, group_id);
            }
            if (cur_status == DrawBrokenLine) {
                if (points.isEmpty()) { // 点下第一个点时
                    group = new QGraphicsItemGroup; // 创建一个组合
                    scene->addItem(group);
                    group->setData(areaORlen_key, QVariant(0.0));
                    group->setData(shape_key, "折线段");
                    group->setData(group_id_key, group_id);
                }
                points.append(start_point);
            }
        }
        // 编辑时
        else if (cur_status == Select
                 || cur_status == Fill
                 || cur_status == Translate
                 || cur_status == Delete
                 || cur_status==Zoomin
                 || cur_status==Zoomout)
        {
            isSaved = false;
            // 先选中
            QPointF pointScene = ui->graphicsView->mapToScene(e1->pos()); //转换到Scene坐标
            QGraphicsItem *item = NULL;
            item = scene->itemAt(pointScene, ui->graphicsView->transform()); //获取光标下的绘图项
            if (item) {
                if (item->group()) {
                    item = item->group();
                }
                // 状态栏显示长度/面积
                item->setCursor(Qt::SizeAllCursor); // 改变鼠标样式

                QString shapeinfo = "面积";
                if (item->data(shape_key).toString() == "直线"
                        || item->data(shape_key).toString() == "曲线"
                        || item->data(shape_key).toString() == "折线")
                    shapeinfo = "长度";
                QString str;
                if (item->group()) str = item->group()->data(areaORlen_key).toString();
                else str = item->data(areaORlen_key).toString();
                shapeinfo_label.setText(QString("选中对象: %1   " + shapeinfo + ": %2    ").arg(item->data(shape_key).toString()).arg(str));
            } else {
                shapeinfo_label.setText("");
                return;
            }

            if (cur_status == Fill) { // 填充
                switch (item->type())  //绘图项的类型
                {
                      case QGraphicsRectItem::Type: //矩形框
                      {
                        QGraphicsRectItem *theItem = qgraphicsitem_cast<QGraphicsRectItem*>(item);
                        setBrushColor(theItem);
                        break;
                      }
                      case QGraphicsEllipseItem::Type: // 椭圆
                      {
                        QGraphicsEllipseItem *theItem;
                        theItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
                        setBrushColor(theItem);
                        break;
                      }
                      case QGraphicsPolygonItem::Type: //多边形
                      {
                        QGraphicsPolygonItem *theItem = qgraphicsitem_cast<QGraphicsPolygonItem*>(item);
                        setBrushColor(theItem);
                        break;
                      }
                      case QGraphicsLineItem::Type: //直线，设置线条颜色
                      {
                        QGraphicsLineItem *theItem = qgraphicsitem_cast<QGraphicsLineItem*>(item);
                        QPen pen = theItem->pen();
                        QColor color = theItem->pen().color();
                        color = QColorDialog::getColor(color,this,"选择线条颜色");
                        if (color.isValid())
                        {
                            pen.setColor(color);
                            theItem->setPen(pen);
                        }
                        break;
                      }
                     case QGraphicsItemGroup::Type:
                     {
                        QGraphicsItemGroup *thegroup = qgraphicsitem_cast<QGraphicsItemGroup*>(item);
                        QColor color = QColorDialog::getColor(color,this,"选择线条颜色");
                        QList<QGraphicsItem *>	items_list = scene->items();
                        for (int i = 0; i < items_list.size(); i++) {
                            if (items_list[i]->type() == QGraphicsLineItem::Type
                                    && items_list[i]->data(group_id_key).toInt() == thegroup->data(group_id_key).toInt())
                            {
                                QGraphicsLineItem *theItem = qgraphicsitem_cast<QGraphicsLineItem*>(items_list[i]);
                                QPen pen = theItem->pen();
                                if (color.isValid())
                                {
                                    pen.setColor(color);
                                    theItem->setPen(pen);
                                }
                            }
                        }
                        break;
                     }
                }
                scene->clearSelection();
            }
            else if (cur_status == Zoomin) {
                item->setPos(QPointF(item->x() + 10, item->y() + 10));
                item->update();
            }
            else if (cur_status == Zoomout) {
                item->setPos(QPointF(item->x() - 10, item->y() - 10));
                item->update();
            }
            else if (cur_status == Translate) {  // 平移
                // 平移模态对话框
                tDialog = new TranslateDialog(this);
                // 接受信号，包含输入的平移量
                connect(tDialog, &TranslateDialog::translate_value,
                        [=](float x, float y)
                        {

                            item->setPos(QPointF(item->x() + x, item->y() + y));

                            //item->rect().setX(item->x() + x);

                        }
                        );
                tDialog->exec();
            }

            else if (cur_status == Delete) {  // 删除
                // 删除所选中的绘图项
                int cnt = scene->selectedItems().count();
                if (cnt > 0)
                for (int i = 0; i < cnt; i++) {
                    QGraphicsItem *item = scene->selectedItems().at(0);
                    scene->removeItem(item);
                }
            }
            scene->clearSelection();
        }
        else {  // 其他选项
            shapeinfo_label.setText("");
        }
    }
    else if (e1->button() == Qt::RightButton) { // 鼠标右击 画多边形
        if (cur_status == DrawPolygon) {
            start_point = ui->graphicsView->mapToScene(e1->pos());
            points.append(start_point);
            for (auto e1 : points) qDebug() << e1.x() << e1.y() << endl;

            // 画多边形
            QGraphicsPolygonItem *item = new QGraphicsPolygonItem(QPolygonF(points));
            item->setFlags(QGraphicsItem::ItemIsMovable
                           | QGraphicsItem::ItemIsSelectable
                           | QGraphicsItem::ItemIsFocusable);
            double area = ComputePolygonArea(points);
            item->setData(areaORlen_key, area);
            item->setData(shape_key, "多边形");
            scene->addItem(item);
            // 清空points容器，便于下一个多边形绘制
            points.clear();
        }
        else if (cur_status == DrawBrokenLine) {
            start_point = ui->graphicsView->mapToScene(e1->pos());
            points.append(start_point);
            // 画折线
            for (int i = 0; i < points.size() - 1; i++) {
                QGraphicsLineItem *line_item = new QGraphicsLineItem(QLineF(points[i], points[i + 1]));//x,y 为左上角的图元局部坐标，图元中心点为0,0
                line_item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

                QPen pen;
                pen.setWidth(2);
                line_item->setPen(pen);
                line_item->setSelected(false);
                line_item->clearFocus();
                line_item->setData(shape_key, "折线中直线");
                line_item->setData(group_id_key, group_id);
                // 计算长度
                double len = sqrt(pow((points[i].x() - points[i + 1].x()), 2) + pow((points[i].y() - points[i + 1].y()), 2));
                line_item->setData(areaORlen_key, len);
                group->setData(areaORlen_key, QVariant(group->data(areaORlen_key).toDouble() + len));
                // 加入到组合中
                group->addToGroup(qgraphicsitem_cast<QGraphicsItem*>(line_item));
            }
            points.clear();
            // 设置可选可移动
            group->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            group->setSelected(false);
            group_id++;
        }
    }
}

void MainWindow::mouseMove_slot(QMouseEvent *e1) { // 鼠标移动过程中状态栏显示坐标

    QPointF point = ui->graphicsView->mapToScene(e1->pos());
    coordinate_label.setText(QString("坐标：(%1, %2)").arg(point.x()).arg(point.y()));
    if (cur_status == DrawLine
            || cur_status == DrawRect
            || cur_status == DrawEllipse
            || cur_status == DrawPolygon
            || cur_status == DrawCurve
            || cur_status == DrawBrokenLine)
    {
        // 设置鼠标样式
        ui->graphicsView->setCursor(Qt::CrossCursor);
    }
    if ((e1->buttons() & Qt::LeftButton) && cur_status == DrawCurve) { // 画曲线
        end_point = ui->graphicsView->mapToScene(e1->pos());
        QGraphicsLineItem *line_item = new QGraphicsLineItem(QLineF(start_point, end_point));//x,y 为左上角的图元局部坐标，图元中心点为0,0
        line_item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

        QPen pen;
        pen.setWidth(2);
        line_item->setPen(pen);
        line_item->setSelected(false);
        line_item->clearFocus();
        line_item->setData(shape_key, "曲线中直线");
        line_item->setData(group_id_key, group_id);


        // 计算长度
        double len = sqrt(pow((start_point.x() - end_point.x()), 2) + pow((start_point.y() - end_point.y()), 2));
        line_item->setData(areaORlen_key, len);
        group->setData(areaORlen_key, QVariant(group->data(areaORlen_key).toDouble() + len));
        scene->addItem(line_item);
        // 保存直线
        group->addToGroup(qgraphicsitem_cast<QGraphicsItem*>(line_item));
        start_point = end_point;
    }
}

void MainWindow::mouseReleased_slot(QMouseEvent *e1) {  // 鼠标释放时绘制图形

    if (e1->button() == Qt::LeftButton) {
        // 获取鼠标释放时的坐标
        end_point = ui->graphicsView->mapToScene(e1->pos());
        switch(cur_status) {
        case DrawLine:  // 绘制直线
        {
            QGraphicsLineItem *line_item = new QGraphicsLineItem(QLineF(start_point, end_point));//x,y 为左上角的图元局部坐标，图元中心点为0,0
            line_item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            QPen pen;
            pen.setWidth(2);
            line_item->setPen(pen);
            // 计算长度
            double len = sqrt(pow((start_point.x() - end_point.x()), 2) + pow((start_point.y() - end_point.y()), 2));
            line_item->setData(areaORlen_key, QVariant(len));
            line_item->setData(shape_key, "直线");
            scene->addItem(line_item);
            break;
        }
        case DrawRect:  // 绘制矩形
        {
            // 保持start_point在左上方
            if (start_point.x() > end_point.x()) {
                QPointF tmp = start_point;
                start_point = end_point;
                end_point = tmp;
            }
            QGraphicsRectItem *rect_item = new QGraphicsRectItem(QRectF(start_point, end_point));//x,y 为左上角的图元局部坐标，图元中心点为0,0
            rect_item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            double area = (end_point.x() - start_point.x()) * (end_point.y() - start_point.y());
            rect_item->setData(areaORlen_key, QVariant(area));
            rect_item->setData(shape_key, "矩形");
            scene->addItem(rect_item);
            break;
        }
        case DrawEllipse:  // 绘制椭圆
        {
            // 保持start_point在左上方
            if (start_point.x() > end_point.x()) {
                QPointF tmp = start_point;
                start_point = end_point;
                end_point = tmp;
            }
            QGraphicsEllipseItem *item=new QGraphicsEllipseItem(QRectF(start_point, end_point));
            item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            double area = PI * (end_point.x() - start_point.x()) * 0.5 * (end_point.y() - start_point.y()) * 0.5;
            item->setData(areaORlen_key, area);
            item->setData(shape_key, "椭圆");
            scene->addItem(item);
            break;
        }
        case DrawCurve:{
            // 设置可选可移动
            group->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            group_id++;
            break;
        }
        default:           
            break;
        }
    }
}

void MainWindow::mouseDoubleClick_slot(QMouseEvent *e) {  // 鼠标双击时显示长度/面积对话框
    if (cur_status == Select) {
        QPointF pointScene = ui->graphicsView->mapToScene(e->pos()); //转换到Scene坐标
        QGraphicsItem *item = NULL;
        item = scene->itemAt(pointScene, ui->graphicsView->transform()); //获取光标下的绘图项
        if (item) {

            QString shapeinfo = "面积";
            if (item->data(shape_key).toString() == "直线"
                    || item->data(shape_key).toString() == "曲线中直线"
                    || item->data(shape_key).toString() == "折线中直线"
                    || item->data(shape_key).toString() == "曲线"
                    || item->data(shape_key).toString() == "折线")
                shapeinfo = "长度";
            QString str;
            if (item->group()) str = item->group()->data(areaORlen_key).toString();
            else str = item->data(areaORlen_key).toString();
            QMessageBox::information(this, shapeinfo, str);
        }
    }
}

void MainWindow::on_actionZoomout_triggered()
{

}

void MainWindow::on_actionZoomin_triggered()
{

}
void MainWindow::actZoomout_triggered_slot()
{

}

void MainWindow::actSave_triggered_slot()
{

}

bool MainWindow::save(QString &FileName)
{

}

void MainWindow::actOpen_triggered_slot()
{

}

bool MainWindow::open(QString &FileName)
{

}

void MainWindow::actNew_triggered_slot()
{

}

void MainWindow::closeEvent(QCloseEvent *)
{

}

