#ifndef CLOGCONSOLE_H
#define CLOGCONSOLE_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>

namespace Ui {
class CLogconsole;
}

class CLogconsole : public QWidget
{
    Q_OBJECT

public:
    explicit CLogconsole(QWidget *parent = nullptr);
    ~CLogconsole();
public slots:
    void appendLog(const QString &message);
    void openLogWindow();
private:
    Ui::CLogconsole *ui;
signals:
    void logWindowSwitch(bool isVisible);
};

#endif // CLOGCONSOLE_H
