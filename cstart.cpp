#include "cstart.h"

CStart::CStart(QObject *parent)
    : QObject{parent}
{
    qDebug() << "CStart instancié à l'adresse :" << this;
}

void CStart::debug(const QString &function, const QString &msg, const QString &level)
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

void CStart::onInitialParamsUpdated(const QString &path, bool checkAutoBs, bool checkAutoGame, int Emulator)
{
    _appPath = path;
    _AutoEmul = checkAutoBs;
    _AutoGame = checkAutoGame;
    _Emulator = Emulator;
    debug("PARAM", "Paramètre bien reçu dans CStart !", "Info");
}

// Méthode principal de lancement des Scénario !
void CStart::onScenaStartClick(const QVariant &aliasGame, const QVariant &aliasScena, const int &Emulator)
{
    // Copier adb.exe et les DLL dans le répertoire temporaire
    copyAdbAndDlls();
    // 0. Configuration primaire
    //debug("APP", "Jeu sélectionné: "+selectedGame, "Info");
    //debug("APP", "Scénario sélectionné: "+selectedScena,"Info");
    _aliasGame = aliasGame; // 1 = 1945 air force,
    _aliasScena = aliasScena; // 1 = Niv-91_AutoWin
    _Emulator = Emulator;
    if (_aliasGame == 1) {
        _packageGame = "com.os.airforce";
    }

    // 1. Lancer BlueStack
    if (!_AutoEmul) {
        debug("PARAM", "Le lancement automatique de l'émulateur"+ QString::number(_Emulator) + " est désactivé.", "Info");
        if (!_isEmulatorRunning(_Emulator)) {
            debug("PreStart", "L'émulateur"+ QString::number(_Emulator) + "  n'est pas lancé.", "Error");
            emit showMessageBox("Information", "Option de lancement automatique de l'émulateur"+ QString::number(_Emulator) + "  désactivée, à vous de le faire...", QMessageBox::Information);
            return; // Si BlueStacks n'est pas lancé, arrêter l'exécution
        } else {
            debug("PreStart", "L'émulateur"+ QString::number(_Emulator) + " est bien lancé.", "Info");
        }// Fin Vérif BSRunning 1
    } else {
        debug("PARAM", "Le lancement automatique de l'émulateur"+ QString::number(_Emulator) + " est activé.", "Info");
        if (!_isEmulatorRunning(_Emulator)) {
            // Si ce n'est pas le cas, essayer de le lancer
            if (!onStartBS()) {
                debug("PreStart", "L'émulateur"+ QString::number(_Emulator) + "  ne c'est pas lancé, arrêt du scénario.", "Error");
                return;
            }
            debug("PreStart", "L'émulateur"+ QString::number(_Emulator) + " vient d'être lancé.", "Info");
        } else {
            // Si BlueStacks est déjà lancé
            debug("PreStart", "L'émulateur"+ QString::number(_Emulator) + "  est bien lancé.", "Info");
        }
    } //Fin Vérif AutoBs 2

    // 2. Lancer le jeu

    if (!_AutoGame) {
        debug("PARAM", "Le lancement automatique du jeu est désactivé.", "Info");
        if (!_isGameRunning()) {
            debug("PreStart", "Le Jeu n'est pas lancé.", "Error");
            emit showMessageBox("Information", "Option de lancement automatique du jeu désactivée, à vous de le faire...", QMessageBox::Information);
            return; // Si le jeu n'est pas lancé, arrêter l'exécution
        } else {
            debug("PreStart", "Le Jeu est bien lancé.", "Info");
        }// Fin Vérif GameRunning 1
    } else {
        debug("PARAM", "Le lancement automatique du jeu est activé.", "Info");
        QThread::msleep(10000);
        onStartGame();
        if (!_isGameRunning()) {
            debug("PreStart", "Le jeu ne c'est pas lancé, arrêt du scénario.", "Error");
            return; // Si le jeu n'est pas lancé, arrêter l'exécution
        } else {
            debug("PreStart", "Le jeu est bien lancé.", "Info");
        }// Fin Vérif GameRunning 2
    }//Fin Verif AutoGame

    // 3. Vérifier si le jeu est bien lancé (Dernière vérification!)

    if (!_isGameRunning()) {
        debug("PreStart", "Le jeu n'est pas lancé.", "Error");
        return; // Si le jeu n'est pas lancé, arrêter l'exécution
    }

    QThread::sleep(40); //en seconde!
    // 4. Exécuter la méthode onScenaStart pour commencer le scénario
    debug("PreStart", "Conditions remplie pour démmarer le Scénario!", "Warning");
    emit scenaStartRequest();
    //onScenaStart();
} // Fin onScenaStartClick()


//=========================================================||
//               ** Gestion de BlueStacks **               ||
//=========================================================||

bool CStart::onStartBS()
{
    debug("APP-BS", "Vérification du lancement de l'émulateur...", "Info");

    // Vérifier si le fichier de l'application existe
    if (!_appPath.isEmpty()) {
        QProcess *process = new QProcess(this);
        process->startDetached(_appPath);
        QThread::msleep(500);
        if (_isEmulatorRunning(_Emulator)) {
            process->deleteLater();
            //debug("APP-BS", "BlueStacks lancé avec succès.", "Info");
            return true;
        }
        debug("APP-BS", "Erreur lors du lancement de l'émulateur !", "Error");
        process->deleteLater();
    } else {
        debug("APP-BS", "L'émulateur n'a pas été trouvé à l'emplacement spécifié.", "Error");
    }
    return false;
} //Fin onStartBSvoid
void CStart::onCloseBS()
{
    debug("APP-BS", "Vérification de la fermeture de l'émulateur...", "Info");

    // Vérifier si BlueStacks est en cours d'exécution
    if (_isEmulatorRunning(_Emulator)) {
        debug("APP-BS", "L'émulateur est en cours d'exécution. Fermeture en cours...", "Warning");

        // Tenter de fermer BlueStacks via le nom du processus
        QProcess process;
        QString filter;
        QString checkString;

        if (_Emulator == 1) {
            // Vérification pour BlueStacks
            filter = "HD-Player*"; // Supposé que le processus de BlueStacks commence par "HD-Player"
            checkString = "HD-Player";
        } else if (_Emulator == 2) {
            // Vérification pour LDPlayer
            filter = "dnplayer*"; // Supposé que le processus de LDPlayer commence par "dnplayer"
            checkString = "dnplayer";
        }
        process.start("taskkill", QStringList() << "/F" << "/IM" << filter);
        process.waitForFinished();

        QThread::msleep(500);
        // Lire la sortie de la commande
        QString output = process.readAllStandardOutput();
        process.close(); // Réinitialisation explicite

        //debug("APP-BS", "output de close :"+output, "Error");
        // Vérifier si la fermeture via le nom du service a réussi
        if (output.contains(checkString, Qt::CaseInsensitive)) {
            debug("APP-BS", "L'émulateur fermé avec succès via le process.", "Info");
        } else {
            debug("APP-BS", "Erreur lors de la fermeture de l'émulateur via le process.", "Error");
        }
    } else {
        debug("APP-BS", "L'émulateur est déjà fermé.", "Info");
    }
    QThread::msleep(250);
    emit closeStartThread();
} //Fin onCloseBS

bool CStart::_isEmulatorRunning(int Emulator)
{
    QProcess *process = new QProcess(this);
    QString filter;
    QString checkString;
    // Choisir le filtre et la chaîne de vérification en fonction de l'émulateur sélectionné
    if (Emulator == 1) {
        // Vérification pour BlueStacks
        filter = "imagename eq HD-Player*"; // Supposé que le processus de BlueStacks commence par "HD-Player"
        checkString = "HD-Player";
    } else if (Emulator == 2) {
        // Vérification pour LDPlayer
        filter = "imagename eq dnplayer*"; // Supposé que le processus de LDPlayer commence par "dnplayer"
        checkString = "dnplayer";
    }
    process->start("tasklist", QStringList() << "/fi" << filter);
    process->waitForFinished();

    // Lire la sortie de la commande
    QString output = process->readAllStandardOutput();
    process->close(); // Réinitialisation explicite

    //debug("APP-BS", "output de vérif 2:"+output, "Error");
    // Vérifier si BlueStacks est dans la sortie de la commande
    if (output.contains(checkString, Qt::CaseInsensitive)) {
        process->deleteLater();
        return true;
    }
    process->deleteLater();
    return false;
}//Fin _isBlueStackRunning
//======================================================||
//                ** Gestion des Jeux **                ||
//======================================================||

bool CStart::onStartGame()
{
    debug("APP-Game", "Démarrage de la méthode onStartGame.", "Info");

    // 2. Vérifier si BlueStacks est lancé
    if (!_isEmulatorRunning(_Emulator)) {
        debug("APP-Game", "L'émulateur n'est pas lancé. Arrêt du scénario.", "Error");
        return false; // Arrêter la méthode
    }

    // 3. Ce connecter à BlueStacks (adb)
    if (!connectToBlueStacks()) {
        debug("APP-Game", "Échec de la connexion à l'émulateur après le délai maximal.", "Error");
        return false;
    }

    // 4. Lancer le jeu si connection réussis (adb)
    QThread::sleep(8);
    QStringList arguments;
    //Commande Forum "Monkey" (problème de "redémarrage du jeu"):
    //adb -s 127.0.0.1:5555 shell monkey -p com.os.airforce -c android.intent.category.LAUNCHER 1
    //Command Chat GPT (Problème de redémarrage également!):
    //adb -s 127.0.0.1:5555 shell am start -n com.os.airforce
    //Ma Commande trouvé en analysant Bs... (fonctionnel à tout point pour le moment):
    arguments << "-s" << "127.0.0.1:5555" << "shell" << "am" << "start" << "-n"
              << _packageGame + "/com.google.firebase.MessagingUnityPlayerActivity";
    //debug("Command", "Commande : "+QString(QStringList() << arguments), "Warning")
    // Exécuter la commande via un process
    QProcess adbProcess;
    adbProcess.close();
    adbProcess.startDetached(_adbDir + "adb.exe", QStringList() << arguments);
    adbProcess.close(); // Réinitialisation explicite

    QThread::msleep(4000);
    if (!_isGameRunning()) {
        debug("APP-Game", "Échec du démarrage du Jeux...", "Error");
        return false; // Arrêter la méthode en cas d'erreur
    }
    //debug("APP-Game", "Le jeu a été lancé avec succès!", "Info");
    return true; //Ouff...
} //Fin onStartGame

bool CStart::connectToBlueStacks() {

    //Configurer l'environnement pour permettre l'exécution de la commande.
    _env = QProcessEnvironment::systemEnvironment();
    _env.insert("PATH", _adbDir + _env.value("PATH"));

    QProcess adbConnectProcess;
    adbConnectProcess.setProcessEnvironment(_env);
    adbConnectProcess.close();

    _maxWaitTime = 30000;  // Délai d'attente maximal (30 secondes)
    _waitInterval = 3000;   // Intervalle entre les vérifications
    _elapsedTime = 0;  // Temps écoulé depuis le début de l'attente

    while (_elapsedTime < _maxWaitTime) {
        // Démarrer la commande de connexion
        adbConnectProcess.close();
        adbConnectProcess.start(_adbDir + "adb.exe", QStringList() << "connect" << "127.0.0.1:5555");

        if (!adbConnectProcess.waitForStarted()) {
            debug("APP-Game", "Erreur lors du démarrage du processus ADB.", "Error");
        }
        // Attendre que la sortie soit prête à être lue
        if (!adbConnectProcess.waitForReadyRead()) {
            debug("APP-Game", "Erreur lors de la lecture de la sortie.", "Error");
        }

        // Lire la sortie pour vérifier la connexion
        QString output = adbConnectProcess.readAllStandardOutput();
        debug("APP-Game", "output:"+output, "Warning");

        // Vérifier si la connexion a été réussie
        if (output.contains("connected", Qt::CaseInsensitive)) {
            debug("APP-Game", "Connexion réussie à l'émulateur.", "Info");
            adbConnectProcess.close();
            return true;  // Connexion réussie, sortir de la boucle
        }

        // Si la connexion échoue, attendre et réessayer
        debug("APP-Game", "Connexion échoué à l'émulateur. Autre tentative dans : 3s", "Warning");
        _elapsedTime += _waitInterval;
        if (_elapsedTime < _maxWaitTime) {
            QThread::msleep(_waitInterval);
        }
    }
    adbConnectProcess.close();
    return false;
}

void CStart::onCloseGame() {
    debug("APP-Game", "Tentative de fermeture du jeu.", "Info");

    QProcess adbProcess;
    adbProcess.setProcessEnvironment(_env);
    adbProcess.close();

    // Exécuter la commande pour arrêter le jeu
    QStringList arguments;
    arguments << "-s" << "127.0.0.1:5555" << "shell" << "am" << "force-stop" << _packageGame;

    adbProcess.start(_adbDir + "adb.exe", arguments);

    if (!adbProcess.waitForStarted()) {
        debug("APP-Game", "Échec du démarrage de la commande force-stop.", "Error");
        return;
    }

    if (!adbProcess.waitForFinished()) {
        debug("APP-Game", "La commande force-stop n'a pas pu être terminée.", "Error");
        adbProcess.close();
        return;
    }

//    QString output = adbProcess.readAllStandardOutput().trimmed();

    //debug("APP-Game", "Commande force-stop exécutée. Sortie : " + output, "Info");
    adbProcess.close();
}
bool CStart::_isGameRunning() {
    //debug("APP-Game", "Vérification si le jeu est en cours d'exécution.", "Info");

    QProcess adbProcess;
    adbProcess.setProcessEnvironment(_env);
    adbProcess.close();

    // Exécuter la commande pour obtenir les processus en cours
    QStringList arguments;
    arguments << "-s" << "127.0.0.1:5555" << "shell" << "pidof" << _packageGame;

    adbProcess.start(_adbDir + "adb.exe", arguments);

    if (!adbProcess.waitForStarted()) {
        debug("APP-Game", "Échec du démarrage de la commande pidof.", "Error");
        return false;
    }

    if (!adbProcess.waitForFinished()) {
        debug("APP-Game", "La commande pidof n'a pas pu être terminée.", "Error");
        adbProcess.close();
        return false;
    }

    QString output = adbProcess.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        //debug("APP-Game", "Le jeu est en cours d'exécution.", "Info");
        adbProcess.close();
        return true;  // Le jeu est en cours d'exécution
    }

    //debug("APP-Game", "Le jeu n'est pas en cours d'exécution.", "Info");
    adbProcess.close();
    return false;  // Le jeu n'est pas trouvé
}

//=========================================================||
//           ** Gestion des dossiers/fichiers **           ||
//=========================================================||
bool CStart::_copyFile(const QString &source, const QString &destination) {
    QFile file(source);
    if (file.exists()) {
        return file.copy(destination);
    }
    return false;
}
bool CStart::_copyDir(const QString &sourceDir, const QString &destinationDir) {
    QDir source(sourceDir);
    if (!source.exists()) {
        return false;
    }

    // Créer le répertoire de destination s'il n'existe pas
    QDir destination(destinationDir);
    if (!destination.exists()) {
        destination.mkpath(".");
    }

    // Copier tous les fichiers et sous-répertoires du répertoire source vers le répertoire destination
    foreach (QString dirItem, source.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString srcFile = source.absoluteFilePath(dirItem);
        QString destFile = destination.absoluteFilePath(dirItem);

        if (QFileInfo(srcFile).isDir()) {
            // Si c'est un sous-répertoire, appeler la fonction récursive
            if (!_copyDir(srcFile, destFile)) {
                return false;
            }
        } else {
            // Si c'est un fichier, copier directement
            QFile::copy(srcFile, destFile);
        }
    }

    return true;
}

void CStart::copyAdbAndDlls() {
    // Emplacement temporaire où le dossier sera copié
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    _adbDir = tempPath + "/BS-AutoGame/adb/";

    // Copier tout le dossier /adb/ressources/adb/ dans le répertoire temporaire
    if (!_copyDir(":/adb/ressources/adb/", _adbDir)) {
        debug("APP-DIR", "Erreur lors de la copie du dossier adb dans :"+_adbDir, "Warning");
        return;
    }

    debug("APP-DIR", "Dossier adb copié avec succès dans :"+_adbDir, "Info");
}
