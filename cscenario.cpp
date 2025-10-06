#include "cscenario.h"
#include "qdir.h"
#include <QDebug>

//logconsole* CScenario::logWindow = nullptr;

CScenario::CScenario(const QString &name, QObject *parent)
    : QObject(parent), scenarioName(name) {
    qDebug() << "CScenario instancié à l'adresse :" << this;
}

CScenario::~CScenario() {
    if (thread && thread->isRunning()) {
        thread->quit();
        thread->wait();
    }
    delete thread;
}
void CScenario::debug(const QString &function, const QString &msg, const QString &level)
{
    // Définir les couleurs en fonction du niveau
    QString color;
    if (level == "Error") {
        color = "red";
    } else if (level == "Warning") {
        color = "yellow";
    } else { // Par défaut, "Info"
        color = "green";
    }
    QString colorCode;
    if (level == "Error") {
        colorCode = "#FF0000"; // Rouge
    } else if (level == "Warning") {
        colorCode = "#FFFF00"; // Jaune
    } else { // Par défaut, "Info"
        colorCode = "#00FF00"; // Vert
    }

    // Construire le message final avec la couleur
    QString PreMsg = "[" + level + "] [" + function + "] ";
    QString finalMessage = PreMsg + msg;

    // Ajouter les couleurs pour le terminal (ANSI escape codes) et dans la logconsole (HTML)
    QString coloredMessage = QString("\033[38;2;%1m%2\033[0m").arg(color == "red" ? "255;0;0" : color == "yellow" ? "255;255;0" : "0;255;0").arg(finalMessage);
    QString ConsoleMsg = QString("<font color='%1'>[%2] [%3] %4</font>").arg(colorCode).arg(level).arg(function).arg(msg);

    // Afficher dans le terminal & logconsole
    qDebug().noquote().nospace() << coloredMessage;
    emit sig_logMessage(ConsoleMsg);
}
int CScenario::executeCommandAdb(const QStringList &arguments, const QString &context) {
    QStringList fullArguments;
    _adbDir = tempPath + "/BS-AutoGame/adb/";

    _env = QProcessEnvironment::systemEnvironment();
    _env.insert("PATH", _adbDir + _env.value("PATH"));
    _program = _adbDir + "adb.exe";
    QProcess process;
    process.setProcessEnvironment(_env);
    process.close();

    fullArguments << "-s" << "127.0.0.1:5555";
    fullArguments << arguments;

    //debug("Adb", "Commande : .../adb.exe" + fullArguments.join(" "), "Info");

    // Connecter pour capturer les erreurs dès qu'elles se produisent
    QObject::connect(&process, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        switch (error) {
        case QProcess::FailedToStart:
            debug(context, "Le programme n'a pas pu être démarré : " + _program, "Error");
            break;
        case QProcess::Crashed:
            debug(context, "Le programme s'est arrêté de manière inattendue : " + _program, "Error");
            break;
        default:
            debug(context, "Erreur inconnue : " + _program, "Error");
        }
    });

    // Démarrer le processus
    process.start(_program, fullArguments);
    if (!process.waitForFinished(5000)) { // Timeout après 5 secondes
        debug(context, "La commande a expiré : " + _program + " " + fullArguments.join(" "), "Error");
        return 0;
    }

    // Récupérer les sorties
    QString stdOutput = process.readAllStandardOutput();
    QString stdError = process.readAllStandardError();

    debug("Adb", "Commande exécutée : /adb.exe" + fullArguments.join(" "), "Info");
    process.close();

    // Déboguer les sorties
    if (!stdOutput.isEmpty()) {
        //debug(context, "(1) Sortie standard : " + stdOutput.trimmed(), "Warning");
        return 1; //retourn 1 si la sortie est remplie
    }

    if (!stdError.isEmpty()) {
        //debug(context, "(0) Erreur standard : " + stdError.trimmed(), "Warning");
        return 0; //retourn 0 si la sortie est est remplie
    }

    if (stdOutput.isEmpty() && stdError.isEmpty()) {
        //debug(context, "(2) Aucune sortie détectée (standard ou erreur).", "Warning");
        return 2; //retourn 2 si les deux sorties sont vide!
    }

    return 3; //retourn 3 si les deux sorties sont remplies
}

void CScenario::start()
{
    stopRequested = false;  // Réinitialiser le flag d'arrêt

    // Connecter le thread pour démarrer et arrêter correctement
    connect(thread, &QThread::started, this, &CScenario::executeScenario);
    connect(this, &CScenario::sig_scenarioStopped, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    debug("START", "Scénario démarré dans un nouveau thread.", "Info");

    // Démarrer le thread
    thread->start();
}

void CScenario::stop()
{
    debug("STOP", "Arrêt du scénario "+scenarioName+" demandé.", "Info");
    stopRequested = true;  // Demander l'arrêt propre

    QThread::msleep(1000);

    // Assurer que le thread est arrêté proprement
    if (thread && thread->isRunning()) {
        debug("STOP", "Attente de l'arrêt complet du thread.", "Warning");
        thread->quit();
        thread->wait();  // Attendre que le thread se termine avant de continuer
    }

    emit sig_scenarioStopped(scenarioName);
    debug("STOP", "Scénario " + scenarioName + " arrêté.", "Info");
}

void CScenario::setRepeatCount(int count)
{
    if (count == 0) {
        repeatCount = count+1;
        debug("STOP", "Le nombre de répétitions est défini à " + QString::number(repeatCount) + ". Arrêt des répétitions, dernier scénario en cours.", "Warning");
        stopRequested = true;  // Si le nombre de répétitions est 1, arrêtez le scénario
    } else {
        repeatCount = count;
    }
}

void CScenario::setPublicite(bool checked)
{
    publiciteChecked = checked;
    //debug("Publicité est activé : ") + qDebug() << "" << publiciteChecked;
    //debug("Publicité est activé : " + (publiciteChecked ? "true" : "false"));
}

void CScenario::setInfini(bool checked)
{
    infiniChecked = checked;
    //qDebug() << "Infini est activé : " << infiniChecked;
}

void CScenario::executeScenario()
{
    // Démarrage du scénario
    emit sig_scenarioStarted(scenarioName);

    debug("START", "Exécution du scénario : " + scenarioName, "Info");
    debug("START", "Répétitions : " + QString::number(repeatCount) + ", Mode infini : " + QString(infiniChecked ? "true" : "false"), "Info");
    debug("START", "Publicité activée : " + QString(publiciteChecked ? "true" : "false"), "Info");

    if (infiniChecked) {
        repeatCount = -1;  // Valeur -1 pour indiquer une répétition infinie
    }
    //Copie des Images/Template dans le répertoire Temp :
    copyImagesToTemp();
    //Vérification écran de chargement
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    std::string TemplateChargement = (screenshotDir + "/Chargement_Template-1.png").toStdString();
    cv::Mat grayTemplateChargement = convertToGray(TemplateChargement);

    // Boucle pour vérifier le chargement
    while (true) {
        QString screenshotPath = Screenshot();
        if (screenshotPath.isEmpty()) {
            debug("LOADING", "Erreur lors de la capture de l'écran.", "Error");
            QThread::sleep(2); // Attendre avant de réessayer
            continue;
        }

        cv::Mat grayScreenshot = convertToGray(screenshotPath.toStdString());
        if (!compareImagesIdemSize(grayTemplateChargement, grayScreenshot, 0.01)) {
            debug("LOADING", "Écran de chargement non détecté, sortie de la boucle.", "Info");
            break; // Sortir de la boucle si l'écran de chargement n'est pas détecté
        }

        debug("LOADING", "Chargement du jeu détecté, nouvelle vérification dans 3 secondes.", "Info");
        QThread::sleep(3); // Attendre x secondes avant de réessayer
    }

    if (scenarioName == "Niv-91_AutoWin") {
        while (!stopRequested) {

            //Gestion des Pop-up & Sélection de l'avionique pour le niveau 91!
            QThread::sleep(10);
            if (publiciteChecked) {
                debug("Niv-91_AutoWin", "Anti-Pub/Pop-up activé", "Info");
                StartPub();
            } else {
                debug("Niv-91_AutoWin", "Anti-Pub/Pop-up désactivé", "Info");
            }
            QThread::msleep(5000);
            selectPlane();

            for (int i = 0; i < repeatCount || repeatCount == -1; ++i) {
                // Log de la répétition avant d'appeler onTimeout
                if (repeatCount != -1) {
                    debug("START", "Répétition " + QString::number(i + 1) + " sur " + QString::number(repeatCount), "Info");
                } else {
                    debug("START", "Répétition n° " + QString::number(currentRepeatCount + 1), "Info");
                }
                QThread::msleep(1500);
                runNiv91AutoWin();
                QThread::msleep(1000);
                onTimeout();

            }
            if (stopRequested) {
                    debug("STOP", "Arrêt demandé, sortie de la boucle.", "Warning");
                break;  // Sortir de la boucle si l'arrêt est demandé
            }
        }
    } else {
        debug("ERROR", "Aucune méthode associée au scénario : " + scenarioName, "Error");
    }
}

void CScenario::runNiv91AutoWin()
{
    debug("Niv-91_AutoWin", "Démarrage de Niv-91_AutoWin.", "Info");
    //Debut du scenario Niv91 1945 airforce
    std::string TemplateChargement = (screenshotDir + "/Chargement_Template-2.png").toStdString();
    cv::Mat grayTemplateChargement = convertToGray(TemplateChargement);
    std::string NeedPlaqueTemplate = (screenshotDir + "/NeedPlaqueScreen_Template.png").toStdString();
    cv::Mat grayTemplateNeedPlaque = convertToGray(NeedPlaqueTemplate);

    //Sélection du niveau
    QThread::msleep(1500);
    selectLvl(); //Sélectionne le niveau et clique sur "Jouer"
    //(Lancmeent de la partie, si suffisament de plaque alors la partie se lance, sinon, nouvelle fenêttre d'achat de plaque)

    QThread::msleep(200);
    screenshotPath = Screenshot();

    cv::Mat grayScreenshot = convertToGray(screenshotPath.toStdString());
    if (!compareImagesIdemSize(grayTemplateNeedPlaque, grayScreenshot, 0.02)) {
        debug("LOADING", "Écran d'achat de Plaque non détecté, poursuite de la séquence.", "Info");

        //Vérification écran de chargement de la partie avant lancement du script!
        while (true) {
            screenshotPath = Screenshot();

            cv::Mat grayScreenshot = convertToGray(screenshotPath.toStdString());
            if (!compareImagesIdemSize(grayTemplateChargement, grayScreenshot, 0.02)) {
                debug("LOADING", "Écran de chargement de la partie non détecté, lancement du script.", "Info");
                break; // Sortir de la boucle si l'écran de chargement n'est pas détecté
            }

            debug("LOADING", "Chargement de la partie détecté, nouvelle vérification.", "Info");
            QThread::msleep(200); // Attendre x secondes avant de réessayer
        }

        //Lancement du script de la partie!
        scriptNiv91();

        QThread::msleep(5000);
        tap(550, 1800, "Click sur Bouton pour valider la récompense");

        QThread::sleep(15);
        debug("Niv-91_AutoWin", "Fin du scénario Niv-91 AutoWin.", "Info");

    } else {
        debug("LOADING", "Écran d'achat de Plaque détecté, manque de crédit pour faire une partie.", "Warnning");
        repeatCount = 1;
        stopRequested = true;
    }
}

void CScenario::onTimeout()
{
    currentRepeatCount++;
    //debug("current RepeatCount :" + QString::number(currentRepeatCount) + ">=" + "repeatCount :" + QString::number(repeatCount));

    if (repeatCount == -1) {
        return;
    }else if (currentRepeatCount >= repeatCount) {
        debug("Timeout", "Fin des répétitions. Arrêt du scénario.", "Info");
        stop();
    }
}

//============================================================||
//             ** Méthode Général d'OpenCV **                 ||
//============================================================||

//Prendre une Screenshot :
QString CScenario::Screenshot() {
    // Définir le chemin du répertoire de capture d'écran
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    QDir dir(screenshotDir);
    if (!dir.exists()) {
        if (!dir.mkpath(screenshotDir)) { // Crée les répertoires manquants
            debug("ScreenShot", "Impossible de créer le répertoire de capture d'écran : " + screenshotDir, "Error");
            return "";
        }
    }
    // Chemin complet de la capture d'écran
    screenshotPath = screenshotDir + "/screenshot.png";
    AndroidScreenshotPath = "/sdcard/Pictures/Screenshots/screenshot.png";

    // Capture d'écran via ADB
    QStringList screencapCommand = {"shell", "screencap", "-p", AndroidScreenshotPath};
    int screencapOutput = executeCommandAdb(screencapCommand, "Capture d'écran");
    //debug("ScreenShot", "Sortie de screencapCommand: " + QString::number(screencapOutput), "Warning");
    if (screencapOutput != 2) {
        debug("ScreenShot", "Erreur lors de la capture d'écran via adb.", "Error");
        return "";
    }

    // Transférer l'image sur le PC
    QStringList pullCommand = {"pull", AndroidScreenshotPath, screenshotPath};
    int pullOutput = executeCommandAdb(pullCommand, "Transfert de l'image");
    //debug("ScreenShot", "Sortie de pullCommand: " + QString::number(pullOutput), "Warning");
    if (pullOutput != 0) {
        debug("ScreenShot", "Erreur lors du transfert de l'image via adb.", "Error");
        return "";
    }

    // Supprimer l'image du périphérique Android
    QStringList removeCommand = {"shell", "rm", AndroidScreenshotPath};
    executeCommandAdb(removeCommand, "ScreenShot");
    //debug("ScreenShot", "Sortie de removeCommand: " + QString::number(removeOutput), "Warning");

    // Retourner le chemin de la capture d'écran
    return screenshotPath;
}

// Comparer deux images de même taille (full screen)
bool CScenario::compareImagesIdemSize(const cv::Mat &grayTemplate, const cv::Mat &grayScreenshot, double threshold) {
    //double threshold = 0.01;
    // Vérifier que les deux images ont les mêmes dimensions
    if (grayTemplate.size() != grayScreenshot.size()) {
        debug("CompImg", "Les dimensions des images ne correspondent pas.", "Error");
        return false;
    }

    // Calculer la distance euclidienne normalisée entre les deux images
    double similarity = cv::norm(grayTemplate, grayScreenshot, cv::NORM_L2) / (grayTemplate.rows * grayTemplate.cols);

    debug("CompImg", "Similitude calculée : " + QString::number(similarity), "Info");

    // Retourner true si la similitude est inférieure au seuil, sinon false
    return similarity < threshold;
}

//Convertir en niveau de Gris pour utilisation par openCV (peut également utiliser l'autre méthode d'openCV)
cv::Mat CScenario::convertToGray(const std::string &imagePath) {
    // Lire l'image en couleur
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

    // Vérifier si l'image est valide
    if (image.empty()) {
        qDebug() << "Erreur : Impossible de charger l'image à partir du chemin : " << QString::fromStdString(imagePath);
        return cv::Mat(); // Retourner une image vide si l'image n'a pas pu être chargée
    }
    cv::Mat grayImage;
    // Convertir l'image en niveaux de gris si ce n'est pas déjà fait
    if (image.channels() > 1) {
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
        qDebug() << "Conversion de l'image " << QString::fromStdString(imagePath) << " en gris.";
    } else {
        grayImage = image; // Si l'image est déjà en niveaux de gris
    }

    return grayImage;
}

//Permet de détecter un Template_Button sur une Screenshot et d'en retirer les coordonée.
cv::Point CScenario::detectButton(const cv::Mat &screenshot, const cv::Mat &buttonTemplate, double threshold) {
    if (screenshot.empty()) {
        qDebug() << "Erreur : L'image de la capture d'écran est vide.";
        return cv::Point(-1, -1); // Coordonnées invalides
    }

    if (buttonTemplate.empty()) {
        qDebug() << "Erreur : Le modèle de bouton est vide.";
        return cv::Point(-1, -1); // Coordonnées invalides
    }

    // Appliquer la correspondance de template
    cv::Mat result;
    cv::matchTemplate(screenshot, buttonTemplate, result, cv::TM_CCOEFF_NORMED);

    // Trouver la valeur maximale et sa localisation
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    qDebug() << "Score de correspondance : " << maxVal;

    // Vérifier si la correspondance dépasse le seuil
    if (maxVal >= threshold) {
        cv::Point center(maxLoc.x + buttonTemplate.cols / 2, maxLoc.y + buttonTemplate.rows / 2);
        qDebug() << "Bouton détecté aux coordonnées : (" << center.x << ", " << center.y << ")";
        return center; // Retourner les coordonnées du bouton (centre du bouton?)
    }

    qDebug() << "Aucune correspondance trouvée avec le seuil " << threshold;
    return cv::Point(-1, -1); // Aucun bouton détecté
}

void CScenario::clickPoint(const cv::Point& point)
{
    // Vérification que les coordonnées sont valides
    if (point.x != -1 && point.y != -1)
    {
        // Log du clic
        debug("Clic", "Clic sur les coordonnées : (" + QString::number(point.x) + ", " + QString::number(point.y) + ")", "Info");

        // Commande pour simuler le clic via ADB (ou autre méthode de contrôle)
        QStringList tapCommand = {"shell", "input", "tap", QString::number(point.x), QString::number(point.y)};
        executeCommandAdb(tapCommand, "Clic");
        QThread::msleep(350);
    }
    else
    {
        // Log d'erreur si la position est invalide
        debug("Clic", "Point invalide pour le clic.", "Erreur");
    }
}

void CScenario::tap(int x, int y, const QString& description)
{
    QStringList tapCommand = {"shell", "input", "tap", QString::number(x), QString::number(y)};
    executeCommandAdb(tapCommand, description);
}


//============================================================||
//                   ** Gestion du jeu **                     ||
//============================================================||

void CScenario::StartPub() {
    debug("PUB", "Gestion des publicités en cours.", "Info");

    while (true) {
        // Capture d'écran et obtenir le chemin
        screenshotPath = Screenshot();
        if (screenshotPath.isEmpty()) {
            debug("PUB", "Erreur lors de la capture d'écran.", "Error");
            return;
        }
        debug("PUB", "Capture enregistrée dans : " + screenshotPath, "Info");

        cv::Mat screen = convertToGray(screenshotPath.toStdString());
        if (screen.empty()) {
            debug("PUB", "Impossible de charger l'image capturée.", "Error");
            return;
        }

        cv::Rect PubButton = findCloseButton(screen);

        if (PubButton.area() > 0) {
            // Simuler un clic sur le bouton publicitaire détecté
            int x = PubButton.x + PubButton.width / 2;
            int y = PubButton.y + PubButton.height / 2;
            tap(x, y, "Click-Close-PUB-Button");
            //debug("PUB", "Sortie de tapCommand: " + QString::number(tapOutput), "Warning");
            debug("PUB", QString("Publicité détectée et bouton cliqué à (%1, %2).").arg(x).arg(y), "Info");
        } else {
            // Si aucun bouton publicitaire n'est détecté, vérifier si l'écran principal est affiché
            if (isMainGameScreen(screen)) { // Implémentez une méthode isMainGameScreen
                debug("PUB", "Écran principal détecté. Fin de la gestion des publicités.", "Info");
                break;
            }
        }
        QThread::msleep(300);
    }
}

cv::Rect CScenario::findCloseButton(const cv::Mat &grayImage) {
    // Liste des chemins des modèles
    std::vector<std::string> templatesNames = {
        "/Close_Button_Template-1.png",
        "/Close_Button_Template-2.png",
    };

    debug("PUB", "Recherche de bouton de fermeture des publicités !", "Warning");

    // Itération sur les modèles
    for (const std::string &templateName : templatesNames) {
        // Créer le chemin complet du modèle
        QString templatePath = screenshotDir + QString::fromStdString(templateName);
        debug("PUB", "Chargement du modèle : " + QString::fromStdString(templateName), "Info");

        // On convertie l'image en gris avec la ligne suivante (ou ma méthode ConvertToGray) :
        cv::Mat buttonTemplate = cv::imread(templatePath.toStdString(), cv::IMREAD_GRAYSCALE);
        if (buttonTemplate.empty()) {
            debug("PUB", "Impossible de charger le modèle :" + templatePath, "Error");
            continue; // Passer au modèle suivant
        }
        //qDebug() << "Modèle chargé, dimensions : " << buttonTemplate.cols << "x" << buttonTemplate.rows;

        // Utiliser detectButton pour trouver le bouton
        cv::Point detectedPoint = detectButton(grayImage, buttonTemplate, 0.8);

        if (detectedPoint.x != -1 && detectedPoint.y != -1) {
            debug("PUB", "Bouton trouvé aux coordonnées : (" + QString::number(detectedPoint.x) + ", " + QString::number(detectedPoint.y) + ")", "Info");
            return cv::Rect(detectedPoint.x, detectedPoint.y, buttonTemplate.cols, buttonTemplate.rows);
        } else {
            debug("PUB", "Aucune correspondance trouvée pour le modèle : " + QString::fromStdString(templateName), "Info");
        }
    }
    debug("PUB", "Aucun bouton de publicité trouvé.", "Info");

    return cv::Rect(); // Aucun bouton trouvé
}

bool CScenario::isMainGameScreen(const cv::Mat &grayImage) {
    // Charger l'image de référence de l'écran principal
    QString mainScreenTemplatePath = screenshotDir + "/Main_Game_Screen.png";
    // On convertie l'image en gris avec la ligne suivante (ou ma méthode ConvertToGray) :
    cv::Mat mainScreenTemplate = cv::imread(mainScreenTemplatePath.toStdString(), cv::IMREAD_GRAYSCALE);

    if (mainScreenTemplate.empty()) {
        debug("PUB", "Impossible de charger le modèle de l'écran principal : " + mainScreenTemplatePath, "Error");
        return false;
    }
    cv::Mat result;
    // Calculer la similitude entre les deux images en utilisant la méthode de corrélation
    double similarity = cv::norm(grayImage, mainScreenTemplate, cv::NORM_L2) / (grayImage.rows * grayImage.cols);

    debug("isMainGameScreen", QString("Similitude calculée : %1").arg(similarity), "Info");

    // Déterminer si l'écran actuel correspond à l'écran principal
    const double threshold = 0.05;
    return similarity < threshold;
}

//      Extraction des screenshot nécéssaire du dossier temps !
QString CScenario::extractResourceToTemp(const QString &resourcePath) {
    std::string basePath = tempPath.toStdString() + "/BS-AutoGame/screenshot/";
    QString _resourcePath = QString::fromStdString(basePath) + resourcePath;
    QFile resourceFile(_resourcePath);
    debug("ExtractRsr", "Fichier demandé : " + _resourcePath, "Error");
    debug("ExtractRsr", "basePath : " + QString::fromStdString(basePath), "Error");

    if (!resourceFile.exists()) {
        qDebug() << "Erreur : Ressource introuvable :" << _resourcePath;
        return QString();
    }

    QString tempFilePath = tempPath + "/BS-AutoGame/screenshot/" + QFileInfo(_resourcePath).fileName();
    if (!resourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Erreur : Impossible d'ouvrir la ressource :" << _resourcePath;
        return QString();
    }

    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Erreur : Impossible de créer le fichier temporaire :" << tempFilePath;
        return QString();
    }

    tempFile.write(resourceFile.readAll());
    resourceFile.close();
    tempFile.close();

    debug("ExtractRsr", "Fichier demandé : " + tempFilePath, "Error");

    return tempFilePath;
}

void CScenario::copyImagesToTemp()
{
    // Définir le chemin des ressources et du dossier de destination
    QString sourceDir = ":/img/ressources/images/Scena-airforce/";
    QString targetDir = QDir::tempPath() + "/BS-AutoGame/screenshot/";

    // Vérifier si le répertoire cible existe, sinon le créer
    QDir dir(targetDir);
    if (!dir.exists()) {
        if (!dir.mkpath(targetDir)) {
            debug("copyImagesToTemp", "Impossible de créer le répertoire cible : " + targetDir, "Error");
            return;
        }
    }

    // Liste des images à copier depuis les ressources
    QStringList imageFiles = {
        // Ajoutez ici les noms des images
        "Close_Button_Template-1.png", "Close_Button_Template-2.png",
        "Main_Game_Screen.png", "Back_Button_Template.png", "Niv91MainScreen.png",
        "Solo_Button.png", "Chapitre10MainScreen.png", "SwitchAvion_Button_Template.png",
        "PlaneAssist_RepublicF105_Template-1.png", "PlaneAssist_RepublicF105_Template-2.png",
        "PlaneMitsubichi-Mosquito_Template-1.png", "PlaneMitsubichi-Mosquito_Template-2.png",
        "PlaneSupport_ChainLightning_Template-1.png", "PlaneSupport_ChainLightning_Template-2.png",
        "Chargement_Template-1.png", "AvioniqueMainScreen.png", "UsedPlane_Template-1.png",
        "Play_Button_Template.png", "Chargement_Template-2.png", "EndGame_Template-1.png",
        "EndGame_Template-2.png", "NeedPlaqueScreen_Template.png"
    };

    for (const QString &imageName : imageFiles) {
        QString sourcePath = sourceDir + imageName;
        QString targetPath = targetDir + imageName;

        // Vérifier si l'image existe déjà dans le répertoire cible
        if (QFile::exists(targetPath)) {
            //debug("copyImagesToTemp", "L'image existe déjà dans le répertoire cible : " + imageName, "Info");
            continue; // Sauter cette image et passer à la suivante
        }

        // Utiliser QResource pour accéder à l'image dans les ressources
        QResource resource(sourcePath);
        if (!resource.isValid()) {
            debug("copyImagesToTemp", "Image non trouvée dans les ressources : " + sourcePath, "Error");
            continue;
        }

        // Copier le fichier dans le répertoire cible
        QFile file(resource.absoluteFilePath());
        if (file.copy(targetPath)) {
            debug("copyImagesToTemp", "Image copiée avec succès : " + imageName, "Info");
        } else {
            debug("copyImagesToTemp", "Échec de la copie de l'image : " + imageName, "Error");
        }
    }
}

//============================================================||
//           ** Gestion du sélector Avionique **              ||
//============================================================||

void CScenario::selectPlane()
{
    debug("PlANE", "Début de Séléction de l'ensemble Avionique !", "Info");
    // Template :
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    std::string TemplateAvionMainScreen = (screenshotDir + "/AvioniqueMainScreen.png").toStdString();
    std::string TemplateBack = (screenshotDir + "/Back_Button_Template.png").toStdString();
    std::string TemplatePlane1 = (screenshotDir + "/PlaneMitsubichi-Mosquito_Template-1.png").toStdString();
    std::string TemplatePlane2 = (screenshotDir + "/PlaneMitsubichi-Mosquito_Template-2.png").toStdString();
    std::string TemplatePlaneAssist1 = (screenshotDir + "/PlaneAssist_RepublicF105_Template-1.png").toStdString();
    std::string TemplatePlaneAssist2 = (screenshotDir + "/PlaneAssist_RepublicF105_Template-2.png").toStdString();
    std::string TemplatePlaneSupport1 = (screenshotDir + "/PlaneSupport_ChainLightning_Template-1.png").toStdString();
    std::string TemplatePlaneSupport2 = (screenshotDir + "/PlaneSupport_ChainLightning_Template-2.png").toStdString();
    std::string TemplateSwitchAvion = (screenshotDir + "/SwitchAvion_Button_Template.png").toStdString();


    screenshotPath = Screenshot();
    //On vérifie si on es bien sur l'écran principal (menu)
    cv::Mat grayScreen = convertToGray(screenshotPath.toStdString());
    if (!isMainGameScreen(grayScreen)) {
        debug("PLANE", "Ecran principal non détecté ?! Recherche pub/pop-up...", "Warning");
        findCloseButton(grayScreen);
    }
    QThread::msleep(500);
    // On clique sur les coordonées : x=280,y=600 (pour choisir l'avion)
    tap(280, 600, "Clic-Choisir-Avion");
    QThread::msleep(100);
    tap(150, 175, "Clic-Emplacement1");

    debug("PLANE", "Attente de chargement.", "Info");
    QThread::msleep(1500); //Petite pause pour laisser charger la page d'avionnique!

    // Conversion des templates en gris
    cv::Mat grayTemplateAvioniqueMainScreen = convertToGray(TemplateAvionMainScreen);

    if (!compareImagesIdemSize(grayTemplateAvioniqueMainScreen, grayScreen, 0.025)) {
        debug("PLANE", "Écran d'avionique non conforme au choix.", "Info");

        cv::Mat grayTempalteSwitchAvion = convertToGray(TemplateSwitchAvion);

        // VERIF 1 : Vérification de l'avion principal
        cv::Mat grayTemplatePlane1 = convertToGray(TemplatePlane1);
        if (!verifyPlane(grayTemplatePlane1)) {
            debug("PLANE", "Avion principal non conforme au choix.", "Info");
            cv::Mat grayTemplatePlane2 = convertToGray(TemplatePlane2);
            selectGrade(2); //Selection du grade d'avion (2)
            if (!scrollUntilFound(grayTemplatePlane2)) {
                debug("PLANE", "Avion principal non trouvé, abandon de la recherche.", "Error");
            } else {
                debug("PLANE", "Avion principal conforme au choix.", "Info");
            }
        }

        switchAvion(grayTempalteSwitchAvion);
        // VERIF 2 : Vérification de l'assistant
        cv::Mat grayTemplatePlaneAssist1 = convertToGray(TemplatePlaneAssist1);
        if (!verifyPlane(grayTemplatePlaneAssist1)) {
            debug("PLANE", "Avion assistant non conforme au choix.", "Info");
            cv::Mat grayTemplatePlaneAssist2 = convertToGray(TemplatePlaneAssist2);
            selectGrade(1);
            if (!scrollUntilFound(grayTemplatePlaneAssist2)) {
                debug("PLANE", "Avion assistant non trouvé, abandon de la recherche.", "Error");
            } else {
                debug("PLANE", "Avion assistant conforme au choix.", "Info");
            }
        }

        switchAvion(grayTempalteSwitchAvion);
        // VERIF 3 : Vérification du support
        cv::Mat grayTemplatePlaneSupport1 = convertToGray(TemplatePlaneSupport1);
        if (!verifyPlane(grayTemplatePlaneSupport1)) {
            debug("PLANE", "Support non conforme au choix.", "Info");
            cv::Mat grayTemplatePlaneSupport2 = convertToGray(TemplatePlaneSupport2);
            selectGrade(1);
            if (!scrollUntilFound(grayTemplatePlaneSupport2)) {
                debug("PLANE", "Support non trouvé, abandon de la recherche.", "Error");
            } else {
                debug("PLANE", "Support conforme au choix.", "Info");
            }
        }
    }
    debug("PLANE", "Écran d'avionique conforme au choix.", "Info");
    QThread::msleep(1500);
    // QUITTER : Capture et retour au menu
    cv::Mat grayTemplateBack = convertToGray(TemplateBack);
    backToMainMenu(grayTemplateBack);

}

bool CScenario::verifyPlane(cv::Mat& grayTemplate)
{
    QThread::msleep(350);
    screenshotPath = Screenshot();
    cv::Mat screen = convertToGray(screenshotPath.toStdString());
    cv::Point detectedPoint = detectButton(screen, grayTemplate, 0.665);

    if (detectedPoint.x != -1 && detectedPoint.y != -1) {
        debug("PLANE", "Avion trouvé aux coordonnées : (" + QString::number(detectedPoint.x) + ", " + QString::number(detectedPoint.y) + ")", "Info");
        return true;
    } else {
        debug("PLANE", "Aucune correspondance trouvée pour l'avion.", "Info");
        return false;
    }
}

void CScenario::selectGrade(int gradeNumber)
{
    // Coordonnées des boutons "Grade 1", "Grade 2" et "Grade 3"
    std::map<int, cv::Point> gradePositions = {
        {1, cv::Point(330, 1770)},
        {2, cv::Point(540, 1770)},
        {3, cv::Point(750, 1770)}
    };

    // Vérification si le grade demandé existe
    if (gradePositions.find(gradeNumber) != gradePositions.end()) {
        cv::Point gradePoint = gradePositions[gradeNumber];
        debug("PLANE", "Click sur le bouton 'Grade " + QString::number(gradeNumber) + "' !", "Info");
        clickPoint(gradePoint);
    } else {
        debug("PLANE", "Numéro de grade invalide : " + QString::number(gradeNumber), "Error");
    }
}



bool CScenario::scrollUntilFound(cv::Mat& grayTemplate)
{
    bool found = false;
    int scrollCount = 0;
    const int maxScrolls = 7;

    while (!found && scrollCount < maxScrolls) {
        screenshotPath = Screenshot();
        cv::Mat screen = convertToGray(screenshotPath.toStdString());
        cv::Point detectedPoint = detectButton(screen, grayTemplate, 0.8);
        found = detectedPoint.x != -1;

        if (!found) {
            // Vérifier si on a atteint la limite des scrolls
            if (scrollCount >= maxScrolls - 1) {
                debug("PLANE", "Élément non trouvé après " + QString::number(maxScrolls) + " scrolls, arrêt.", "Warning");
                return false;
            }

            // Descendre la molette
            QStringList tapCommand = {"shell", "input", "swipe", "540", "1440", "540", "1240", "500"};
            executeCommandAdb(tapCommand, "Descente molette");
            debug("PLANE", "Molette descendue (" + QString::number(scrollCount + 1) + "/" + QString::number(maxScrolls) + "), recherche continue...", "Info");

            scrollCount++;
        } else {
            debug("PLANE", "Click sur le bouton d'avion trouvé! ", "Info");
            clickPoint(detectedPoint);
        }
    }
    debug("PLANE", "Avion trouvé dans la séléction !", "Info");

    //On clique sur "Utiliser" (l'avion)
    QThread::msleep(550);
    screenshotPath = Screenshot();
    cv::Mat screen = convertToGray(screenshotPath.toStdString());
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    std::string TemplateUsedPlane = (screenshotDir + "/UsedPlane_Template-1.png").toStdString();
    cv::Mat grayTemplateUsedPlane = convertToGray(TemplateUsedPlane);

    cv::Point detectedPoint = detectButton(screen, grayTemplateUsedPlane, 0.9);
    if (detectedPoint.x != -1 && detectedPoint.y != -1) {
        debug("PLANE", "Click sur le bouton 'Utiliser' ! ", "Info");
        clickPoint(detectedPoint);
        return true;
    } else {
        debug("PLANE", "Aucun bouton 'Utiliser' trouvée.", "Warning");
        return false;
    }
    return false;
}

void CScenario::switchAvion(cv::Mat& grayTemplate)
{
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    std::string TemplateAvionMainScreen = (screenshotDir + "/AvioniqueMainScreen.png").toStdString();
    cv::Mat screen = convertToGray(TemplateAvionMainScreen);

    cv::Point detectedPoint = detectButton(screen, grayTemplate, 0.9);
    if (detectedPoint.x != -1 && detectedPoint.y != -1) {
        debug("PLANE", "Click sur le bouton 'SwitchAvionMenu' ! ", "Info");
        clickPoint(detectedPoint);
    } else {
        debug("PLANE", "Aucun bouton 'SwitchAvionMenu' trouvée.", "Warning");
    }
}

void CScenario::backToMainMenu(cv::Mat& grayTemplateBack)
{
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    std::string TemplateAvionMainScreen = (screenshotDir + "/AvioniqueMainScreen.png").toStdString();
    cv::Mat screen = convertToGray(TemplateAvionMainScreen);


    cv::Point detectedPoint = detectButton(screen, grayTemplateBack, 0.9);
    if (detectedPoint.x != -1 && detectedPoint.y != -1) {
        debug("PLANE", "Click sur le bouton 'BACK' ! ", "Info");
        clickPoint(detectedPoint);
    } else {
        debug("PLANE", "Aucun bouton 'BACK' trouvée.", "Warning");
    }
}

//============================================================||
//           ** Gestion du sélector de Niveau **              ||
//============================================================||


void CScenario::selectLvl()
{
    screenshotDir = tempPath + "/BS-AutoGame/screenshot";
    std::string imgSolo_Button = (screenshotDir + "/Solo_Button.png").toStdString();
    std::string imgChapitre10MainScreen = (screenshotDir + "/Chapitre10MainScreen.png").toStdString();
    std::string imgNiv91MainScreen = (screenshotDir + "/Niv91MainScreen.png").toStdString();
    std::string imgPlay_Button_Template = (screenshotDir + "/Play_Button_Template.png").toStdString();
    // Coordonée du niveau 91 (à cliquer) : x=545, y=1560
    cv::Mat graySolo_Button = convertToGray(imgSolo_Button);

    QThread::msleep(500); //Petit Pause avant screenshot (chargement)
    screenshotPath = Screenshot();
    cv::Mat grayScreen = convertToGray(screenshotPath.toStdString());
    //On vérifie si on es bien sur l'écran principal (menu)
    if (!isMainGameScreen(grayScreen)) {
        debug("LEVEL", "Ecran principal non détecté ?! Recherche pub/pop-up...", "Warning");
        findCloseButton(grayScreen);
    }
    QThread::msleep(150);
    cv::Point detectedPoint = detectButton(grayScreen, graySolo_Button, 0.7);
    clickPoint(detectedPoint);

    QThread::msleep(200); //Petit Pause avant screenshot (chargement)S
    screenshotPath = Screenshot();
    cv::Mat grayScreen2 = convertToGray(screenshotPath.toStdString());
    cv::Mat grayChapitre10MainScreen = convertToGray(imgChapitre10MainScreen);
    //On compare la screenshot pour voir si elle correspond au bon Chapitre...
    int maxRetries = 15;
    int attempts = 0;

    while (!compareImagesIdemSize(grayChapitre10MainScreen, grayScreen2, 0.0165) && attempts < maxRetries) {
        tap(240, 450, "Click PrécédentChapitre");
        QThread::msleep(200);
        screenshotPath = Screenshot();
        grayScreen2 = convertToGray(screenshotPath.toStdString());
        attempts++;
    }

    if (attempts >= maxRetries) {
        debug("LEVEL", "Erreur : Impossible de trouver Chapitre 10 après plusieurs tentatives.", "Error");
        return;
    }
    tap(545, 1560, "Click Lvl91"); // Clique sur le Niveau 91 !

    tap(550, 420, "Click Difficulté Moyenne"); //Clique sur la difficulté Moyenne (80%)
    QThread::msleep(400);
    screenshotPath = Screenshot();
    grayScreen2 = convertToGray(screenshotPath.toStdString());
    //Vérifier si nous somme bien dans la fênettre du Niveau 91 avec les configuration souhaité!
    cv::Mat grayNiv91MainScreen = convertToGray(imgNiv91MainScreen);
    if (!compareImagesIdemSize(grayNiv91MainScreen, grayScreen2, 0.0165)) { //Monter le % si le niveau est "passé"
        debug("LEVEL", "L'écran du Niveau91 ne correspond pas!", "Error");
        return;
    }
    debug("LEVEL", "Niveau 91 avec Difficulté Moyenne bien sélectionner, lancement de la Partie !", "Info");

    cv::Mat grayPlay_Button_Tempalte = convertToGray(imgPlay_Button_Template);
    detectedPoint = detectButton(grayScreen2, grayPlay_Button_Tempalte, 0.9);
    if (detectedPoint.x != -1 && detectedPoint.y != -1) {
        debug("LEVEL", "Click sur le bouton 'JOUER' ! ", "Info");
        clickPoint(detectedPoint); //Lancement de la "Partie" !!!!
    } else {
        debug("LEVEL", "Aucun bouton 'JOUER' trouvée.", "Warning");
    }
    QThread::msleep(100);
    tap(350, 1400, "Click 'Jeu Manuel(5 Médailes)' "); // Clique sur jeu Manuel !
}

//============================================================||
//             ** Gestion du Script Niv91 ! **                ||
//============================================================||

// Variables pour stocker la dernière position du swipe
int lastX = 530;  // Position initiale de l'avion
int lastY = 1530;

void CScenario::swipeCommand(int xb, int yb, int duration, const QString& description) {
    // Utiliser les dernières coordonnées enregistrées comme point de départ
    QStringList swipeCommand = {"shell", "input", "swipe",
                                QString::number(lastX), QString::number(lastY),
                                QString::number(xb), QString::number(yb),
                                QString::number(duration)};

    executeCommandAdb(swipeCommand, description);

    // Mettre à jour les coordonnées après le swipe
    lastX = xb;
    lastY = yb;
}

void CScenario::pressAndHold(int x, int y, int duration, const QString& description)
{
    QStringList swipeCommand = {"shell", "input", "swipe",
                                QString::number(x), QString::number(y),
                                QString::number(x), QString::number(y),
                                QString::number(duration)};

    executeCommandAdb(swipeCommand, description);

    // Mettre à jour la position actuelle
    lastX = x;
    lastY = y;
}

void CScenario::scriptNiv91()
{
    QElapsedTimer timer;
    timer.start();
    debug("SC-Niv91", "Début du Script Niv91, T=0s", "Info");


    std::string EndGame_Template1 = (screenshotDir + "/EndGame_Template-1.png").toStdString();
    std::string EndGame_Template2 = (screenshotDir + "/EndGame_Template-2.png").toStdString();

    cv::Mat grayEndGame_Template1 = convertToGray(EndGame_Template1);
    cv::Mat grayEndGame_Template2 = convertToGray(EndGame_Template2);
    // Maintien initial de l'avion
    pressAndHold(530, 1530, 1000, "Maintien avion");

    int lastScreenshotTime = 0; // Stocke le dernier temps où une capture a été prise

    // Swipes dynamiques basés sur la dernière position
    for (int i = 0; i < 400; i++) { //i = 1 >> T=0.1+duration= 0.5 seconde, 4 minutes = 240s >> i=300
        int duration = 450;
        if (i % 2 == 0) {
            swipeCommand(170, 1530, duration, "Déplacement à gauche");
        } else {
            swipeCommand(940, 1530, duration, "Déplacement à droite");
        }

        // Vérification écran de fin toutes les 5 secondes (50 itérations de 100ms)
        int elapsedTime = timer.elapsed() / 1000;

        if (elapsedTime - lastScreenshotTime >= 5) {
            lastScreenshotTime = elapsedTime; // Mise à jour du dernier moment de capture
            (void)QtConcurrent::run([this]() { //QtConcurrent permet d'exécuter la capture dans un Thread séparé!
                screenshotPath = Screenshot();
            });
            cv::Mat grayScreenshot = convertToGray(screenshotPath.toStdString());

            if (compareImagesIdemSize(grayEndGame_Template1, grayScreenshot, 0.025) ||
                compareImagesIdemSize(grayEndGame_Template2, grayScreenshot, 0.025)) {
                debug("LOADING", "Écran de fin de partie détecté, arrêt du script.", "Info");
                break; // Sortie immédiate de la boucle
            }
        }

        // Première partie de Bonus
        if (elapsedTime == 65) {
            debug("SC-Niv91", QString("%1secondes, activation du bonus").arg(elapsedTime), "Info");
            //tap(110,1220, "Click Bonus 'Haut'");
            //tap(110,1580, "Click Bonus 'Bas'");
            tap(110,1400, "Click Bonus 'Centre'");
        }
        if (elapsedTime == 74) {
            debug("SC-Niv91", QString("%1secondes, activation du bonus").arg(elapsedTime), "Info");
            //tap(110,1220, "Click Bonus 'Haut'");
            //tap(110,1400, "Click Bonus 'Centre'");
            tap(110,1580, "Click Bonus 'Bas'");
        }
        if (elapsedTime == 83) {
            debug("SC-Niv91", QString("%1secondes, activation du bonus").arg(elapsedTime), "Info");
            tap(110,1220, "Click Bonus 'Haut'");

            swipeCommand(lastX, lastY, 2500, "Maintient position");
        }
        //Deuxième partie de Bonus
        if (elapsedTime == 214) {
            debug("SC-Niv91", QString("%1secondes, activation du bonus").arg(elapsedTime), "Info");
            tap(110,1400, "Click Bonus 'Centre'");
            swipeCommand(170, lastY, 500, "Position Gauche");
            swipeCommand(lastX, lastY, 5000, "Maintient position");
        }
        if (elapsedTime == 224) {
            debug("SC-Niv91", QString("%1secondes, activation du bonus").arg(elapsedTime), "Info");

            tap(110,1580, "Click Bonus 'Bas'");
            swipeCommand(lastX, lastY, 2000, "Maintient position");
        }
        if (elapsedTime == 232) {
            debug("SC-Niv91", QString("%1secondes, activation du bonus").arg(elapsedTime), "Info");
            tap(110,1220, "Click Bonus 'Haut'");
        }
    }    
    // Calcul du temps écoulé en secondes
    int totalTime = (timer.elapsed() / 1000) - 10;
    debug("SC-Niv91", QString("Fin du Script Niv91 ! T=%1s").arg(totalTime), "Info");

    QThread::msleep(200);
}

