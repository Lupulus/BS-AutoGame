#include "chelp.h"
#include "qscreen.h"
#include "ui_chelp.h"

CHelp::CHelp(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::chelp)
{
    ui->setupUi(this);

    qDebug() << "CHelp instancié à l'adresse :" << this;
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    // setGeometry(800, 100, 270, 150);
    setFixedSize(270, 150);
    ui->textBrowser->setHtml("<small><u>C:\\Program Files\\BlueStacks_nxt\\HD-Player.exe</u></small>");
}

CHelp::~CHelp()
{
    delete ui;
}

void CHelp::leaveEvent(QEvent *event)
{
    hide();
    QWidget::leaveEvent(event);
}
void CHelp::showAtCursor(const QPoint &position)
{
    // Vérifier les limites de l'écran
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    QPoint adjustedPosition = position;

    if (!screenGeometry.contains(position)) {
        qDebug() << "Position hors limites, ajustement requis.";
        adjustedPosition.setX(qBound(screenGeometry.left(), position.x(), screenGeometry.right() - width()));
        adjustedPosition.setY(qBound(screenGeometry.top(), position.y(), screenGeometry.bottom() - height()));
    }
    move(position);  // Déplace la fenêtre au niveau du point spécifié
    show();          // Affiche la fenêtre d'aide
}


void CHelp::showHelpPage(int pageIndex)
{
    if (!ui || !ui->stackedWidget) {
        qDebug() << "Erreur : stackedWidget non initialisé.";
        return;
    }
    if (pageIndex ==0) {
        setFixedSize(271, 151);
    } else if (pageIndex ==1) {
        setFixedSize(271, 111);
        ui->groupBox_2->setFixedSize(269, 109);
        ui->label_2->setMaximumSize(249, 89);
    } else if (pageIndex ==2) {
        setFixedSize(271, 131);
        ui->groupBox_3->setFixedSize(269, 129);
    }
    ui->stackedWidget->setCurrentIndex(pageIndex);
    show();
}
