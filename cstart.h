#ifndef CSTART_H
#define CSTART_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QListView>
#include <QStringListModel>
#include <QThread>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QCoreApplication>


class CStart : public QObject
{
    Q_OBJECT
public:
    explicit CStart(QObject *parent = nullptr);
    void debug(const QString &function, const QString &msg, const QString &level = "Info");

signals:
    void sig_logMessage(const QString &message);
    void showMessageBox(const QString &title, const QString &message, QMessageBox::Icon icon);
    void scenaStartRequest();
    void closeStartThread();
private:
    QString _appPath;
    QString _adbDir;
    QString _adbConnectCommand;
    QString _packageGame;

    QVariant _aliasGame;
    QVariant _aliasScena;
    int _Emulator;

    int _maxWaitTime;
    int _waitInterval;
    int _elapsedTime;

    bool _isEmulatorRunning(int Emulator);
    bool _isGameRunning();
    bool _AutoEmul;
    bool _AutoGame;
    void copyAdbAndDlls();

    QProcessEnvironment _env;
    qint64 pid;
public slots:
    void onInitialParamsUpdated(const QString &path, bool checkAutoBs, bool checkAutoGame, int Emulator);
    void onScenaStartClick(const QVariant &aliasGame, const QVariant &aliasScena, const int &Emulator);

    //BlueStacks
    bool onStartBS();
    void onCloseBS();
    //Jeux
    bool onStartGame();
    void onCloseGame();
    bool connectToBlueStacks();
    //Fichier/Dossier
    bool _copyFile(const QString &source, const QString &destination);
    bool _copyDir(const QString &sourceDir, const QString &destinationDir);

};

#endif // CSTART_H
