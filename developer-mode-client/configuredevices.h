#ifndef CONFIGUREDEVICES_H
#define CONFIGUREDEVICES_H

#include <QtCore/QPair>
#include <QtWidgets/QDialog>

namespace Ui {
    class ConfigureDevices;
}

class ConfigureDevices : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureDevices(QWidget *parent = 0);
    ~ConfigureDevices();

    void loadFromConfig();

private Q_SLOTS:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_removeButton_clicked();

    void on_actionButtonsBox_accepted();
    void on_actionButtonsBox_rejected();

    void currentItemChanged();

private:
    void updateTargetsList();
    void enableButtons(bool enable);

    Ui::ConfigureDevices *ui;
};

#endif // CONFIGUREDEVICES_H
