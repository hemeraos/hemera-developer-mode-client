#include "configuredevices.h"
#include "ui_configuredevices.h"

#include "configuretarget.h"

#include "hemeradevelopermodedevice.h"
#include "hemeradevelopermodetargetmanager.h"

#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtWidgets/QMessageBox>

#define TARGETS_MANAGER Hemera::DeveloperMode::TargetManager *tm = Hemera::DeveloperMode::TargetManager::instance();

ConfigureDevices::ConfigureDevices(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigureDevices)
{
    ui->setupUi(this);

    updateTargetsList();
    currentItemChanged();

    connect(ui->targetsListWidget, &QListWidget::currentItemChanged, this, &ConfigureDevices::currentItemChanged);
}

ConfigureDevices::~ConfigureDevices()
{
    delete ui;
}

void ConfigureDevices::currentItemChanged()
{
    enableButtons(ui->targetsListWidget->currentItem() != Q_NULLPTR);
}

void ConfigureDevices::enableButtons(bool enable)
{
    ui->editButton->setEnabled(enable);
    ui->removeButton->setEnabled(enable);
}

void ConfigureDevices::on_actionButtonsBox_accepted()
{
    accept();
}

void ConfigureDevices::on_actionButtonsBox_rejected()
{
    reject();
}

void ConfigureDevices::on_addButton_clicked()
{
//     m_configureTarget->showTargetConfig();
    ConfigureTarget ct(this);
    if (ct.exec() && ct.isValid()) {
        updateTargetsList();
    }
}

void ConfigureDevices::on_editButton_clicked()
{
    if (ui->targetsListWidget->currentItem() != Q_NULLPTR) {
        TARGETS_MANAGER;

        QListWidgetItem *item = ui->targetsListWidget->item(ui->targetsListWidget->currentRow());
        QString targetInitialName = item->text();

        Hemera::DeveloperMode::Device::Ptr target = tm->loadDevice(targetInitialName);

        if (!target.isNull()) {
            ConfigureTarget ct(targetInitialName, target->url(), this);
            if (ct.exec() && ct.isValid()) {
                updateTargetsList();
            }
        }
    }
}

void ConfigureDevices::on_removeButton_clicked()
{
    if (ui->targetsListWidget->currentItem() != Q_NULLPTR) {
        QListWidgetItem *item = ui->targetsListWidget->takeItem(ui->targetsListWidget->currentRow());
        QString targetName = item->text();

        if (Hemera::DeveloperMode::TargetManager::instance()->removeKnownDevice(targetName)) {
            updateTargetsList();

            ui->targetsListWidget->removeItemWidget(item);
            delete item;

            // check if any targets are left. Deactivate edit/remove if there are none
            if (ui->targetsListWidget->count() == 0) {
                enableButtons(false);
            }
        }
    }
}

void ConfigureDevices::updateTargetsList()
{
    TARGETS_MANAGER;

    ui->targetsListWidget->clear();

    Q_FOREACH (QString targetName, tm->availableDevices()) {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/device.png"), targetName);
        ui->targetsListWidget->addItem(item);
    }
}
