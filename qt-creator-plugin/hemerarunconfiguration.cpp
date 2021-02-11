#include "hemerarunconfiguration.h"

#include "ui_hemerarunconfigurationwidget.h"

#include "hemerabuildconfiguration.h"
#include "hemerakitinformation.h"
#include "hemeraproject.h"
#include "hemeraconstants.h"
#include "hemeratarget.h"
#include "hemeratoolchain.h"

#include <hemeradevelopermodeapplicationoutput.h>
#include <hemeradevelopermodecontroller.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/helpmanager.h>
#include <qtsupport/qtoutputformatter.h>
#include <qtsupport/qtkitinformation.h>
#include <projectexplorer/localenvironmentaspect.h>
#include <projectexplorer/target.h>

#include <utils/pathchooser.h>
#include <utils/detailswidget.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>
#include <utils/stringutils.h>

#include <QFormLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QCheckBox>
#include <QTimer>

#include <QtCore/QJsonObject>

namespace Hemera {
namespace Internal {

// ----- CLASS HemeraRunConfiguration ----------------------------------------------------------------

HemeraRunConfiguration::HemeraRunConfiguration(ProjectExplorer::Target *parent, Core::Id id)
    : ProjectExplorer::RunConfiguration(parent, id)
{
    setDefaultDisplayName(tr("Run on Hemera Device or Emulator"));
}

HemeraRunConfiguration::~HemeraRunConfiguration()
{
}

bool HemeraRunConfiguration::fromMap(const QVariantMap &map)
{
    bool result = RunConfiguration::fromMap(map);
    if (!result) {
        return false;
    }
    m_applicationId = map.value(QLatin1String(Constants::RUN_CONFIG_APPLICATION_ID)).toString();
    m_starName = map.value(QLatin1String(Constants::RUN_CONFIG_STAR)).toString();
    return true;
}

QVariantMap HemeraRunConfiguration::toMap() const
{
    QVariantMap data = RunConfiguration::toMap();
    data.insert(QLatin1String(Constants::RUN_CONFIG_APPLICATION_ID), m_applicationId);
    data.insert(QLatin1String(Constants::RUN_CONFIG_STAR), m_starName);
    return data;
}

void HemeraRunConfiguration::setApplicationId(const QString &applicationId)
{
    m_applicationId = applicationId;
}

QString HemeraRunConfiguration::applicationId() const
{
    return m_applicationId;
}

void HemeraRunConfiguration::setStar(const QString &star)
{
    m_starName = star;
}

QString HemeraRunConfiguration::star() const
{
    return m_starName;
}

QWidget *HemeraRunConfiguration::createConfigurationWidget()
{
    return new HemeraRunConfigurationWidget(this);
}

Utils::OutputFormatter *HemeraRunConfiguration::createOutputFormatter() const
{
    return new QtSupport::QtOutputFormatter(target()->project());
}

// ----- CLASS HemeraRunConfigurationWidget  ---------------------------------------------------------

HemeraRunConfigurationWidget::HemeraRunConfigurationWidget(HemeraRunConfiguration *hemeraRunConfiguration, QWidget *parent)
    : DetailsWidget(parent)
    , ui(new Ui::HemeraRunConfigurationWidget)
    , m_hemeraRunConfiguration(hemeraRunConfiguration)
{
    setUseCheckBox(false);
    setState(Utils::DetailsWidget::Collapsed);

    QWidget *contentWidget = new QWidget(this);
    ui->setupUi(contentWidget);
    setWidget(contentWidget);

    // Find out about our device
    ProjectExplorer::IDevice::ConstPtr idevice = ProjectExplorer::DeviceKitInformation::device(hemeraRunConfiguration->target()->kit());
    HemeraTarget::ConstPtr hemeraTarget = idevice.dynamicCast<const HemeraTarget>();
    if (hemeraTarget.isNull()) {
        qWarning() << "Could not retrieve a valid Hemera target!!";
        return;
    }

    // For now, it's just easy
    ui->simpleRadio->setChecked(true);
    ui->advancedRadio->setEnabled(false);

    // Populate stored data
    ui->applicationIdEdit->setText(hemeraRunConfiguration->applicationId());
    ProjectExplorer::DeviceKitInformation::device(hemeraRunConfiguration->target()->kit());
    for (const QString &handler : hemeraTarget->target()->stars()) {
        ui->starCombo->addItem(handler);
    }
    ui->starCombo->setCurrentText(hemeraRunConfiguration->star());

    connect(ui->starCombo, &QComboBox::currentTextChanged, hemeraRunConfiguration, &HemeraRunConfiguration::setStar);
    connect(ui->starCombo, &QComboBox::currentTextChanged, this, &HemeraRunConfigurationWidget::updateSummary);

    updateSummary();
}

HemeraRunConfigurationWidget::~HemeraRunConfigurationWidget()
{
    delete ui;
}

void HemeraRunConfigurationWidget::updateSummary()
{
    if (ui->simpleRadio->isChecked()) {
        setSummaryText(tr("Running %1 on handler %2 in Simple Mode").arg(m_hemeraRunConfiguration->applicationId(),
                                                                         m_hemeraRunConfiguration->star()));
    } else {
        setSummaryText(tr("Running applications %1 on handler %2 in Advanced Mode").arg(m_hemeraRunConfiguration->applicationId(),
                                                                                        m_hemeraRunConfiguration->star()));
    }
}

// ----- CLASS HemeraRunConfigurationFactory ---------------------------------------------------------

HemeraRunConfigurationFactory::HemeraRunConfigurationFactory(QObject *parent)
    : IRunConfigurationFactory(parent)
{
    setObjectName(QLatin1String("HemeraRunConfigurationFactory"));
}

HemeraRunConfigurationFactory::~HemeraRunConfigurationFactory()
{
}

bool HemeraRunConfigurationFactory::canCreate(ProjectExplorer::Target *parent, Core::Id id) const
{
    if (!canHandle(parent)) {
        return false;
    }
    return availableCreationIds(parent).contains(id);
}

bool HemeraRunConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    return canCreate(parent, ProjectExplorer::idFromMap(map));
}

bool HemeraRunConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source) const
{
    const HemeraRunConfiguration * const rlrc
            = qobject_cast<HemeraRunConfiguration *>(source);
    return rlrc && canCreate(parent, source->id());
}

QList<Core::Id> HemeraRunConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent, CreationMode mode) const
{
    Q_UNUSED(mode)
    QList<Core::Id> result;

    if (!canHandle(parent)) {
        return result;
    }

    result << Constants::RUN_CONFIG_ID;

    return result;
}

ProjectExplorer::RunConfiguration *HemeraRunConfigurationFactory::doCreate(ProjectExplorer::Target *parent, Core::Id id)
{
    return new HemeraRunConfiguration(parent, id);
}

ProjectExplorer::RunConfiguration *HemeraRunConfigurationFactory::doRestore(ProjectExplorer::Target *parent,
                                                                            const QVariantMap &map)
{
    return new HemeraRunConfiguration(parent, ProjectExplorer::idFromMap(map));
}

ProjectExplorer::RunConfiguration *HemeraRunConfigurationFactory::clone(ProjectExplorer::Target *parent,
                                                                        ProjectExplorer::RunConfiguration *source)
{
    QTC_ASSERT(canClone(parent, source), return 0);
    return new HemeraRunConfiguration(parent, source->id());
}

bool HemeraRunConfigurationFactory::canHandle(const ProjectExplorer::Target *target) const
{
    if (!target->project()->supportsKit(target->kit()))
        return false;
    const Core::Id deviceType = ProjectExplorer::DeviceTypeKitInformation::deviceTypeId(target->kit());
    return deviceType == Hemera::Constants::HEMERA_DEVICE_TYPE || deviceType == Hemera::Constants::HEMERA_DEVICE_TYPE_EMULATOR ||
           deviceType == Hemera::Constants::HEMERA_DEVICE_TYPE_DEVICE;
}

QString HemeraRunConfigurationFactory::displayNameForId(Core::Id id) const
{
    if (id == Constants::RUN_CONFIG_ID) {
        return tr("Base Hemera Run Configuration");
    }

    return QString();
}


// ----- CLASS HemeraRunControl ---------------------------------------------------------

HemeraRunControl::HemeraRunControl(HemeraRunConfiguration *rc)
    : RunControl(rc, ProjectExplorer::NormalRunMode)
    , m_shuttingDown(false)
{
    setIcon(QLatin1String(ProjectExplorer::Constants::ICON_RUN_SMALL));
}

HemeraRunControl::~HemeraRunControl()
{
    stop();
}

void HemeraRunControl::start()
{
    qDebug() << "Starting Hemera run control!";
    // Find out about our device
    ProjectExplorer::IDevice::ConstPtr idevice = ProjectExplorer::DeviceKitInformation::device(runConfiguration()->target()->kit());
    HemeraTarget::ConstPtr hemeraTarget = idevice.dynamicCast<const HemeraTarget>();
    if (hemeraTarget.isNull()) {
        appendMessage(tr("Run Control could not find a valid Hemera Target!") + QLatin1Char('\n'), Utils::ErrorMessageFormat);
        Q_EMIT finished();
        return;
    }

    m_nativeTarget = hemeraTarget->target();

    QObject::connect(m_nativeTarget->ensureDeveloperModeController(), &DeveloperMode::Operation::finished, this, &HemeraRunControl::onDeveloperModeControllerEnsured);
}

void HemeraRunControl::onDeveloperModeControllerEnsured(DeveloperMode::Operation *operation)
{
    if (operation->isError()) {
        appendMessage(tr("Could not retrieve Developer Mode Controller for selected Hemera target!") + QLatin1Char('\n'), Utils::ErrorMessageFormat);
        Q_EMIT finished();
        return;
    }

    HemeraRunConfiguration *rc = qobject_cast<HemeraRunConfiguration*>(runConfiguration());

    // OK, we're on.
    // Application output!
    Hemera::DeveloperMode::ApplicationOutput *applicationOutput = new Hemera::DeveloperMode::ApplicationOutput(m_nativeTarget, rc->applicationId(), this);
    connect(applicationOutput, &Hemera::DeveloperMode::ApplicationOutput::newMessage, [this] (const QDateTime &, const QJsonObject &message) {
        appendMessage(message.value(QStringLiteral("message")).toString() + QLatin1Char('\n'), Utils::StdOutFormat);
    });

    m_controller = m_nativeTarget->developerModeController();

    // Do we know what to do?
    if (rc->star().isEmpty()) {
        // We still have no orbit handler configured. Let's check...
        if (m_nativeTarget->stars().count() > 1) {
            appendMessage(tr("No orbit handler configured, and more than one orbit handler available on the target. "
                             "Please configure this run control first.") + QLatin1Char('\n'), Utils::ErrorMessageFormat);
            Q_EMIT finished();
            return;
        } else if (m_nativeTarget->stars().count() == 0) {
            appendMessage(tr("No orbit handlers found in the target - the cache data is likely corrupted.") + QLatin1Char('\n'), Utils::ErrorMessageFormat);
            Q_EMIT finished();
            return;
        } else {
            // Easy peasy
            rc->setStar(m_nativeTarget->stars().first());
        }
    }

    qDebug() << "Attempting on " << rc->star();

    // Monitor what's happening here...
    // Longer timeout
    QTimer *timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->start(10000);
    connect(m_controller.data(), &Hemera::DeveloperMode::Controller::statusChanged, this, [this, rc, timeoutTimer] {
        timeoutTimer->stop();
        if (m_controller->statusOf(rc->star()) == Hemera::DeveloperMode::Controller::Status::Running) {
            appendMessage(tr("Developer mode started, application running.") + QLatin1Char('\n'), Utils::NormalMessageFormat);
            // We are now running!
            Q_EMIT started();
        } else if (m_controller->statusOf(rc->star()) == Hemera::DeveloperMode::Controller::Status::Stopped && !m_shuttingDown) {
            // Gravity has reverted.
            appendMessage(tr("Gravity could not load Orbital Application '%1'.").arg(rc->applicationId()) + QLatin1Char('\n'), Utils::ErrorMessageFormat);
            Q_EMIT finished();
        } else if (m_controller->statusOf(rc->star()) == Hemera::DeveloperMode::Controller::Status::Stopped && m_shuttingDown) {
            appendMessage(tr("Developer mode terminated successfully.") + QLatin1Char('\n'), Utils::NormalMessageFormat);

            // We're done for today.
            Q_EMIT finished();
        } else if (!m_shuttingDown) {
            // Restart timer and wait until it settles.
            timeoutTimer->start();
        }
    });

    m_controller->startSimple(rc->star(), rc->applicationId());
}

ProjectExplorer::RunControl::StopResult HemeraRunControl::stop()
{
    if (m_controller.isNull()) {
        return StoppedSynchronously;
    } else if (!m_controller->isValid()) {
        return StoppedSynchronously;
    }

    HemeraRunConfiguration *rc = qobject_cast<HemeraRunConfiguration*>(runConfiguration());
    if (!rc) {
        // It might be invalid at this point.
        return StoppedSynchronously;
    }

    m_shuttingDown = true;
    m_controller->stop(rc->star());
    return AsynchronousStop;
}

bool HemeraRunControl::isRunning() const
{
    if (m_controller.isNull()) {
        return false;
    }

    HemeraRunConfiguration *rc = qobject_cast<HemeraRunConfiguration*>(runConfiguration());
    if (!rc) {
        // It might be invalid at this point.
        return false;
    }

    return m_controller->statusOf(rc->star()) == Hemera::DeveloperMode::Controller::Status::Running;
}

QString HemeraRunControl::displayName() const
{
    HemeraRunConfiguration *rc = qobject_cast<HemeraRunConfiguration*>(runConfiguration());
    if (!rc) {
        // It might be invalid at this point.
        return QString();
    }
    return tr("%1 on %2 (on handler %3)").arg(rc->applicationId(),
                                              HemeraKitInformation::targetName(runConfiguration()->target()->kit()),
                                              rc->star());
}

// ----- CLASS HemeraRunControlFactory ---------------------------------------------------------

HemeraRunControlFactory::HemeraRunControlFactory(QObject *parent)
    : IRunControlFactory(parent)
{
}

bool HemeraRunControlFactory::canRun(ProjectExplorer::RunConfiguration *runConfiguration,
                ProjectExplorer::RunMode mode) const
{
    if (mode != ProjectExplorer::NormalRunMode && mode != ProjectExplorer::DebugRunMode &&
        mode != ProjectExplorer::CallgrindRunMode && mode != ProjectExplorer::MemcheckRunMode) {
        return false;
    }
    return qobject_cast<HemeraRunConfiguration *>(runConfiguration);
}

ProjectExplorer::RunControl *HemeraRunControlFactory::create(ProjectExplorer::RunConfiguration *runConfig,
                                                             ProjectExplorer::RunMode mode, QString *errorMessage)
{
    Q_UNUSED(errorMessage);

    if (!canRun(runConfig, mode)) {
        errorMessage = new QString(tr("This run configuration cannot be run in the chosen mode."));
        return 0;
    }

    HemeraRunConfiguration *rc = qobject_cast<HemeraRunConfiguration *>(runConfig);

    if (!rc) {
        errorMessage = new QString(tr("This run configuration is not a valid Hemera run configuration."));
        return 0;
    }

    switch (mode) {
    case ProjectExplorer::NormalRunMode:
        return new HemeraRunControl(rc);
    case ProjectExplorer::DebugRunMode:
        //return HemeraDebugSupport::createDebugRunControl(rc, errorMessage);
    case ProjectExplorer::QmlProfilerRunMode:
    case ProjectExplorer::NoRunMode:
    case ProjectExplorer::DebugRunModeWithBreakOnMain:
    case ProjectExplorer::CallgrindRunMode:
        //return HemeraDebugSupport::createDebugRunControl(rc, errorMessage);
    case ProjectExplorer::MemcheckRunMode:
        //return HemeraDebugSupport::createDebugRunControl(rc, errorMessage);
    case ProjectExplorer::ClangStaticAnalyzerMode:
    default:
        QTC_CHECK(false); // The other run modes are not supported
    }

    return 0;
}

} // namespace Internal
} // namespace Hemera

