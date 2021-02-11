#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "configuredevices.h"
#include "starinfowidget.h"

#include <hemeradevelopermodecontroller.h>
#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodestar.h>
#include <hemeradevelopermodetarget.h>
#include <hemeradevelopermodetargetmanager.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>

#include <QtGui/QStandardItemModel>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentTarget(0)
    , m_applicationsModel(new QStandardItemModel(this))
    , m_featuresModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    connect(ui->quitAction, &QAction::triggered, qApp, &QApplication::quit);
    auto hideOrShowHandlerBox = [this] {
        ui->starBox->setVisible(ui->starBox->count() > 1);
        ui->starLabel->setVisible(ui->starBox->count() > 1);
    };
    connect(ui->starBox->model(), &QAbstractItemModel::rowsInserted, hideOrShowHandlerBox);
    connect(ui->starBox->model(), &QAbstractItemModel::rowsRemoved, hideOrShowHandlerBox);

    // Triggers for developer mode
    void (QComboBox::*currentIndexChangedSignal)(const QString &txt) = &QComboBox::currentIndexChanged;
    connect(ui->advancedModeCheckBox, &QCheckBox::toggled, this, &MainWindow::onAdvancedModeToggled);
    connect(ui->starBox, currentIndexChangedSignal, this, &MainWindow::onStarBoxIndexChanged);
    connect(ui->startStopButton, &QPushButton::clicked, [this] {
        // First of all, disable the button so we don't get duplicate events.
        ui->startStopButton->setEnabled(false);
        ui->developmentTabWidget->setEnabled(false);
        // Let's make sure we're not fucking up.
        if (m_currentTarget->developerModeController()->statusOf(ui->starBox->currentText()) == Hemera::DeveloperMode::Controller::Status::Stopped) {
            if (ui->advancedModeCheckBox->isChecked()) {
                //m_currentTarget->developerModeController()->startAdvanced();
            } else {
                m_currentTarget->developerModeController()->startSimple(ui->starBox->currentText(), ui->applicationComboBox->currentText());
            }
        } else if (m_currentTarget->developerModeController()->statusOf(ui->starBox->currentText()) == Hemera::DeveloperMode::Controller::Status::Running) {
            m_currentTarget->developerModeController()->stop(ui->starBox->currentText());
        } else {
            // Ouch.
            ui->startStopButton->setEnabled(true);
            qDebug() << "error status";
            return;
        }
    });

    updateTargetsList();

    // Add features
    QStandardItem *feature;
    feature = new QStandardItem(tr("Audio"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Audio));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Video"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Video));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Serial Ports"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::SerialPorts));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Console"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Console));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Printers"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Printers));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Disks"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Disks));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Hyperspace"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Hyperspace));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Network"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::Network));
    m_featuresModel->appendRow(feature);

    feature = new QStandardItem(tr("Check for Updates"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::CheckForUpdates));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Download Updates"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::DownloadUpdates));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Update Applications"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::UpdateApplications));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Install Applications"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::InstallApplications));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Remove Applications"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::RemoveApplications));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Manage Software Repositories"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::ManageSoftwareRepositories));
    m_featuresModel->appendRow(feature);
    feature = new QStandardItem(tr("Update System"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::UpdateSystem));
    m_featuresModel->appendRow(feature);

    feature = new QStandardItem(tr("Software Keyboard"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::SoftwareKeyboard));
    m_featuresModel->appendRow(feature);

    feature = new QStandardItem(tr("Legacy Devices"));
    feature->setData(static_cast<int>(Hemera::DeveloperMode::Controller::LegacyDevices));
    m_featuresModel->appendRow(feature);

    // Assign models
    ui->featuresView->setModel(m_featuresModel);
    ui->applicationsView->setModel(m_applicationsModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::aboutQt()
{
    QApplication::aboutQt();
}

void MainWindow::on_aboutAction_triggered()
{
    QString aboutText = QString(tr("<h3>Hemera Developer Mode Client %1.%2.%3</h3>"
                        "<p>Copyright 2013-2014 Ispirata S.r.l.. All rights reserved.</p>"
                        "<p>Hemera Developer Mode Client is part of the <a href=\"%4\">Hemera</a> SDK. See the <a href=\"%5\">documentation</a> "
                        "for more information.</p>"
                        "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, "
                        "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"))
                    .arg(VERSION_MAJOR)
                    .arg(VERSION_MINOR)
                    .arg(VERSION_PATCH)
                    .arg("http://www.hemera.io/")
                    .arg("http://doc.hemera.io/");

    QMessageBox::about(this, tr("About Hemera Developer Mode Client"), aboutText);
}

void MainWindow::on_configureTargetsButton_clicked()
{
    ConfigureDevices cd;
    cd.exec();
    updateTargetsList();
}

void MainWindow::on_targetsComboBox_currentIndexChanged(const QString &text)
{
    TARGETS_MANAGER;

    // Disconnect, in case
    if (!m_currentTarget.isNull()) {
        if (m_currentTarget->hasAcquiredDeveloperModeController()) {
            disconnect(m_currentTarget->developerModeController().data(), &Hemera::DeveloperMode::Controller::statusChanged, this, &MainWindow::developerModeStatusChanged);
        }
    }

    m_currentTarget.clear();

    m_currentTarget = tm->loadDevice(text);
    onTargetChanged();

    if (m_currentTarget.isNull()) {
        return;
    }

    connect(m_currentTarget.data(), &Hemera::DeveloperMode::Target::onlineChanged,     this, &MainWindow::onTargetChanged);
    connect(m_currentTarget.data(), &Hemera::DeveloperMode::Target::targetInfoChanged, this, &MainWindow::onTargetChanged);
}

void MainWindow::onTargetChanged()
{
    if (m_currentTarget.isNull()) {
        ui->txtTargetStatus->setText("");
    } else {
        ui->txtTargetStatus->setText(m_currentTarget->isOnline() ? "ONLINE" : "OFFLINE");
    }

    updateTargetDetails();
}

void MainWindow::onAdvancedModeToggled(bool checked)
{
    // If we are switching to simple, we have to check whether our combobox has items at all (or there's an active injection).
    bool disable = !checked && ui->applicationComboBox->count() <= 0;
    if (!m_currentTarget.isNull()) {
        disable = disable && m_currentTarget->developerModeController()->statusOf(ui->starBox->currentText()) != Hemera::DeveloperMode::Controller::Status::Running;
    }
    qDebug() << "Start stop button disable: " << disable;
    ui->startStopButton->setDisabled(disable);
}

void MainWindow::quit()
{
    QApplication::quit();
}

void MainWindow::updateDeveloperModeDetails()
{
    if (m_currentTarget.isNull() || !m_currentTarget->isOnline()) {
        // Nothing to do here.
        qDebug() << "Not updating devmode";
        return;
    }

    ui->developmentTabWidget->setEnabled(m_currentTarget->hasAcquiredDeveloperModeController());

    connect(m_currentTarget->developerModeController().data(), &Hemera::DeveloperMode::Controller::statusChanged, this, &MainWindow::developerModeStatusChanged);
}

void MainWindow::developerModeStatusChanged(const QString &star, Hemera::DeveloperMode::Controller::Status status)
{
    // Follow up only if there's a match
    if (star != ui->starBox->currentText()) {
        return;
    }

    switch (status) {
        case Hemera::DeveloperMode::Controller::Status::Running:
            ui->developmentTabWidget->setEnabled(true);
            // If it is running, we have to disable everything but the button.
            ui->applicationComboBox->setEnabled(false);
            ui->applicationsView->setEnabled(false);
            ui->featuresView->setEnabled(false);
            ui->advancedModeCheckBox->setEnabled(false);
            // DO NOT BREAK!!
        case Hemera::DeveloperMode::Controller::Status::Stopped:
            // Stable
            ui->startStopButton->setEnabled(true);
            if (status == Hemera::DeveloperMode::Controller::Status::Stopped) {
                ui->developmentTabWidget->setEnabled(true);
                ui->applicationComboBox->setEnabled(true);
                ui->applicationsView->setEnabled(true);
                ui->featuresView->setEnabled(true);
                ui->advancedModeCheckBox->setEnabled(true);
            }
            ui->startStopButton->setText(status == Hemera::DeveloperMode::Controller::Status::Stopped ? tr("Start") : tr("Stop"));
            // Update UX as well
            onAdvancedModeToggled(ui->advancedModeCheckBox->isChecked());
            break;
        default:
            // Unstable
            ui->startStopButton->setEnabled(false);
            ui->developmentTabWidget->setEnabled(false);
            ui->startStopButton->setText(tr("Switching..."));
            break;
    }
}

void MainWindow::updateTargetDetails()
{
    ui->developmentTabWidget->setDisabled(true);

    ui->txtBoardId->setText("");
    ui->txtBoardName->setText("");
    ui->txtBoardType->setText("");
    ui->txtSystemCpu->setText("");
    ui->txtSystemArch->setText("");
    ui->txtSystemMemory->setText("");
    ui->txtSystemRelease->setText("");

    ui->applicationComboBox->clear();
    ui->starBox->clear();
    m_applicationsModel->clear();

    qDeleteAll(m_starInfoWidgets);
    m_starInfoWidgets.clear();
    m_activeOrbits.clear();

    if (!m_currentTarget.isNull() && m_currentTarget->isOnline()) {
        // CPU frequency
        int freqValue = m_currentTarget->cpuFrequency();
        QString freqStr;

        if (freqValue > (1024*1024)) {
            freqStr = QString("%1.%2 GHz").arg(freqValue / (1024*1024)).arg(qRound((freqValue % (1024*1024)) / 10000.0));
        } else if (freqValue % 1024) {
            freqStr = QString("%1 MHz").arg(freqValue / 1024);
        } else {
            freqStr = QString("%1 KHz").arg(freqValue);
        }

        ui->txtBoardId->setText(m_currentTarget->id());
//        ui->txtBoardName->setText(m_currentTarget->applianceName());
        ui->txtHardwareType->setText(m_currentTarget->targetName());
//        ui->txtBoardType->setText(m_currentTarget->isProductionBoard() ? tr("No") : tr("Yes"));
        ui->txtSystemCpu->setText(QString("%1 x %2").arg(m_currentTarget->availableCores()).arg(freqStr));
//        ui->txtSystemArch->setText(m_currentTarget->architecture());

        QString memStr;
        int memValue = m_currentTarget->totalMemory();
        if (memValue > (1024*1024*1024)) {
            memStr = QString("%1.%2 GB").arg(memValue / (1024*1024*1024)).arg(qRound((memValue % (1024*1024*1024)) / 10000000.0));
        } else {
            memStr = QString("%1.%2 MB").arg(memValue / (1024*1024)).arg(qRound((memValue % (1024*1024)) / 10000.0));
        }

        ui->txtSystemMemory->setText(memStr);
        ui->txtSystemRelease->setText(m_currentTarget->hemeraRelease());

//        ui->developmentTabWidget->setDisabled(m_currentTarget->isProductionBoard());
        ui->developmentTabWidget->setDisabled(false);

        for (const QString &app : m_currentTarget->installedApps()) {
            QStandardItem *item = new QStandardItem(app);
            item->setData(app);
            m_applicationsModel->appendRow(item);
        }

        // Orbit stars come last for good reason (triggers & such)
        for (const QString &star : m_currentTarget->stars()) {
            ui->starBox->addItem(star, star);

            // Create info widget
            StarInfoWidget *widget = new StarInfoWidget(m_currentTarget->star(star));
            ui->deviceInformationLayout->insertWidget(ui->deviceInformationLayout->count() - 1, widget);
            m_starInfoWidgets.insert(star, widget);
        }

        // Listen for developer mode changes
//        if (!m_currentTarget->isProductionBoard()) {
        if (true) {
            connect(m_currentTarget.data(), &Hemera::DeveloperMode::Target::developerModeControllerChanged, this, &MainWindow::updateDeveloperModeDetails);
            updateDeveloperModeDetails();
        }
    }
}

void MainWindow::updateApplicationComboBoxList()
{
    Hemera::DeveloperMode::Star *star = m_currentTarget->star(ui->starBox->currentText());
    if (star == Q_NULLPTR) {
        qWarning() << "No Star for selected star" << star;
        return;
    }

    // Do we need to do this? If we are not stable it's a really bad idea, I tell you. Unless we're empty.
    if (star->phase() != Hemera::DeveloperMode::Star::Phase::MainSequence && ui->applicationComboBox->count() > 0) {
        qDebug() << "Skipping update, unstable";
        return;
    }

    // Don't push it. Build the list, compare, and update just in case. We don't want to disrupt the UI for nothing.
    QStringList currentBoxList;
    for (int i = 0; i < ui->applicationComboBox->count(); ++i) {
        currentBoxList.append(ui->applicationComboBox->itemText(i));
    }
    QStringList nextBoxList;
    for (const QString &app : m_currentTarget->installedApps()) {
        // Wait! Before we add anything to the list, we have to verify it's not either the running or resident orbit.
        QString trimmedApp = app;
        trimmedApp.remove('.');
        if (star->activeOrbit() != trimmedApp && star->residentOrbit() != trimmedApp) {
            nextBoxList.append(app);
        }
    }

    // Compare
    bool repopulate = false;
    if (currentBoxList.count() != nextBoxList.count()) {
        // Easy
        repopulate = true;
    } else if (!currentBoxList.toSet().subtract(nextBoxList.toSet()).isEmpty()) {
        // Compared sets are different
        repopulate = true;
    }

    if (repopulate) {
        // Populate combobox (and lock remote interaction)
        QString currentApplication = ui->applicationComboBox->currentText();
        ui->applicationComboBox->clear();
        ui->applicationComboBox->addItems(nextBoxList);
        int index = ui->applicationComboBox->findText(currentApplication);
        if (index >= 0) {
            ui->applicationComboBox->setCurrentIndex(index);
        } else if (ui->applicationComboBox->count() > 0) {
            // Hm. Time for some defaults.
            ui->applicationComboBox->setCurrentIndex(0);
        }
    }
}

void MainWindow::onStarBoxIndexChanged()
{
    // First of all, do we have a valid index?
    if (ui->starBox->currentText().isEmpty()) {
        // Screw this.
        ui->developmentTabWidget->setEnabled(false);
    }

    // Kneel and disconnect
    QList< QMetaObject::Connection >::iterator i = m_starConnections.begin();
    while (i != m_starConnections.end()) {
        QObject::disconnect(*i);
        i = m_starConnections.erase(i);
    }

    Hemera::DeveloperMode::Star *star = m_currentTarget->star(ui->starBox->currentText());
    if (star == Q_NULLPTR) {
        qWarning() << "No Star for selected star" << star;
        return;
    }

    m_starConnections.append(connect(star, &Hemera::DeveloperMode::Star::activeOrbitChanged, this, &MainWindow::updateApplicationComboBoxList));

    // Oh, and of course update developer mode.
    updateDeveloperModeDetails();

    // Just to be sure.
    updateApplicationComboBoxList();
}

void MainWindow::updateTargetsList()
{
    disconnect(ui->targetsComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_targetsComboBox_currentIndexChanged(QString)));

    TARGETS_MANAGER;

    ui->targetsComboBox->clear();

    Q_FOREACH (QString targetName, tm->availableDevices()) {
        ui->targetsComboBox->addItem(targetName);
    }

    on_targetsComboBox_currentIndexChanged(ui->targetsComboBox->currentText());
    connect(ui->targetsComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_targetsComboBox_currentIndexChanged(QString)));
}
