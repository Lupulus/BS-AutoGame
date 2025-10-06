#include <QApplication>
#include <QIcon>
#include "cgui.h"
#include "clogconsole.h"
#include "cparam.h"
#include "chelp.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CLogconsole c;
    CStart s(nullptr);
    CHelp h(nullptr);
    CParam p(nullptr, &h);

    CGui w(nullptr, &c, &p, &h);
    QIcon icon(":/img/ressources/images/icon.ico");
    w.setWindowIcon(icon);
    w.setWindowTitle("Emul-AutoGame");
    w.show();
    c.setWindowIcon(icon);
    c.setWindowTitle("Emul-AutoGame-Console");
    p.setWindowIcon(icon);
    p.setWindowTitle("Emul-AutoGame-Paramètres");
    h.setWindowIcon(icon);
    h.setWindowTitle("Emul-AutoGame-Aide");
    //c.show();
    qputenv("QT_MESSAGE_PATTERN", QByteArray("[%{type}] %{file}:%{line} - %{message}"));
    return a.exec();
}
