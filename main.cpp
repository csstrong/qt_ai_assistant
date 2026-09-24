#include "mainwindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 统一使用扁平风格 + 中文字体，观感更接近前端页面
    QApplication::setStyle("Fusion");
    QFont font("Microsoft YaHei", 9);
    a.setFont(font);

    MainWindow w;
    w.show();
    return a.exec();
}
