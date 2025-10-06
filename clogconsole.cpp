#include "clogconsole.h"
#include "ui_clogconsole.h"

CLogconsole::CLogconsole(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CLogconsole)
{
    ui->setupUi(this);

    qDebug() << "CLogconsole instancié à l'adresse :" << this;

    setWindowTitle("Console Log"); // Titre de la fenêtre
    setGeometry(80, 100, 540, 200); // Position et taille de la fenêtre
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);  // Pour la fenêtre indépendante
    ui->logTextEdit->setReadOnly(true); // Empêche l'édition directe
    ui->logTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // Barre verticale si nécessaire
    ui->logTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded); // Barre horizontale si nécessaire
    ui->logTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->logTextEdit->setStyleSheet(
        "background-color: #1e1e1e; color: green; font-family: Consolas, monospace; font-size: 14px; border: none;"
    );
    ui->logTextEdit->setText("Console initié!");

}

CLogconsole::~CLogconsole()
{
    delete ui;
}


void CLogconsole::appendLog(const QString &message)
{
    static QString lastMessage;
    if (message == lastMessage) {
        return;
    }
    lastMessage = message;
    // Ajouter le message dans le QTextEdit
    ui->logTextEdit->append(message);

    // Défilement automatique vers le bas
    ui->logTextEdit->verticalScrollBar()->setValue(ui->logTextEdit->verticalScrollBar()->maximum());

}

void CLogconsole::openLogWindow()
{
    if (isVisible()) {
        hide();
        emit logWindowSwitch(false);
    } else {
        show();
        emit logWindowSwitch(true);
    }
    // S'assurer que la barre de défilement est positionnée en bas lors de l'ouverture de la fenêtre
    QScrollBar *scrollBar = ui->logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
