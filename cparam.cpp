#include "cparam.h"
#include "ui_cparam.h"

CParam::CParam(QWidget *parent, CHelp *Help)
    : QWidget(parent)
    , ui(new Ui::CParam)
    , _Help(Help)
{
    ui->setupUi(this);

    //Initialiser Selecteur Emulateur
    qDebug() << "CParam instancié à l'adresse :" << this;
    QSettings settings("BS-AutoGame", "CParam");

    int defaultEmulator = settings.value("Emulator", 2).toInt();
    setDefaultEmulatorPath(defaultEmulator);

    //Connection pour changer le place holder!
    connect(ui->rButton_1, &QRadioButton::clicked, this, &CParam::updateEmulatorPath);
    connect(ui->rButton_2, &QRadioButton::clicked, this, &CParam::updateEmulatorPath);

    // Connexions des boutons d'aide
    connect(ui->btn_help_1, &QPushButton::clicked, this, [this]() {
        showHelpAtButton(ui->btn_help_1);
        //debug("[BTN] Aide affiché, page0");
        _Help->showHelpPage(0);
    });

    connect(ui->btn_help_2, &QPushButton::clicked, this, [this]() {
        showHelpAtButton(ui->btn_help_2);
        _Help->showHelpPage(1);
        //debug("[BTN] Aide affiché, page1");
    });

    connect(ui->btn_help_3, &QPushButton::clicked, this, [this]() {
        showHelpAtButton(ui->btn_help_3);
        _Help->showHelpPage(2);
        //debug("[BTN] Aide affiché, page1");
    });
    // Autres boutons
    connect(ui->btn_SaveQuit, &QPushButton::clicked, this, &CParam::onSaveQuit, Qt::UniqueConnection);
    connect(ui->btn_OpenFile, &QPushButton::clicked, this, &CParam::openFile);

    // Récupération des valeurs sauvegardées
    _isChecked1 = settings.value("IsChecked1", true).toBool(); // AutoEmul
    _isChecked2 = settings.value("IsChecked2", true).toBool(); // AutoGame
    _isEmulator = defaultEmulator;

    ui->cB_AutoExe->blockSignals(true);
    ui->cB_AutoExe_2->blockSignals(true);

    ui->cB_AutoExe->setChecked(_isChecked1);
    ui->cB_AutoExe_2->setChecked(_isChecked2);

    ui->cB_AutoExe->blockSignals(false);
    ui->cB_AutoExe_2->blockSignals(false);

    //qDebug() << "[INIT] AutoEmul:" << _isChecked1 << "AutoGame:" << _isChecked2;

    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    setFixedSize(400, 340);
}

CParam::~CParam()
{
    delete ui;
}

void CParam::updateEmulatorPath()
{

    if (ui->rButton_1->isChecked()) {
        qDebug() << "BlueStacks sélectionné";
        ui->lineEdit_AppRoad->clear();
        ui->lineEdit_AppRoad->setPlaceholderText("C:\\Program Files\\BlueStacks_nxt\\HD-Player.exe");
    } else if (ui->rButton_2->isChecked()) {
        qDebug() << "LDPlayer sélectionné";
        ui->lineEdit_AppRoad->clear();
        ui->lineEdit_AppRoad->setPlaceholderText("D:\\LDPlayer\\LDPlayer9\\dnplayer.exe");
    }
    ui->lineEdit_AppRoad->update();
    ui->lineEdit_AppRoad->repaint();
}

void CParam::showHelpAtButton(QPushButton *button)
{
    if (!_Help) {
        debug("Erreur : _Help non initialisé !");
        return;
    }
    //debug("Test ShowHelpAtButton, bouton");
    // Calcule la position globale du bouton
    QPoint globalPosition = button->mapToGlobal(QPoint(0, button->height()));
    //qDebug() << "Test ShowHelpAtButton2, position: " << globalPosition;
    _Help->showAtCursor(globalPosition);
}

void CParam::openParamWindow()
{
    if (isVisible()) {
        hide();
        emit logWindowSwitch(false);
    } else {
        show();
        emit logWindowSwitch(true);
    }
}

void CParam::debug(const QString &msg)
{
    // Définir le préfixe avec le nom de la méthode
    QString PreMsg = "[" + QString(__FUNCTION__) + "] ";
    QString finalMessage = PreMsg + msg;
    qDebug().noquote().nospace() << finalMessage;

    emit sig_logMessage(finalMessage);
}


void CParam::onSaveQuit()
{
    QSettings settings("BS-AutoGame", "CParam");
    QString appPath = ui->lineEdit_AppRoad->text();

    if (ui->rButton_1->isChecked()) {
        appPath = "C:\\Program Files\\BlueStacks_nxt\\HD-Player.exe";
    } else if (ui->rButton_2->isChecked()) {
        appPath = "D:\\LDPlayer\\LDPlayer9\\dnplayer.exe";
    }
    ui->lineEdit_AppRoad->setText(appPath); // Met à jour l'UI pour cohérence

    // Ajout de la vérification du chemin
    if (!checkPath(appPath) || !QFile::exists(appPath)) {
        QMessageBox::warning(this, "Erreur", "Le chemin spécifié ne correspond pas à un exécutable valide.");
        debug("[PARAM] Erreur : Chemin invalide ou fichier inexistant -> " + appPath);
        return;
    }

    if (!appPath.isEmpty()) {
        debug("[PARAM] Chemin sauvegardé : " + appPath);
    } else {
        debug("[PARAM] Erreur : Aucun chemin détecté !");
    }

    // Détermine l'émulateur sélectionné
    if (ui->rButton_1->isChecked()) {
        _isEmulator = 1; //BlueStacks
    } else if (ui->rButton_2->isChecked()) {
        _isEmulator = 2; //LDPlayer
    }

    _isChecked1 = ui->cB_AutoExe->isChecked(); //AutoEmul
    _isChecked2 = ui->cB_AutoExe_2->isChecked(); //AutoGame
    // Sauvegarde des paramètres
    settings.setValue("AppPath", appPath);
    settings.setValue("IsChecked1", _isChecked1);
    settings.setValue("IsChecked2", _isChecked2);
    settings.setValue("Emulator", _isEmulator);

    //qDebug() << "[INIT] AutoEmul:" << _isChecked1 << "AutoGame:" << _isChecked2;


    emit sendInitialParams(appPath, _isChecked1, _isChecked2, _isEmulator);

    if (isVisible()) {
        hide();
        emit logWindowSwitch(false);
    } else {
        hide();
        emit logWindowSwitch(false);
    }
}

void CParam::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,"Sélectionner un fichier .exe", "C:\\Program Files\\", "Fichiers Exécutables (*.exe)");  // Filtre pour les fichiers .exe

    // Vérifier si un fichier a été sélectionné
    if (!filePath.isEmpty()) {
        filePath.replace("/", "\\");
        if (!checkPath(filePath)) {
            QMessageBox::warning(this, "Erreur", "Le fichier sélectionné n'est pas un exécutable valide de BlueStacks ou LDPlayer.");
            debug("[PARAM] Fichier non valide: "+filePath);
        } else {
            ui->lineEdit_AppRoad->setText(filePath);
        }
    } else {
        // Si aucun fichier n'a été sélectionné
        debug("[PARAM] Aucun fichier sélectionné.");
    }
}

void CParam::setDefaultEmulatorPath(int emulator)
{
    if (emulator == 1) {
        ui->rButton_1->setChecked(true); // BlueStacks sélectionné
        ui->lineEdit_AppRoad->clear();
        ui->lineEdit_AppRoad->setPlaceholderText("C:\\Program Files\\BlueStacks_nxt\\HD-Player.exe");
        qDebug() << "[PARAM] BlueStacks défini comme émulateur par défaut.";
    } else if (emulator == 2) {
        ui->rButton_2->setChecked(true); // LDPlayer sélectionné
        ui->lineEdit_AppRoad->clear();
        ui->lineEdit_AppRoad->setPlaceholderText("D:\\LDPlayer\\LDPlayer9\\dnplayer.exe");
        qDebug() << "[PARAM] LDPlayer défini comme émulateur par défaut.";
    } else {
        qDebug() << "[PARAM] Erreur : Valeur d'émulateur inconnue.";
    }

    ui->lineEdit_AppRoad->update();
    ui->lineEdit_AppRoad->repaint();
}

bool CParam::checkPath(const QString &filePath)
{
    return (filePath.startsWith("C:\\Program Files\\BlueStacks_nxt\\") &&
            filePath.endsWith("HD-Player.exe", Qt::CaseInsensitive)) ||
           (filePath.startsWith("D:\\LDPlayer\\LDPlayer9\\") &&
            filePath.endsWith("dnplayer.exe", Qt::CaseInsensitive));
}
