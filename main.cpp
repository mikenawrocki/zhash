#include <QtGui/QApplication>
#include "zhash.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    zHash w;
    w.show();

    return a.exec();
}
