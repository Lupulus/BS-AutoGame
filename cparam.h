#ifndef CPARAM_H
#define CPARAM_H

#include <QWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include "chelp.h"
#include <QPoint>
#include <QSettings>

namespace Ui {
class CParam;
}

class CParam : public QWidget
{
    Q_OBJECT

public:
    explicit CParam(QWidget *parent = nullptr, CHelp *Help = nullptr);
    ~CParam();
    bool _isChecked1;
    bool _isChecked2;
    int _isEmulator;
    QSettings settings;


    void debug(const QString& msg);

public slots:
    void openParamWindow();
    void onSaveQuit();
private:
    void showHelpAtButton(QPushButton *button);
    Ui::CParam *ui;

    CHelp *_Help = nullptr;
    void updateEmulatorPath();
    void setDefaultEmulatorPath(int emulator);
private slots:
    void openFile();
    bool checkPath(const QString &filePath);

signals:
    void logWindowSwitch(bool isVisible);
    void sendInitialParams(const QString &path, bool checkAutoBs, bool checkAutoGame, int _isEmulator);
    void sig_logMessage(const QString &message);

};

#endif // CPARAM_H
