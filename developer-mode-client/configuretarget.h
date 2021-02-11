#ifndef CONFIGURETARGET_H
#define CONFIGURETARGET_H

#include <QtWidgets/QDialog>

#include <lib/hemeradevelopermodetarget.h>

class QAbstractButton;

namespace Ui {
    class ConfigureTarget;
}

class ConfigureTarget : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureTarget(QWidget *parent = 0);
    explicit ConfigureTarget(const QString& name, const QUrl &url, QWidget *parent = 0);
    virtual ~ConfigureTarget();

    bool isValid();

    QString targetName();

private Q_SLOTS:
    void on_actionButtonsBox_clicked(QAbstractButton * button);
    void onTargetDataChanged(const QString &text);

private:
    void reset();

    Ui::ConfigureTarget *ui;
    QString m_initialName;
};

#endif // CONFIGURETARGET_H
