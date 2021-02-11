#include "hemeraconfigurationpages.h"

#include "hemerawizards.h"

#include "ui_hemeradevicesetupwidget.h"
#include "ui_hemeradeviceassociatewidget.h"
#include "ui_hemeraemulatorsetupwidget.h"
#include "ui_genericprogresswidget.h"

#include <hemeradevelopermodedevice.h>
#include <hemeradevelopermodeemulator.h>
#include <hemeradevelopermodehyperdiscoveryclient.h>
#include <hemeradevelopermodeoperation.h>
#include <hemeradevelopermodetargetmanager.h>

#include <QtCore/QTimer>

#include <QtGui/QFontDatabase>
#include <QtGui/QPainter>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

namespace Hemera {
namespace Internal {

HemeraDeviceConfigureWizardPage::HemeraDeviceConfigureWizardPage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::HemeraDeviceSetupWidget)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->devicesListView->setModel(m_model);
    ui->devicesListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->devicesListView->setItemDelegate(new DeviceItemDelegate(ui->devicesListView));

    ui->deviceNameLineEdit->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[a-zA-Z0-9_-.]*")), this));

    setTitle(tr("Select Device"));

    // Register Wizard fields
    qRegisterMetaType<QItemSelectionModel*>();
    registerField(QStringLiteral("device.configuration.selection"), this, "selectionModel");
    registerField(QStringLiteral("device.configuration.name"), ui->deviceNameLineEdit);
}

HemeraDeviceConfigureWizardPage::~HemeraDeviceConfigureWizardPage()
{
    delete ui;
}

QItemSelectionModel *HemeraDeviceConfigureWizardPage::selectionModel() const
{
    return ui->devicesListView->selectionModel();
}

void HemeraDeviceConfigureWizardPage::initializePage()
{
    // Start discovering!
    m_discoveryClient = new DeveloperMode::HyperDiscoveryClient(this);
    connect(m_discoveryClient, &DeveloperMode::HyperDiscoveryClient::capabilityDiscovered, [this] (QByteArray capability, const QHostAddress &address, int port, int ttl) {
        // TODO: Handle TTL decently for device discovery.
        Q_UNUSED(ttl);

        QString hardwareId = QString::fromLatin1(capability);
        hardwareId.remove(QStringLiteral("hwid."));

        // Ok, we have the hardware ID. We have to add the device only if it is not registered yet.
        QString deviceName = Hemera::DeveloperMode::TargetManager::deviceNameFromQuery(hardwareId);
        if (!deviceName.isEmpty()) {
            // It's already registered, not interesting.
            return;
        }

        // Good! Construct the URL.
        // TODO/FIXME: Do not hardcode the protocol!!!
        QUrl deviceUrl;
        deviceUrl.setScheme(QStringLiteral("https"));
        deviceUrl.setHost(address.toString());
        deviceUrl.setPort(port);
        addDeviceById(hardwareId, tr("Generic Hemera Device"), DeveloperMode::Device::DeviceType::TabletLike, deviceUrl, true);
    });

    // Keep scanning every 10 seconds
    QTimer *timer = new QTimer(this);
    timer->setInterval(10000);
    connect(timer, &QTimer::timeout, [this] { m_discoveryClient->scanCapabilities(QList<QByteArray>() << "hwid.*"); });
    // From now.
    m_discoveryClient->scanCapabilities(QList<QByteArray>() << "hwid.*");

    // Complete changed.
    Q_EMIT completeChanged();
    connect(ui->deviceNameLineEdit, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &QWizardPage::completeChanged);
}

bool HemeraDeviceConfigureWizardPage::validatePage()
{
    if (Hemera::DeveloperMode::TargetManager::availableDevices().contains(ui->deviceNameLineEdit->text())) {
        QMessageBox::warning(this, tr("Device already exists"), tr("Device %1 is apparently already in Hemera's system. "
                                                                   "Please choose a different one.").arg(ui->deviceNameLineEdit->text()));
        return false;
    }

    return QWizardPage::validatePage();
}

bool HemeraDeviceConfigureWizardPage::isComplete() const
{
    return selectionModel()->currentIndex().isValid() && !ui->deviceNameLineEdit->text().isEmpty();
}

void HemeraDeviceConfigureWizardPage::cleanupPage()
{
    // Stop discovering
    m_discoveryClient->deleteLater();
    m_model->clear();
}

void HemeraDeviceConfigureWizardPage::addDeviceById(const QString &id, const QString &name, DeveloperMode::Device::DeviceType deviceType, const QUrl &url, bool discovered)
{
    // Do we already have an item with the same hardware ID?
    if (!m_model->findItems(id).isEmpty()) {
        // Skip.
        return;
    }

    // Create!
    QStandardItem *item = new QStandardItem(QIcon(), id);
    item->setData(id, HardwareIdRole);
    item->setData(name, GenericNameRole);
    item->setData(static_cast<int>(deviceType), DeviceTypeRole);
    item->setData(QPixmap(QStringLiteral(":/pixmaps/icons/audio-card.png")), DeviceIconRole);
    item->setData(discovered, IsDeviceDiscoveredRole);
    item->setData(url, DeviceURL);

    // Add to model. Delegate will take care of the rest.
    m_model->appendRow(item);
}


HemeraDeviceAssociateWizardPage::HemeraDeviceAssociateWizardPage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::HemeraDeviceAssociateWidget)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    ui->emulatorListView->setModel(m_model);
    ui->emulatorListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->emulatorListView->setItemDelegate(new DeviceItemDelegate(ui->emulatorListView));

    setTitle(tr("Associate to Emulator"));

    // Register Wizard fields
    qRegisterMetaType<QItemSelectionModel*>();
    registerField(QStringLiteral("device.associate.selection"), this, "selectionModel");

    connect(ui->addEmulatorButton, &QPushButton::clicked, [this] {
        // Create it from
        HemeraEmulatorConfigurationWizard wizard(this);
        wizard.exec();
        populateEmulatorModel();
    });
}

HemeraDeviceAssociateWizardPage::~HemeraDeviceAssociateWizardPage()
{
    delete ui;
}

QItemSelectionModel *HemeraDeviceAssociateWizardPage::selectionModel() const
{
    return ui->emulatorListView->selectionModel();
}

void HemeraDeviceAssociateWizardPage::initializePage()
{
    // Repopulate the Emulators' model.
    populateEmulatorModel();

    // Complete changed.
    Q_EMIT completeChanged();
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &QWizardPage::completeChanged);
}

void HemeraDeviceAssociateWizardPage::populateEmulatorModel()
{
    // First of all, clear the old crap.
    m_model->clear();

    // Now get dem emulators and add them.
    QHash< QString, QString > emulators = Hemera::DeveloperMode::TargetManager::registeredEmulators();

    for (QHash< QString, QString >::const_iterator i = emulators.constBegin(); i != emulators.constEnd(); ++i) {
        // Create!
        QStandardItem *item = new QStandardItem(QIcon(), i.key());
        item->setData(i.value(), HardwareIdRole);
        item->setData(i.key(), GenericNameRole);
        item->setData(0, DeviceTypeRole);
        item->setData(QPixmap(QStringLiteral(":/pixmaps/icons/computer.png")), DeviceIconRole);
        item->setData(false, IsDeviceDiscoveredRole);

        // Add to model. Delegate will take care of the rest.
        m_model->appendRow(item);
    }
}

bool HemeraDeviceAssociateWizardPage::isComplete() const
{
    return selectionModel()->currentIndex().isValid();
}

void HemeraDeviceAssociateWizardPage::cleanupPage()
{
    m_model->clear();
}


HemeraDeviceCreateWizardPage::HemeraDeviceCreateWizardPage(const QString &hsdk, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::GenericProgressWidget)
    , m_hsdk(hsdk)
{
    ui->setupUi(this);

    setTitle(tr("Add and associate Device"));

    // Register Wizard fields
    registerField(QStringLiteral("device.create.name"), this, "deviceName", SIGNAL(deviceNameChanged()));
}

HemeraDeviceCreateWizardPage::~HemeraDeviceCreateWizardPage()
{
    delete ui;
}

void HemeraDeviceCreateWizardPage::initializePage()
{
    // Let's start!
    ui->detailsLabel->setText(tr("Verifying configuration..."));
    ui->progressBar->setValue(0);

    // Process
    QTimer::singleShot(0, this, SLOT(startProcessing()));
}

void HemeraDeviceCreateWizardPage::startProcessing()
{
    // Is everything alright? We need everything in place.
    QString deviceName = field(QStringLiteral("device.configuration.name")).toString();
    QItemSelectionModel *deviceSelector = field(QStringLiteral("device.configuration.selection")).value<QItemSelectionModel*>();
    QModelIndex selectedDevice = deviceSelector->currentIndex();
    QItemSelectionModel *emulatorSelector = field(QStringLiteral("device.associate.selection")).value<QItemSelectionModel*>();
    QModelIndex selectedEmulator = emulatorSelector->currentIndex();

    if (!selectedDevice.isValid() || !selectedEmulator.isValid() || deviceName.isEmpty() ||
        Hemera::DeveloperMode::TargetManager::availableDevices().contains(deviceName)) {
        finishedWithError(tr("Some of the selected values are invalid! Could not create the device."));
        return;
    }

    // Verify the selection is consistent
    QString emulatorName = DeveloperMode::TargetManager::emulatorNameFromQuery(emulatorSelector->model()->data(selectedEmulator, GenericNameRole).toString());
    if (emulatorName.isEmpty()) {
        finishedWithError(tr("You have chosen an invalid emulator! Could not create the device."));
        return;
    }

    ui->detailsLabel->setText(tr("Adding device to system configuration..."));
    ui->progressBar->setValue(20);
    // Create the device
    if (deviceSelector->model()->data(selectedDevice, IsDeviceDiscoveredRole).toBool()) {
        // If it is discovered...
        if (!DeveloperMode::TargetManager::instance()->createKnownDevice(deviceName, deviceSelector->model()->data(selectedDevice, HardwareIdRole).toString(), m_hsdk)) {
            finishedWithError(tr("Could not create the device! This might be a serious problem in your Hemera SDK installation."));
            return;
        }
    } else {
        if (!DeveloperMode::TargetManager::instance()->createStaticDevice(deviceName, deviceSelector->model()->data(selectedDevice, DeviceURL).toUrl(), m_hsdk)) {
            finishedWithError(tr("Could not create the device! This might be a serious problem in your Hemera SDK installation."));
            return;
        }
    }

    // Hello, new device!
    DeveloperMode::Device::Ptr device = DeveloperMode::TargetManager::instance()->loadDevice(deviceName);
    if (device.isNull()) {
        finishedWithError(tr("The created device could not be loaded! This might be a serious problem in your Hemera SDK installation."));
        DeveloperMode::TargetManager::instance()->removeKnownDevice(deviceName);
        return;
    }

    ui->detailsLabel->setText(tr("Connecting to device..."));
    ui->progressBar->setValue(40);

    if (!device->waitForTargetInfo(25000)) {
        finishedWithError(tr("Could not connect to device! Please verify its connectivity and try again."));
        DeveloperMode::TargetManager::instance()->removeKnownDevice(deviceName);
        return;
    }

    ui->detailsLabel->setText(tr("Starting emulator..."));
    ui->progressBar->setValue(60);

    DeveloperMode::Emulator::Ptr emulator = DeveloperMode::TargetManager::instance()->loadEmulator(emulatorName);
    DeveloperMode::Operation *operation = emulator->start();
    if (!operation) {
        finishedWithError(tr("Could not start emulator %1!").arg(emulatorName));
        DeveloperMode::TargetManager::instance()->removeKnownDevice(deviceName);
        return;
    }
    if (!operation->synchronize(15000)) {
        finishedWithError(tr("Could not start emulator %1! Details: %2: %3").arg(emulatorName, operation->errorName(), operation->errorMessage()));
        DeveloperMode::TargetManager::instance()->removeKnownDevice(deviceName);
        return;
    }

    ui->detailsLabel->setText(tr("Connecting to emulator..."));
    ui->progressBar->setValue(75);

    if (!emulator->waitForTargetInfo(25000)) {
        finishedWithError(tr("Could not connect to emulator! You might need to reinstall it again."));
        DeveloperMode::TargetManager::instance()->removeKnownDevice(deviceName);
        return;
    }

    ui->detailsLabel->setText(tr("Associating device..."));
    ui->progressBar->setValue(90);

    if (!DeveloperMode::TargetManager::instance()->associate(device, emulator)) {
        finishedWithError(tr("Could not associate %1 to %2! Most likely, this means %2 is not capable of compiling for target %1. "
                             "Choose a different emulator and try again.").arg(deviceName, emulatorName));
        DeveloperMode::TargetManager::instance()->removeKnownDevice(deviceName);
        return;
    }

    ui->detailsLabel->setText(tr("Success!"));
    ui->progressBar->setValue(100);

    // Set the device name!
    m_deviceName = deviceName;
    finished();
}

void HemeraDeviceCreateWizardPage::finishedWithError(const QString &error)
{
    ui->errorLabel->setText(error);
    m_complete = true;
    Q_EMIT completeChanged();
}

void HemeraDeviceCreateWizardPage::finished()
{
    m_complete = true;
    Q_EMIT completeChanged();
}

bool HemeraDeviceCreateWizardPage::isComplete() const
{
    return m_complete;
}

QString HemeraDeviceCreateWizardPage::deviceName() const
{
    return m_deviceName;
}

void HemeraDeviceCreateWizardPage::setDeviceName(const QString &deviceName)
{
    if (m_deviceName == deviceName) {
        return;
    }

    m_deviceName = deviceName;
    Q_EMIT deviceNameChanged();
}

/////// EMULATOR


HemeraEmulatorConfigureWizardPage::HemeraEmulatorConfigureWizardPage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::HemeraEmulatorSetupWidget)
    , m_existingVMModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->vBoxListView->setModel(m_existingVMModel);
    ui->vBoxListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->vBoxListView->setItemDelegate(new DeviceItemDelegate(ui->vBoxListView));

    ui->emulatorNameLineEdit->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[a-zA-Z0-9_-.]*")), this));

    setTitle(tr("Select Emulator Image"));

    // Register Wizard fields
    qRegisterMetaType<QItemSelectionModel*>();
    registerField(QStringLiteral("emulator.configuration.name"), ui->emulatorNameLineEdit);
    registerField(QStringLiteral("emulator.configuration.mode"), this, "chosenMode");
    registerField(QStringLiteral("emulator.configuration.id"), this, "selectedId");
}

HemeraEmulatorConfigureWizardPage::~HemeraEmulatorConfigureWizardPage()
{
    delete ui;
}

QItemSelectionModel *HemeraEmulatorConfigureWizardPage::existingVMSelectionModel() const
{
    return ui->vBoxListView->selectionModel();
}

void HemeraEmulatorConfigureWizardPage::initializePage()
{
    // Add existing (unregistered) VMs
    QStringList registeredEmulatorsIds = Hemera::DeveloperMode::TargetManager::registeredEmulators().values();
    QHash< QString, QString > availableVMs = Hemera::DeveloperMode::TargetManager::availableVirtualMachines();
    for (QHash< QString, QString >::const_iterator i = availableVMs.constBegin(); i != availableVMs.constEnd(); ++i) {
        if (!registeredEmulatorsIds.contains(i.value())) {
            addExistingVM(i.key(), i.value());
        }
    }

    // Complete changed.
    Q_EMIT completeChanged();
    connect(ui->emulatorNameLineEdit, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
    connect(ui->vdiFileLineEdit, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
    connect(ui->startTokenLineEdit, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
    connect(ui->vdiFileRadio, &QRadioButton::toggled, this, &QWizardPage::completeChanged);
    connect(ui->startTokenRadio, &QRadioButton::toggled, this, &QWizardPage::completeChanged);
    connect(ui->existingVBoxRadio, &QRadioButton::toggled, this, &QWizardPage::completeChanged);
    connect(existingVMSelectionModel(), &QItemSelectionModel::selectionChanged, this, &QWizardPage::completeChanged);

    // File selection
    connect(ui->browseVDIButton, &QPushButton::clicked, [this] {
        QString file = QFileDialog::getOpenFileName(this, tr("Select an emulator image to install"), QString(),
                                                    tr("Hemera Emulator Images (*.vdi *.vdi.bz2 *.vdi.7z)"));
        if (file.isEmpty()) {
            return;
        }

        ui->vdiFileLineEdit->setText(file);
    });
}

bool HemeraEmulatorConfigureWizardPage::validatePage()
{
    if (Hemera::DeveloperMode::TargetManager::registeredEmulators().contains(ui->emulatorNameLineEdit->text())) {
        QMessageBox::warning(this, tr("Emulator already exists"), tr("Emulator %1 is apparently already in Hemera's system. "
                                                                     "Please choose a different one.").arg(ui->emulatorNameLineEdit->text()));
        return false;
    }

    // Then we set our correct variable
    if (ui->vdiFileRadio->isChecked()) {
        if (!QFile::exists(ui->vdiFileLineEdit->text())) {
            QMessageBox::warning(this, tr("Invalid file"), tr("File %1 does not exist!").arg(ui->vdiFileLineEdit->text()));
            return false;
        }

        m_chosenMode = EmulatorInstallFromLocalVDI;
        m_selectedId = ui->vdiFileLineEdit->text();
    } else if (ui->startTokenRadio->isChecked()) {
        m_chosenMode = EmulatorInstallFromStart;
        m_selectedId = ui->startTokenLineEdit->text();
    } else if (ui->existingVBoxRadio->isChecked()) {
        m_chosenMode = EmulatorInstallConvertExistingVM;
        m_selectedId = existingVMSelectionModel()->currentIndex().data(HardwareIdRole).toString();
    }

    return QWizardPage::validatePage();
}

bool HemeraEmulatorConfigureWizardPage::isComplete() const
{
    // Check completeness
    bool installFromLocalComplete = ui->vdiFileRadio->isChecked() && !ui->vdiFileLineEdit->text().isEmpty();
    bool installFromStartComplete = ui->startTokenRadio->isChecked() && !ui->startTokenLineEdit->text().isEmpty();
    bool installFromExistingComplete = ui->existingVBoxRadio->isChecked() && existingVMSelectionModel()->currentIndex().isValid();
    return !ui->emulatorNameLineEdit->text().isEmpty() && (installFromLocalComplete || installFromStartComplete || installFromExistingComplete);
}

void HemeraEmulatorConfigureWizardPage::cleanupPage()
{
    m_existingVMModel->clear();
}

void HemeraEmulatorConfigureWizardPage::addExistingVM(const QString &name, const QString &id)
{
    // Do we already have an item with the same VBox ID?
    if (!m_existingVMModel->findItems(id).isEmpty()) {
        // Skip.
        return;
    }

    // Create!
    QStandardItem *item = new QStandardItem(QIcon(), id);
    item->setData(id, HardwareIdRole);
    item->setData(name, GenericNameRole);
    item->setData(QPixmap(QStringLiteral(":/pixmaps/icons/computer.png")), DeviceIconRole);

    // Add to model. Delegate will take care of the rest.
    m_existingVMModel->appendRow(item);
}


HemeraEmulatorCreateWizardPage::HemeraEmulatorCreateWizardPage(const QString &hsdk, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::GenericProgressWidget)
    , m_hsdk(hsdk)
{
    ui->setupUi(this);

    setTitle(tr("Create and install Emulator"));

    // Register Wizard fields
    registerField(QStringLiteral("emulator.create.name"), this, "emulatorName", SIGNAL(emulatorNameChanged()));
}

HemeraEmulatorCreateWizardPage::~HemeraEmulatorCreateWizardPage()
{
    delete ui;
}

void HemeraEmulatorCreateWizardPage::initializePage()
{
    // Let's start!
    ui->detailsLabel->setText(tr("Creating installation procedure..."));
    ui->progressBar->setValue(0);

    // Process
    QTimer::singleShot(0, this, SLOT(startProcessing()));
}

void HemeraEmulatorCreateWizardPage::startProcessing()
{
    // Is everything alright? We need everything in place.
    QString name = field(QStringLiteral("emulator.configuration.name")).toString();
    EmulatorInstallMode installMode = static_cast<EmulatorInstallMode>(field(QStringLiteral("emulator.configuration.mode")).toUInt());
    QString installId = field(QStringLiteral("emulator.configuration.id")).toString();

    if (installMode == EmulatorInstallUnknownMode || name.isEmpty() || installId.isEmpty()) {
        finishedWithError(tr("Some of the selected values are invalid! Could not create the emulator."));
        return;
    }

    Hemera::DeveloperMode::Operation *op = nullptr;

    switch (installMode) {
    case EmulatorInstallFromLocalVDI:
        op = Hemera::DeveloperMode::TargetManager::instance()->installEmulatorFromVDI(name, installId, m_hsdk);
        ui->detailsLabel->setText(tr("Installing Emulator..."));
        break;
    case EmulatorInstallFromStart:
        op = Hemera::DeveloperMode::TargetManager::instance()->installEmulatorFromStartToken(name, installId, QUrl(), m_hsdk);
        ui->detailsLabel->setText(tr("Downloading Emulator..."));
        break;
    case EmulatorInstallConvertExistingVM:
        break;
    default:
        finishedWithError(tr("Tried to install an emulator without specifying a mode! This is an internal error which should be reported."));
        return;
    }

    if (!op) {
        finishedWithError(tr("Could not configure install operation!"));
    }

    connect(op, &Hemera::DeveloperMode::Operation::progress, ui->progressBar, &QProgressBar::setValue);
    connect(op, &Hemera::DeveloperMode::Operation::downloadInfo, [this] (quint64 downloaded, quint64 total, quint64 rate) {
        ui->progressBar->setValue((downloaded*100)/total);
        ui->detailsLabel->setText(tr("Downloading Emulator... (%1 B/s)").arg(rate));
    });
    connect(op, &Hemera::DeveloperMode::Operation::finished, [this, op, name] {
        if (op->isError()) {
            finishedWithError(tr("Installation failed! %1: %2").arg(op->errorName(), op->errorMessage()));
            return;
        }
        ui->detailsLabel->setText(tr("Success!"));
        ui->progressBar->setValue(100);
        setEmulatorName(name);
        finished();
    });
}

void HemeraEmulatorCreateWizardPage::finishedWithError(const QString &error)
{
    ui->errorLabel->setText(error);
    m_complete = true;
    Q_EMIT completeChanged();
}

void HemeraEmulatorCreateWizardPage::finished()
{
    m_complete = true;
    Q_EMIT completeChanged();
}

bool HemeraEmulatorCreateWizardPage::isComplete() const
{
    return m_complete;
}

QString HemeraEmulatorCreateWizardPage::emulatorName() const
{
    return m_emulatorName;
}

void HemeraEmulatorCreateWizardPage::setEmulatorName(const QString &emulatorName)
{
    if (m_emulatorName == emulatorName) {
        return;
    }

    m_emulatorName = emulatorName;
    Q_EMIT emulatorNameChanged();
}

/////// DELEGATE

const int SPACING = 2;
const int DEVICE_ICON_SIZE = 48;
const qreal GENERIC_ICON_OPACITY = 0.6;

DeviceItemDelegate::DeviceItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

DeviceItemDelegate::~DeviceItemDelegate()
{
}

void DeviceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);
    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(DEVICE_ICON_SIZE, DEVICE_ICON_SIZE));
    iconRect.moveTo(QPoint(iconRect.x() + SPACING, iconRect.y() + SPACING));

    QPixmap deviceIcon(qvariant_cast<QPixmap>(index.data(DeviceIconRole)));

    if (!deviceIcon.isNull()) {
        style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, deviceIcon.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // Get font information
    const QFontMetrics hardwareIdFontMetrics(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    const QFontMetrics deviceNameFontMetrics(QFontDatabase::systemFont(QFontDatabase::TitleFont));

    QRect deviceNameRect = optV4.rect;
    deviceNameRect.setX(iconRect.x() + iconRect.width() + SPACING);
    deviceNameRect.setY(deviceNameRect.y() + SPACING);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    painter->drawText(deviceNameRect,
                      deviceNameFontMetrics.elidedText(index.data(GenericNameRole).toString(), Qt::ElideRight, deviceNameRect.width()));

    // Draw Hardware ID text
    QRect hardwareIdRect = optV4.rect;
    hardwareIdRect.setX(iconRect.x() + iconRect.width() + SPACING);
    hardwareIdRect.setY(deviceNameRect.bottom() - hardwareIdFontMetrics.height() - 4);

    QColor fadingColor;
    if (option.state & QStyle::State_Selected) {
        fadingColor = QColor(option.palette.color(QPalette::Disabled, QPalette::HighlightedText));
    } else {
        fadingColor = QColor(option.palette.color(QPalette::Disabled, QPalette::Text));
    }

    painter->setPen(fadingColor);

    painter->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    painter->drawText(hardwareIdRect,
                      hardwareIdFontMetrics.elidedText(index.data(HardwareIdRole).toString(), Qt::ElideRight, hardwareIdRect.width()));

    painter->restore();
}

QSize DeviceItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, DEVICE_ICON_SIZE + 2 * SPACING);
}

bool DeviceItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(view)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return false;
}

} // namespace Internal
} // namespace Hemera

