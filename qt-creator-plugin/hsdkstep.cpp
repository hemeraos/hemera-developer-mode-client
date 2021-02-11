#include "hsdkstep.h"

#include "hemerabuildconfiguration.h"
#include "hemeraconstants.h"
//#include "hemeraparser.h"
#include "hemeraproject.h"
#include "hemeratoolchain.h"

#include <coreplugin/icore.h>

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/target.h>
#include <projectexplorer/toolchain.h>

#include <qtsupport/qtkitinformation.h>
#include <qtsupport/qtparser.h>

#include <utils/qtcprocess.h>

#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>

namespace Hemera {
namespace Internal {

HsdkStep::HsdkStep(ProjectExplorer::BuildStepList *bsl)
    : AbstractProcessStep(bsl, Core::Id(Hemera::Constants::HSDK_STEP_ID))
    , m_clean(false)
{
    ctor();
}

HsdkStep::HsdkStep(ProjectExplorer::BuildStepList *bsl, const Core::Id id)
    : AbstractProcessStep(bsl, id)
    , m_clean(false)
{
    ctor();
}

HsdkStep::HsdkStep(ProjectExplorer::BuildStepList *bsl, HsdkStep *bs)
    : AbstractProcessStep(bsl, bs)
    , m_clean(bs->m_clean)
    , m_buildTargets(bs->m_buildTargets)
    , m_additionalArguments(bs->m_additionalArguments)
{
    ctor();
}

void HsdkStep::ctor()
{
    m_percentProgress = QRegExp(QLatin1String("^\\[\\s*(\\d*)%\\]"));
    m_ninjaProgress = QRegExp(QLatin1String("^\\[\\s*(\\d*)/\\s*(\\d*)"));
    m_ninjaProgressString = QLatin1String("[%f/%t "); // ninja: [33/100
    //: Default display name for the hemera hsdk step.
    setDefaultDisplayName(tr("hsdk"));

    HemeraBuildConfiguration *bc = hemeraBuildConfiguration();
    if (bc) {
        m_activeConfiguration = 0;
    } else {
        // That means the step is in the deploylist, so we listen to the active build config
        // changed signal and react to the activeBuildConfigurationChanged() signal of the buildconfiguration
        m_activeConfiguration = targetsActiveBuildConfiguration();
        connect(target(), SIGNAL(activeBuildConfigurationChanged(ProjectExplorer::BuildConfiguration*)), this, SLOT(activeBuildConfigurationChanged()));
        activeBuildConfigurationChanged();
    }

    connect(static_cast<HemeraProject *>(project()), SIGNAL(buildTargetsChanged()), this, SLOT(buildTargetsChanged()));
}

HsdkStep::~HsdkStep()
{
}

HemeraBuildConfiguration *HsdkStep::hemeraBuildConfiguration() const
{
    return static_cast<HemeraBuildConfiguration *>(buildConfiguration());
}

HemeraBuildConfiguration *HsdkStep::targetsActiveBuildConfiguration() const
{
    return static_cast<HemeraBuildConfiguration *>(target()->activeBuildConfiguration());
}

void HsdkStep::activeBuildConfigurationChanged()
{
    m_activeConfiguration = targetsActiveBuildConfiguration();
}

void HsdkStep::buildTargetsChanged()
{
    QStringList filteredTargets;
    foreach (const QString t, static_cast<HemeraProject *>(project())->buildTargetTitles()) {
        if (m_buildTargets.contains(t))
            filteredTargets.append(t);
    }
    setBuildTargets(filteredTargets);
}

void HsdkStep::setClean(bool clean)
{
    m_clean = clean;
}

QVariantMap HsdkStep::toMap() const
{
    QVariantMap map(AbstractProcessStep::toMap());
    map.insert(QLatin1String(Hemera::Constants::HSDK_STEP_CLEAN), m_clean);
    map.insert(QLatin1String(Hemera::Constants::HSDK_STEP_BUILD_TARGETS), m_buildTargets);
    map.insert(QLatin1String(Hemera::Constants::HSDK_STEP_ADDITIONAL_ARGS), m_additionalArguments);
    return map;
}

bool HsdkStep::fromMap(const QVariantMap &map)
{
    m_clean = map.value(QLatin1String(Hemera::Constants::HSDK_STEP_CLEAN)).toBool();
    m_buildTargets = map.value(QLatin1String(Hemera::Constants::HSDK_STEP_BUILD_TARGETS)).toStringList();
    m_additionalArguments = map.value(QLatin1String(Hemera::Constants::HSDK_STEP_ADDITIONAL_ARGS)).toStringList();

    return BuildStep::fromMap(map);
}


bool HsdkStep::init()
{
    HemeraBuildConfiguration *bc = hemeraBuildConfiguration();
    if (!bc)
        bc = static_cast<HemeraBuildConfiguration *>(target()->activeBuildConfiguration());

    m_tasks.clear();
    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(target()->kit());
    if (!tc) {
        m_tasks.append(ProjectExplorer::Task(ProjectExplorer::Task::Error,
                                             tr("Qt Creator needs a compiler set up to build. Configure a compiler in the kit options."),
                                             Utils::FileName(), -1,
                                             ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
        return true; // otherwise the tasks will not get reported
    }

    QString arguments = Utils::QtcProcess::joinArgs(m_buildTargets);
    Utils::QtcProcess::addArgs(&arguments, additionalArguments());

    setIgnoreReturnValue(m_clean);

    ProjectExplorer::ProcessParameters *pp = processParameters();
    pp->setMacroExpander(bc->macroExpander());
    Utils::Environment env = bc->environment();
    // Force output to english for the parsers. Do this here and not in the toolchain's
    // addToEnvironment() to not screw up the users run environment.
    env.set(QLatin1String("LC_ALL"), QLatin1String("C"));
    pp->setEnvironment(env);
    pp->setWorkingDirectory(bc->buildDirectory().toString());
    pp->setCommand(hsdkCommand(tc, bc->environment()));
    pp->setArguments(arguments);
    pp->resolveAll();

    /*setOutputParser(new HmrParser());
    ProjectExplorer::IOutputParser *parser = target()->kit()->createOutputParser();
    if (parser)
        appendOutputParser(parser);
    outputParser()->setWorkingDirectory(pp->effectiveWorkingDirectory());*/

    return AbstractProcessStep::init();
}

void HsdkStep::run(QFutureInterface<bool> &fi)
{
    bool canContinue = true;
    foreach (const ProjectExplorer::Task &t, m_tasks) {
        addTask(t);
        canContinue = false;
    }
    if (!canContinue) {
        emit addOutput(tr("Configuration is faulty. Check the Issues view for details."), BuildStep::MessageOutput);
        fi.reportResult(false);
        emit finished();
        return;
    }

    AbstractProcessStep::run(fi);
}

ProjectExplorer::BuildStepConfigWidget *HsdkStep::createConfigWidget()
{
    return new HsdkStepConfigWidget(this);
}

bool HsdkStep::immutable() const
{
    return false;
}

void HsdkStep::stdOutput(const QString &line)
{
    if (m_percentProgress.indexIn(line) != -1) {
        bool ok = false;
        int percent = m_percentProgress.cap(1).toInt(&ok);
        if (ok)
            futureInterface()->setProgressValue(percent);
    } else if (m_ninjaProgress.indexIn(line) != -1) {
        bool ok = false;
        int done = m_ninjaProgress.cap(1).toInt(&ok);
        if (ok) {
            int all = m_ninjaProgress.cap(2).toInt(&ok);
            if (ok && all != 0) {
                int percent = 100.0 * done/all;
                futureInterface()->setProgressValue(percent);
            }
        }
    }

    AbstractProcessStep::stdOutput(line);
}

QStringList HsdkStep::buildTargets() const
{
    return m_buildTargets;
}

bool HsdkStep::buildsBuildTarget(const QString &target) const
{
    return m_buildTargets.contains(target);
}

void HsdkStep::setBuildTarget(const QString &buildTarget, bool on)
{
    QStringList old = m_buildTargets;
    if (on && !old.contains(buildTarget))
        old << buildTarget;
    else if (!on && old.contains(buildTarget))
        old.removeOne(buildTarget);
    setBuildTargets(old);
}

void HsdkStep::setBuildTargets(const QStringList &targets)
{
    if (targets != m_buildTargets) {
        m_buildTargets = targets;
        emit targetsToBuildChanged();
    }
}

void HsdkStep::clearBuildTargets()
{
    m_buildTargets.clear();
}

QStringList HsdkStep::additionalArguments() const
{
    return m_additionalArguments;
}

void HsdkStep::setAdditionalArguments(const QStringList &list)
{
    m_additionalArguments = list;
}

QString HsdkStep::hsdkCommand(ProjectExplorer::ToolChain *tc, const Utils::Environment &env) const
{
    Q_UNUSED(env)

    if (tc->type() != QLatin1String(Constants::TOOLCHAIN_TYPE)) {
        qWarning() << "HsdkStep::hsdkCommand: wrong toolchain type" << tc->type();
        return QString();
    }

    if (!tc->isValid()) {
        qWarning() << "HsdkStep::hsdkCommand: invalid toolchain";
        return QString();
    }

    // Actually, it's always the same.
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String("HemeraSettings"));
    QString hsdkExecutable = settings->value(QLatin1String("hsdkExecutable")).toString();
    settings->endGroup();

    return hsdkExecutable;
}

//
// HsdkStepConfigWidget
//

HsdkStepConfigWidget::HsdkStepConfigWidget(HsdkStep *hsdkStep)
    : m_hsdkStep(hsdkStep)
{
    QFormLayout *fl = new QFormLayout(this);
    fl->setMargin(0);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    setLayout(fl);

    m_additionalArguments = new QLineEdit(this);
    fl->addRow(tr("Additional arguments:"), m_additionalArguments);
    m_additionalArguments->setText(m_hsdkStep->additionalArguments().join(QLatin1Char(' ')));

    m_buildTargetsList = new QListWidget;
    m_buildTargetsList->setMinimumHeight(200);
    fl->addRow(tr("Targets:"), m_buildTargetsList);

    // TODO update this list also on rescans of the HemeraLists.txt
    HemeraProject *pro = static_cast<HemeraProject *>(m_hsdkStep->project());
    QStringList targetList = pro->buildTargetTitles();
    targetList.sort();
    foreach (const QString &buildTarget, targetList) {
        QListWidgetItem *item = new QListWidgetItem(buildTarget, m_buildTargetsList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(m_hsdkStep->buildsBuildTarget(item->text()) ? Qt::Checked : Qt::Unchecked);
    }

    updateDetails();

    connect(m_additionalArguments, SIGNAL(textEdited(QString)), this, SLOT(additionalArgumentsEdited()));
    connect(m_buildTargetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    connect(ProjectExplorer::ProjectExplorerPlugin::instance(), SIGNAL(settingsChanged()),
            this, SLOT(updateDetails()));

    connect(pro, SIGNAL(buildTargetsChanged()), this, SLOT(buildTargetsChanged()));
    connect(m_hsdkStep, SIGNAL(targetsToBuildChanged()), this, SLOT(selectedBuildTargetsChanged()));
    connect(pro, SIGNAL(environmentChanged()), this, SLOT(updateDetails()));
    connect(m_hsdkStep, SIGNAL(hsdkCommandChanged()), this, SLOT(updateDetails()));
}

void HsdkStepConfigWidget::additionalArgumentsEdited()
{
    m_hsdkStep->setAdditionalArguments(m_additionalArguments->text().split(QLatin1Char(' ')));
    updateDetails();
}

void HsdkStepConfigWidget::itemChanged(QListWidgetItem *item)
{
    m_hsdkStep->setBuildTarget(item->text(), item->checkState() & Qt::Checked);
    updateDetails();
}

QString HsdkStepConfigWidget::displayName() const
{
    return tr("Hsdk", "Hemera::HsdkStepConfigWidget display name.");
}

void HsdkStepConfigWidget::buildTargetsChanged()
{
    disconnect(m_buildTargetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    m_buildTargetsList->clear();
    HemeraProject *pro = static_cast<HemeraProject *>(m_hsdkStep->target()->project());
    foreach (const QString& buildTarget, pro->buildTargetTitles()) {
        QListWidgetItem *item = new QListWidgetItem(buildTarget, m_buildTargetsList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(m_hsdkStep->buildsBuildTarget(item->text()) ? Qt::Checked : Qt::Unchecked);
    }
    connect(m_buildTargetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    updateSummary();
}

void HsdkStepConfigWidget::selectedBuildTargetsChanged()
{
    disconnect(m_buildTargetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    for (int y = 0; y < m_buildTargetsList->count(); ++y) {
        QListWidgetItem *item = m_buildTargetsList->itemAt(0, y);
        item->setCheckState(m_hsdkStep->buildsBuildTarget(item->text()) ? Qt::Checked : Qt::Unchecked);
    }
    connect(m_buildTargetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
    updateSummary();
}

void HsdkStepConfigWidget::updateDetails()
{
    ProjectExplorer::BuildConfiguration *bc = m_hsdkStep->buildConfiguration();
    if (!bc)
        bc = m_hsdkStep->target()->activeBuildConfiguration();
    if (!bc) {
        m_summaryText = tr("<b>No build configuration found on this kit.</b>");
        updateSummary();
        return;
    }

    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(m_hsdkStep->target()->kit());
    if (tc) {
        QString arguments = Utils::QtcProcess::joinArgs(m_hsdkStep->buildTargets());
        Utils::QtcProcess::addArgs(&arguments, m_hsdkStep->additionalArguments());

        ProjectExplorer::ProcessParameters param;
        param.setMacroExpander(bc->macroExpander());
        param.setEnvironment(bc->environment());
        param.setWorkingDirectory(bc->buildDirectory().toString());
        param.setCommand(m_hsdkStep->hsdkCommand(tc, bc->environment()));
        param.setArguments(arguments);
        m_summaryText = param.summary(displayName());
    } else {
        m_summaryText = QLatin1String("<b>") + ProjectExplorer::ToolChainKitInformation::msgNoToolChainInTarget() + QLatin1String("</b>");
    }
    emit updateSummary();
}

QString HsdkStepConfigWidget::summaryText() const
{
    return m_summaryText;
}

//
// HsdkStepFactory
//

HsdkStepFactory::HsdkStepFactory(QObject *parent) :
    ProjectExplorer::IBuildStepFactory(parent)
{
}

HsdkStepFactory::~HsdkStepFactory()
{
}

bool HsdkStepFactory::canCreate(ProjectExplorer::BuildStepList *parent, const Core::Id id) const
{
    if (parent->target()->project()->id() == Constants::PROJECT_ID)
        return id == Hemera::Constants::HSDK_STEP_ID;
    return false;
}

ProjectExplorer::BuildStep *HsdkStepFactory::create(ProjectExplorer::BuildStepList *parent, const Core::Id id)
{
    if (!canCreate(parent, id))
        return 0;
    HsdkStep *step = new HsdkStep(parent);
    if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN) {
        step->setClean(true);
        step->setAdditionalArguments(QStringList() << QLatin1String("wipe"));
    }
    return step;
}

bool HsdkStepFactory::canClone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source) const
{
    return canCreate(parent, source->id());
}

ProjectExplorer::BuildStep *HsdkStepFactory::clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source)
{
    if (!canClone(parent, source))
        return 0;
    return new HsdkStep(parent, static_cast<HsdkStep *>(source));
}

bool HsdkStepFactory::canRestore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map) const
{
    return canCreate(parent, ProjectExplorer::idFromMap(map));
}

ProjectExplorer::BuildStep *HsdkStepFactory::restore(ProjectExplorer::BuildStepList *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    HsdkStep *bs(new HsdkStep(parent));
    if (bs->fromMap(map))
        return bs;
    delete bs;
    return 0;
}

QList<Core::Id> HsdkStepFactory::availableCreationIds(ProjectExplorer::BuildStepList *parent) const
{
    if (parent->target()->project()->id() == Constants::PROJECT_ID)
        return QList<Core::Id>() << Core::Id(Hemera::Constants::HSDK_STEP_ID);
    return QList<Core::Id>();
}

QString HsdkStepFactory::displayNameForId(const Core::Id id) const
{
    if (id == Hemera::Constants::HSDK_STEP_ID)
        return tr("Hsdk", "Display name for Hemera::HsdkStep id.");
    return QString();
}

void HsdkStep::processStarted()
{
    futureInterface()->setProgressRange(0, 100);
    AbstractProcessStep::processStarted();
}

void HsdkStep::processFinished(int exitCode, QProcess::ExitStatus status)
{
    AbstractProcessStep::processFinished(exitCode, status);
    futureInterface()->setProgressValue(100);
}

} // namespace Internal
} // namespace Hemera
