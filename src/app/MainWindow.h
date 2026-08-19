/**
 * @file    MainWindow.h
 * @brief   主窗口
 *
 * 最小可用的主窗口，演示如何异步调用 SpeedService。
 * 界面布局见 MainWindow.ui，可在 Qt Designer / Qt Creator 中直接编辑。
 */

#pragma once

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onProcessClicked();

private:
    Ui::MainWindow *ui_;
};
