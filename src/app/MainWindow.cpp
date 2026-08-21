/**
 * @file    MainWindow.cpp
 * @brief   主窗口实现（无边框 + 右键菜单）
 */

#include "app/MainWindow.h"
#include "ui_MainWindow.h"

#include "adapters/QProcessRunner.hpp"
#include "core/speed/AudioProcess.hpp"
#include "services/SpeedService.hpp"

#include <QApplication>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMenu>
#include <QMouseEvent>
#include <QtConcurrent/QtConcurrent>

#include <exception>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    // ---------- 1. 去除标题栏 ----------
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // ---------- 2. 设置文本框只读 ----------
    ui_->nameEdit->setReadOnly(true);
    ui_->metaEdit->setReadOnly(true);

    // ---------- 3. 不再连接按钮，因为已删除 ----------
}

MainWindow::~MainWindow()
{
    delete ui_;
}

// ---------- 4. 右键菜单事件 ----------
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *actionProcess = menu.addAction(QStringLiteral("选择谱面并处理"));
    QAction *actionExit = menu.addAction(QStringLiteral("退出"));

    QAction *selected = menu.exec(event->globalPos());
    if (selected == actionProcess) {
        onProcessAction();
    } else if (selected == actionExit) {
        QApplication::quit();
    }
}

// ---------- 5. 处理逻辑（原 onProcessClicked 改名） ----------
void MainWindow::onProcessAction()
{
    const QString file = QFileDialog::getOpenFileName(this,
                                                      QStringLiteral("选择 osu 谱面"),
                                                      QString(),
                                                      QStringLiteral("osu 谱面 (*.osu)"));
    if (file.isEmpty()) {
        return;
    }

    // 显示文件名（不含路径）到 nameEdit
    QFileInfo fileInfo(file);
    ui_->nameEdit->setPlainText(fileInfo.fileName());

    // 清空元数据框，显示“处理中...”
    ui_->metaEdit->setPlainText(QStringLiteral("处理中，请稍候..."));

    // 禁用窗口交互（可考虑通过事件过滤器，此处简单禁用鼠标，非必须）
    // setEnabled(false); // 若禁用，右键菜单也会被禁，不推荐，故省略

    auto *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher]() {
        // 显示处理结果到 metaEdit
        ui_->metaEdit->setPlainText(watcher->result());
        // setEnabled(true);
        watcher->deleteLater();
    });

    watcher->setFuture(QtConcurrent::run([file]() -> QString {
        try {
            adapters::QProcessRunner runner;
            services::SpeedService service(runner);
            services::SpeedRequest req;
#ifdef _WIN32
            req.beatmapPath = core::fs::path(file.toStdWString());
#else
            req.beatmapPath = core::fs::path(file.toStdString());
#endif
            req.outputDir = req.beatmapPath.parent_path() / "outil";
            req.tempo = "1.5";
            req.pitch.clear();
            req.mode = core::speed::TEMPO;
            auto out = service.Process(req);
            return QStringLiteral("处理成功\n输出目录：") + QString::fromStdString(out.string());
        } catch (const std::exception &e) {
            return QStringLiteral("处理失败\n错误信息：") + QString::fromUtf8(e.what());
        }
    }));
}

// ---------- 6. （可选）鼠标拖动窗口 ----------
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}