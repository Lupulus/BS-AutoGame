#ifndef CGUI_H
#define CGUI_H

#include <QMainWindow>
#include <QListView>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QThread>
#include <QCloseEvent>
#include <QMessageBox>
#include <QProcess>

#include "cscenario.h"
#include "cparam.h"
#include "clogconsole.h"
#include "cstart.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CGui;
}
QT_END_NAMESPACE

class CGui : public QMainWindow
{
    Q_OBJECT

public:
    CGui(QWidget *parent = nullptr, CLogconsole *log = nullptr, CParam *Param = nullptr, CHelp *Help = nullptr, CStart *Start = new CStart());
    ~CGui();
    void debug(const QString &function, const QString &msg, const QString &level = "Info");

private:
    Ui::CGui *ui;

    QString _appPath;
    bool _AutoEmul;
    bool _AutoGame;
    int _Emulator;

    QStringListModel *_model;
    QString _selectedGame;
    QString _selectedScena;
    QVariant _alias;
    QVariant _aliasScena;

    int _aliasValue;
    int _aliasValueScena;
    int _repeatCount;

    bool _isPubliciteChecked;
    bool _isInfiniChecked;

    CScenario *_currentScenario = nullptr;
    CLogconsole *_logWindow = nullptr;
    CParam *_Param = nullptr;
    CHelp *_Help = nullptr;
    CStart *_Start;
    QThread *_startThread = new QThread();

private slots:
    //Start
    void onInitialParamsUpdated(const QString &path, bool checkAutoEmul, bool checkAutoGame, int Emulator);
    void onShowMessageBox(const QString &title, const QString &message, QMessageBox::Icon icon);
    void init();
    void onStartClick();
    void onStopClick();
    //Application
    void onGameSelected(int index);
    void onScenaSelected(int index);
    void onConfSelect();
    //Scénarios
    void onScenaStart();
    void checkAndStopScenarios();
    void resetRepeatCount();

    //autre
signals:
    void sig_logMessage(const QString &message);
    void requestInitialParams();
    void requestCloseGame();
    void requestCloseBs();
protected:
    void closeEvent(QCloseEvent *event);
};
#endif // CGUI_H
