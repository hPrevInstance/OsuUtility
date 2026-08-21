#include <QApplication>
#include <QFile>
#include "app/MainWindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
    }

    MainWindow window;
    window.show();
    return app.exec();
}