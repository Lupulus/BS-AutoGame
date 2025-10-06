#ifndef CHELP_H
#define CHELP_H

#include <QWidget>
#include <QEvent>
#include <QPoint>

namespace Ui {
class chelp;
}

class CHelp : public QWidget
{
    Q_OBJECT

public:
    explicit CHelp(QWidget *parent = nullptr);
    ~CHelp();
    void showHelpPage(int pageIndex);
    void showAtCursor(const QPoint &position);
public slots:

private:
    Ui::chelp *ui;
signals:
    void helpWindowSwitch(bool isVisible);
protected:
    void leaveEvent(QEvent *event) override;
};

#endif // CHELP_H
