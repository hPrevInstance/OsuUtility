#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QPushButton button("我的第一个 Windows 程序");
    button.resize(300, 200);
    button.show();
    return app.exec();
}