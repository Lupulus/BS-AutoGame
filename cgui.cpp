#include "cgui.h"
#include "ui_cgui.h"

#include <QStringListModel>
#include <QDebug>
#include <QComboBox>

CGui::CGui(QWidget *parent, CLogconsole *log, CParam *Param, CHelp *Help, CStart *Start)
    : QMainWindow(parent)
    , ui(new Ui::CGui)
{
    ui->setupUi(this);
    //_Help = new CHelp(this);


    init();

    // Fixer la taille de la fenêtre
    setFixedSize(350, 550);

    // Ajouter les éléments dans la cB_Game
    ui->cB_Game->addItem("Veuillez sélectionner un jeu", 0);
    ui->cB_Game->addItem("1945 Air Force", 1);


    // mémorise la référence des fênettre
    _logWindow = log;
    _Param = Param;
    _Help = Help;
    _Start = Start;

    // Connecter le signal de clic pour gérer la sélection
    connect(ui->cB_Game, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CGui::onGameSelected);
    // Connecter le signal pour gérer la sélection de scénario
    connect(ui->cB_Scena, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CGui::onScenaSelected);
    // Connectter le signal pour gérer les boutons :
    connect(ui->btn_ScenaConf, &QPushButton::clicked, this, &CGui::onConfSelect);
    connect(ui->btn_ScenaStart, &QPushButton::clicked, this, &CGui::onStartClick);
    connect(ui->btn_Param, &QPushButton::clicked, _Param, &CParam::openParamWindow);
    connect(ui->btn_Log, &QPushButton::clicked, _logWindow, &CLogconsole::openLogWindow);
    connect(ui->btn_ScenaStop, &QPushButton::clicked, this, [this]() {
        debug("BTN", "Bouton d'arrêt enclenché", "Warning");
        resetRepeatCount();
    });
    // Connecter les signaux de LogConsole (celui de scenario ce trouve dans onScenaStart) :
    connect(this, &CGui::sig_logMessage, _logWindow, &CLogconsole::appendLog);
    connect(_Param, &CParam::sig_logMessage, _logWindow, &CLogconsole::appendLog);

    //Connecter le signal pour gérer le message du bouton de la console :
    connect(_logWindow, &CLogconsole::logWindowSwitch, this, [&](bool isVisible) {
        ui->btn_Log->setText(isVisible ? "Fermer la Console" : "Ouvrir la Console");
    });
    // Connecter les signaux pour les paramètres initiaux :
    connect(_Param, &CParam::sendInitialParams, this, &CGui::onInitialParamsUpdated);
    connect(this, &CGui::requestInitialParams, _Param, &CParam::onSaveQuit);
    // Connection signaux CStart
    connect(_Start , &CStart::sig_logMessage, _logWindow, &CLogconsole::appendLog);
    connect(_Start, &CStart::showMessageBox, this, &CGui::onShowMessageBox);
    connect(_Start, &CStart::scenaStartRequest, this, &CGui::onScenaStart);
    connect(_Start, &CStart::closeStartThread, this, &CGui::onStopClick);
    connect(_Param, &CParam::sendInitialParams, _Start, &CStart::onInitialParamsUpdated);
    connect(this, &CGui::requestCloseGame, _Start, &CStart::onCloseGame);
    connect(this, &CGui::requestCloseBs, _Start, &CStart::onCloseBS);


    emit requestInitialParams();
}

CGui::~CGui()
{
    if (_startThread->isRunning()) {
        _startThread->quit();
        _startThread->wait();
    }
    delete ui;
}

void CGui::init()
{
    ui->gB_ScenaSelect->setVisible(false);
    ui->gB_ScenaInfo->setVisible(false);
    ui->gB_ScenaConf->setVisible(false);
    checkAndStopScenarios();
    // Nettoyer les comboBox
    ui->cB_Scena->clear();
}

void CGui::onShowMessageBox(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.exec();
}

void CGui::debug(const QString &function, const QString &msg, const QString &level)
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

void CGui::onInitialParamsUpdated(const QString &path, bool checkAutoEmul, bool checkAutoGame, int Emulator)
{
    _appPath = path;
    _AutoEmul = checkAutoEmul;
    _AutoGame = checkAutoGame;
    _Emulator = Emulator;
    debug("PARAM", "Paramètre Initiaux :", "Info");
    if (!_appPath.isEmpty()) {
        debug("PARAM", "- "+_appPath, "Info");
    }else {
        debug("PARAM", "- Chemin de l'émulateur " + QString::number(_Emulator) + "pas trouvé", _AutoEmul ? "Warning" : "Info");
    }
    if (!_AutoEmul && _AutoGame) {
        debug("PARAM", "- Lancement de l'émulateur " + QString::number(_Emulator) + " automatique désactivé", "Warning");
        debug("PARAM", "- Lancement du jeu automatique activé", "Info");
    } else if (_AutoEmul && !_AutoGame) {
        debug("PARAM", "- Lancement de l'émulateur " + QString::number(_Emulator) + " automatique activé", "Info");
        debug("PARAM", "- Lancement du jeu automatique désactivé", "Warning");
    } else if (_AutoEmul && _AutoGame) {
        debug("PARAM", "- Lancement de l'émulateur " + QString::number(_Emulator) + " automatique activé", "Info");
        debug("PARAM", "- Lancement du jeu automatique activé", "Info");
    } else if (!_AutoEmul && !_AutoGame) {
        debug("PARAM", "- Lancement de l'émulateur " + QString::number(_Emulator) + " automatique désactivé", "Warning");
        debug("PARAM", "- Lancement du jeu automatique désactivé", "Warning");
    }
} //Fin onInitialParamsUpdated

void CGui::onStartClick() //Lorsqu'on appuie sur le bouton "LAncer le scénario"
{
    _Start->moveToThread(_startThread);
    if (!_startThread->isRunning()) {
        _startThread->start(); // Démarrer le thread si ce n'est pas déjà fait
    }

    // Appeler la méthode dans le thread
    QMetaObject::invokeMethod(_Start, "onScenaStartClick", Qt::QueuedConnection,
                              Q_ARG(QVariant, _alias),
                              Q_ARG(QVariant, _aliasScena),
                              Q_ARG(int , _Emulator));
}

void CGui::onStopClick()
{
    if (_startThread->isRunning()) {
        _startThread->quit();
        _startThread->wait();
    }
}

//============================================================||
//               ** Gestion de l'Application **               ||
//============================================================||

void CGui::onGameSelected(int index)
{
    // Récupérer le texte de l'élément sélectionné (comboBox)
    _selectedGame = ui->cB_Game->itemText(index);
    debug("APP", "Jeu sélectionné : " + _selectedGame, "Info");
    _alias = ui->cB_Game->itemData(index); //Récupération de l'alias

    if (_alias.isValid()) {
        _aliasValue = _alias.toInt();
        debug("APP", "Alias sélectionné : " + QString::number(_aliasValue), "Info");

        // Exemple d'utilisation des alias dans une condition
        if (_aliasValue == 0) {
            debug("APP", "Aucun jeu sélectionné.", "Warning");
            emit requestCloseGame();
            init();
        } else if (_aliasValue == 1) {
            debug("APP", "1945 Air Force sélectionné.", "Info");
            ui->gB_ScenaSelect->setVisible(true); // Afficher la groupBox des scénarios
            ui->stackedWidget->setCurrentIndex(0);

            // Ajouter les scénarios spécifiques à ce jeu
            ui->cB_Scena->addItem("Choisir un Scénario", 0);
            ui->cB_Scena->addItem("Niv-91_AutoWin", 1);
        }
    }
} //Fin onGameSelected

void CGui::onScenaSelected(int index)
{
    // Récupérer le scénario sélectionné
    _selectedScena = ui->cB_Scena->itemText(index);
    _aliasScena = ui->cB_Scena->itemData(index);

    if (_aliasScena.isValid()) {
        _aliasValueScena = _aliasScena.toInt();
        debug("APP", "Scénario sélectionné : " + _selectedScena, "Info");
        debug("APP", "Alias du scénario : " + QString::number(_aliasValueScena), "Info");

        // Afficher des informations spécifiques au scénario
        if (_aliasValueScena == 0) {
            ui->gB_ScenaInfo->setVisible(false);
        } else if (_aliasValueScena ==1) {
            ui->LaScenaInfo->setText("Ce scénario permet de jouer automatiquement le niveau 91 du jeu X fois.");
            ui->gB_ScenaInfo->setVisible(true);
            debug("APP", "Niv 91 AutoWin sélectionné. Affichage des infos.", "Info");
        }
    }
} // Fin onScenaSelected

void CGui::onConfSelect()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->gB_ScenaConf->setVisible(true);
}

void CGui::closeEvent(QCloseEvent *event)
{
    debug("APP", "Procédure de fermeture de l'application", "Info");

    if (_currentScenario) {
        debug("APP", "Scénario actif, demande de confirmation pour forcer l'arrêt.", "Warning");

        // Afficher une boîte de dialogue de confirmation
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this,
            "Forcer l'arrêt",
            "Un scénario est encore actif ! Voulez-vous forcer l'arrêt de l'application ?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            debug("APP", "L'utilisateur a choisi de forcer l'arrêt.", "Warning");

            _currentScenario->stop();

            delete _currentScenario;
            _currentScenario = nullptr;

            QThread::msleep(500);
            emit requestCloseBs();

            QThread::msleep(1500);

            // Fermer la fenêtre de log & Param (si elle sont ouverte)
            if (_logWindow) {
                _logWindow->close();
            }
            if (_Param) {
                _Param->close();
            }
            if (_Help) {
                _Help->close();
            }

            debug("APP", "Fermeture forcé de l'application acceptée.", "Warning");
            event->accept();  // Permet de fermer la fenêtre principale
        } else {
            debug("APP", "L'utilisateur a annulé la fermeture.", "Warning");
            event->ignore();  // Empêche la fermeture de la fenêtre principale
        }
    } else {
        debug("APP", "Aucun scénario actif, fermeture de l'application acceptée.", "Info");
        QThread::msleep(1500);
        //emit requestCloseBs();
        // On ferme l'émulateur :

        _Start->onCloseBS();


        // Fermer la fenêtre de log & Param (si elle sont ouverte)
        QThread::msleep(1000);
        if (_logWindow) {
            _logWindow->close();
        }
        if (_Param) {
            _Param->close();
        }
        if (_Help) {
            _Help->close();
        }

        // Arrêter le thread dédié si actif
        if (_startThread && _startThread->isRunning()) {
            _startThread->quit();
            _startThread->wait();
        }
        debug("APP", "Fermeture de l'application acceptée.", "Warning");
        event->accept();  // Permet de fermer la fenêtre principale
    }
} //Fin onConfSelected
//=========================================================||
//               ** Gestion des Scénarios **               ||
//=========================================================||
void CGui::onScenaStart()
{
    if (_currentScenario) {
        debug("APP-Scena", "Un scénario est déjà en cours.", "Warning");
        return; // Empêcher de lancer un nouveau scénario si un autre est actif
    }

    if (_aliasValueScena != 1) {
        debug("APP-Scena", "Aucun scénario valide sélectionné.", "Error");
        return; // S'assurer qu'un scénario valide est sélectionné
    }

    debug("APP-Scena", "Démarrage du scénario : " + _selectedScena, "Info");
    _currentScenario = new CScenario(_selectedScena);

    // Récupérer les valeurs des checkboxes
    _isPubliciteChecked = ui->checkBox_Pub->isChecked();
    _isInfiniChecked = ui->checkBox_Infini->isChecked();

    // Configurer la répétition en fonction du mode "Infini"
    if (_isInfiniChecked) {
        ui->spinBox_Repeat->setDisabled(true);
        _currentScenario->setRepeatCount(-1); // -1 pour l'infini
    } else {
        ui->spinBox_Repeat->setEnabled(true);
        _currentScenario->setRepeatCount(ui->spinBox_Repeat->value());
    }

    // Configurer les propriétés du scénario
    _currentScenario->setPublicite(_isPubliciteChecked);
    _currentScenario->setInfini(_isInfiniChecked);

    // Gestion des threads pour exécuter le scénario
    QThread *thread = new QThread(this);
    // Attribuer un nom unique au thread en fonction du scénario
    thread->setObjectName("ScenarioThread_" + _selectedScena);

    // Déplacer le scénario dans le thread
    _currentScenario->moveToThread(thread);

    // Connecter les signaux et slots
    connect(_currentScenario, &CScenario::sig_logMessage, _logWindow, &CLogconsole::appendLog);
    connect(thread, &QThread::started, _currentScenario, &CScenario::executeScenario);
    connect(_currentScenario, &CScenario::sig_scenarioStopped, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(_currentScenario, &CScenario::sig_scenarioStopped, this, [&]() {
        QThread::msleep(1000);
        delete _currentScenario;  // Libérer la mémoire
        _currentScenario = nullptr;
        debug("APP-Scena", "Scénario "+_selectedScena+" terminé.", "Info");
        emit requestCloseGame();
        ui->btn_ScenaStart->setChecked(false);
    });

    // Lancer le thread
    thread->start();
    if (_currentScenario) {
        ui->btn_ScenaStart->setChecked(true);
    } else {
        debug("APP-Scena", "Erreur de lancement du scénario.", "Error");
        ui->btn_ScenaStart->setChecked(false);
    }
} //Fin onScenaStart

void CGui::checkAndStopScenarios()
{
    if (_currentScenario) {
        debug("APP-CAS", "Un scénario est actif. Arrêt en cours -> stop().", "Warning");
        _currentScenario->stop();  // Demander l'arrêt du scénario

        // Assurer la fin du thread avant la suppression
        QThread *thread = _currentScenario->thread;
        // Condition obligatoire (Sinon thread principal quitter = plantage) !!
        if (thread && thread != QThread::currentThread()) {
            thread->quit();
            thread->wait();  // Attendre que le thread se termine
        }

        delete _currentScenario;
        _currentScenario = nullptr;
        debug("APP-CAS", "Scénario Stoppé.", "Info");
    } else {
        debug("APP-CAS", "Aucun scénario actif.", "Info");
    }
} //Fin CheckAndStopScenarios (CAS)

void CGui::resetRepeatCount()
{
    if (_currentScenario) {
        debug("APP-CAS", "Réinitialisation du compteur de répétitions.", "Info");
        // Réinitialiser le nombre de répétitions à 1 (pour arrêter la boucle à la prochaine répétition
        ui->spinBox_Repeat->setValue(1);
        ui->checkBox_Pub->setChecked(true);
        ui->checkBox_Infini->setChecked(false);
        ui->btn_ScenaStart->setChecked(false);

        // Si le scénario est en cours, initialisation du Count à 0 pour définir le count à 1 répétition
        if (_currentScenario) {
            _currentScenario->setRepeatCount(0); // (le 0 est normal!) Réinitialiser le scénario avec 1 répétitions
        }
    } else {
        debug("APP-CAS", "Aucun scénario actif.", "Info");
    }
} //Fin resetRepeatCount
