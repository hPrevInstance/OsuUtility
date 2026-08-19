/**
 * @file    MainWindow.cpp
 * @brief   主窗口实现
 */

#include "app/MainWindow.h"
#include "ui_MainWindow.h"

#include "adapters/QProcessRunner.hpp"
#include "core/Fs.hpp"
#include "core/speed/AudioProcess.hpp"
#include "services/SpeedService.hpp"

#include <QtConcurrent/QtConcurrent>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QPushButton>

#include <exception>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    connect(ui_->processButton, &QPushButton::clicked, this, &MainWindow::onProcessClicked);
}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::onProcessClicked()
{
    const QString file = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择 osu 谱面"), QString(), QStringLiteral("osu 谱面 (*.osu)"));
    if (file.isEmpty())
    {
        return;
    }

    ui_->processButton->setEnabled(false);
    ui_->logView->appendPlainText(QStringLiteral("开始处理：") + file);

    auto *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher]()
            {
        ui_->logView->appendPlainText(watcher->result());
        ui_->processButton->setEnabled(true);
        watcher->deleteLater(); });

    // 在后台线程执行服务，避免阻塞 UI
    watcher->setFuture(QtConcurrent::run([file]() -> QString
                                         {
        try
        {
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
            return QStringLiteral("完成：") + QString::fromStdString(out.string());
        }
        catch (const std::exception &e)
        {
            return QStringLiteral("失败：") + QString::fromUtf8(e.what());
        } }));
}
