/**
 * @file    MainWindow.h
 * @brief   主窗口（无边框，右键菜单操作）
 *
 * 无标题栏，通过右键菜单选择谱面处理或退出。
 * 界面包含两个只读文本框：谱面名称和元数据/日志。
 */

#pragma once

#include <QMainWindow>
#include <QPoint>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // 右键菜单
    void contextMenuEvent(QContextMenuEvent *event) override;
    // 鼠标拖动窗口（无边框时）
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void onProcessAction(); // 处理谱面的核心动作

private:
    Ui::MainWindow *ui_;
    QPoint m_dragPosition; // 记录拖动起始位置
};