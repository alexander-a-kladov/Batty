#include <QApplication>
#include <mywidget.h>

int main(int argc, char **argv)
{
    QApplication app(argc,argv);
    app.setApplicationName("batty");
    MyWidget wid;
    app.setQuitOnLastWindowClosed(true);
    wid.show();
    app.exec();
    return 0;
}
