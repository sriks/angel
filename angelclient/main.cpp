#include <QtGui/QApplication>
#include "angelclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AngelClient w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
