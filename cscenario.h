#ifndef CSCENARIO_H
#define CSCENARIO_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QMetaObject>
#include <QMetaMethod>
#include "clogconsole.h"
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QResource>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class CScenario : public QObject
{
    Q_OBJECT
public:
    explicit CScenario(const QString &name, QObject *parent = nullptr);
    ~CScenario();

    void start();  // Démarrer le scénario
    void stop();   // Arrêter le scénario
    void setRepeatCount(int count);
    void setPublicite(bool checked);
    void setInfini(bool checked);
    QThread *thread = new QThread;
    void debug(const QString &function, const QString &msg, const QString &level);

public slots:
    void executeScenario();

signals:
    void sig_scenarioStarted(const QString &name);
    void sig_scenarioStopped(const QString &name);
    void sig_logMessage(const QString &message);

private:
    QString scenarioName;  // Nom du scénario
    QString PreMsg;
    QString screenshotDir;
    QString screenshotPath;
    QString _adbDir;
    QString _program;
    QProcessEnvironment _env;
    const std::string basePath;
    QString AndroidScreenshotPath;
    QString extractResourceToTemp(const QString &resourcePath);
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    int executeCommandAdb(const QStringList &arguments, const QString &context);
    int repeatCount = 1;
    int currentRepeatCount =0;
    bool publiciteChecked;
    bool infiniChecked;
    bool stopRequested = false;
    bool isMainGameScreen(const cv::Mat &currentScreen);
    CLogconsole* logWindow;

    QString Screenshot();
    cv::Point detectButton(const cv::Mat &screenshot, const cv::Mat &buttonTemplate, double threshold);
    bool compareImagesIdemSize(const cv::Mat &grayTemplate, const cv::Mat &grayScreenshot, double threshold);
    void clickPoint(const cv::Point &point);
    bool verifyPlane(cv::Mat &grayTemplate);
    void selectGrade(int gradeNumber);
    bool scrollUntilFound(cv::Mat &grayTemplate);
    void backToMainMenu(cv::Mat &grayTemplateBack);
    void switchAvion(cv::Mat &grayTemplate);
    void tap(int x, int y, const QString &description);
    std::vector<cv::Rect> detectElements(const cv::Mat &image, const cv::Scalar &lowerColor, const cv::Scalar &upperColor);
    void swipeCommand(int xb, int yb, int duration, const QString &description);
    void pressAndHold(int x, int y, int duration, const QString &description);
private slots:
    void runNiv91AutoWin();  // Action spécifique au scénario
    void onTimeout();
    void StartPub();
    void copyImagesToTemp();
    void selectLvl();
    void selectPlane();
    void scriptNiv91();
    cv::Rect findCloseButton(const cv::Mat &image);
    cv::Mat convertToGray(const std::string &imagePath);
};
#endif // CSCENARIO_H
