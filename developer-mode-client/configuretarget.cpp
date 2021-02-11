#include "configuretarget.h"
#include "ui_configuretarget.h"
#include <hemeradevelopermodetargetmanager.h>

#include <QtGui/QCloseEvent>
#include <QtWidgets/QPushButton>
#include <QMessageBox>
#include <QtCore/QDir>

ConfigureTarget::ConfigureTarget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigureTarget)
{
    ui->setupUi(this);

    ui->targetNameLineEdit->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[0-9a-zA-Z_\\-]*")), this));

    connect(ui->targetNameLineEdit, &QLineEdit::textChanged, this, &ConfigureTarget::onTargetDataChanged);

    reset();
}



ConfigureTarget::ConfigureTarget(const QString& name, const QUrl &url, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigureTarget)
{
    ui->setupUi(this);

    // Disable name change
    ui->targetNameLineEdit->setEnabled(false);
    ui->targetNameLineEdit->setText(name);

    // Populate UI
    ui->deviceUrlLineEdit->setText(url.toString());
}

ConfigureTarget::~ConfigureTarget()
{
    delete ui;
}

bool ConfigureTarget::isValid()
{
    return !ui->targetNameLineEdit->text().isEmpty();
}

void ConfigureTarget::on_actionButtonsBox_clicked(QAbstractButton * button)
{
    if (!sender() || !button) {
        return;
    }

    switch (ui->actionButtonsBox->buttonRole(button)) {
        case QDialogButtonBox::AcceptRole: {
            Hemera::DeveloperMode::TargetManager *tm = Hemera::DeveloperMode::TargetManager::instance();
            if (m_initialName.isEmpty()) {
                if (tm->availableDevices().contains(ui->targetNameLineEdit->text())) {
                    QMessageBox::warning(this, tr("Add target failed"), tr("A target with the same name already exists! Please choose a different name."));
                }
            }
            QString pathToHsdk = QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("hsdk");

            if (tm->createStaticDevice(ui->targetNameLineEdit->text(), QUrl::fromUserInput(ui->deviceUrlLineEdit->text()), pathToHsdk)) {
                accept();
            } else {
                QMessageBox::warning(this, tr("Add target failed"), tr("There was a problem while adding your target."));
                reject();
            }
            break;
        }
        case QDialogButtonBox::RejectRole:
            reject();
            break;

        case QDialogButtonBox::ResetRole:
            reset();
            break;
    }
}

void ConfigureTarget::onTargetDataChanged(const QString &text)
{
    Q_UNUSED(text);
    ui->actionButtonsBox->button(QDialogButtonBox::Ok)->setEnabled(isValid());
}

void ConfigureTarget::reset()
{
    ui->targetNameLineEdit->setText(m_initialName);
}

QString ConfigureTarget::targetName()
{
    return ui->targetNameLineEdit->text();
}
