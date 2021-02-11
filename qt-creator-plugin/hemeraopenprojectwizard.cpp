#include "hemeraopenprojectwizard.h"
#include "hemeraprojectmanager.h"
#include "hemerabuildconfiguration.h"
#include "hemerabuildinfo.h"

#include <coreplugin/icore.h>
#include <utils/hostosinfo.h>
#include <utils/pathchooser.h>
#include <utils/fancylineedit.h>
#include <utils/historycompleter.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/abi.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <texteditor/fontsettings.h>
#include <remotelinux/remotelinux_constants.h>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QSettings>
#include <QStringList>
#include <QApplication>

namespace Hemera {
namespace Internal {

class GeneratorInfo
{
    Q_DECLARE_TR_FUNCTIONS(Hemera::Internal::GeneratorInfo)
public:
    enum Ninja { NoNinja, OfferNinja, ForceNinja };
    static QList<GeneratorInfo> generatorInfosFor(ProjectExplorer::Kit *k, Ninja n, bool preferNinja, bool hasCodeBlocks);

    GeneratorInfo();
    explicit GeneratorInfo(ProjectExplorer::Kit *kit, bool ninja = false);

    ProjectExplorer::Kit *kit() const;
    bool isNinja() const;

    QString displayName() const;
    QByteArray generatorArgument() const;
    QByteArray generator() const;

private:
    ProjectExplorer::Kit *m_kit;
    bool m_isNinja;
};

GeneratorInfo::GeneratorInfo()
    : m_kit(0), m_isNinja(false)
{}

GeneratorInfo::GeneratorInfo(ProjectExplorer::Kit *kit, bool ninja)
    : m_kit(kit), m_isNinja(ninja)
{}

ProjectExplorer::Kit *GeneratorInfo::kit() const
{
    return m_kit;
}

bool GeneratorInfo::isNinja() const {
    return m_isNinja;
}

QByteArray GeneratorInfo::generator() const
{
    if (!m_kit)
        return QByteArray();
    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(m_kit);
    ProjectExplorer::Abi targetAbi = tc->targetAbi();
    if (m_isNinja) {
        return "Ninja";
    } else if (targetAbi.os() == ProjectExplorer::Abi::WindowsOS) {
        if (targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2005Flavor
                || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2008Flavor
                || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2010Flavor
                || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2012Flavor) {
            return "NMake Makefiles";
        } else if (targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMSysFlavor) {
            if (Utils::HostOsInfo::isWindowsHost())
                return "MinGW Makefiles";
            else
                return "Unix Makefiles";
        }
    }
    return "Unix Makefiles";
}

QByteArray GeneratorInfo::generatorArgument() const
{
    QByteArray tmp = generator();
    if (tmp.isEmpty())
        return tmp;
    return QByteArray("-GCodeBlocks - ") + tmp;
}

QString GeneratorInfo::displayName() const
{
    if (!m_kit)
        return QString();
    if (m_isNinja)
        return tr("Ninja (%1)").arg(m_kit->displayName());
    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(m_kit);
    ProjectExplorer::Abi targetAbi = tc->targetAbi();
    if (targetAbi.os() == ProjectExplorer::Abi::WindowsOS) {
        if (targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2005Flavor
                || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2008Flavor
                || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2010Flavor
                || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2012Flavor) {
            return tr("NMake Generator (%1)").arg(m_kit->displayName());
        } else if (targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMSysFlavor) {
            if (Utils::HostOsInfo::isWindowsHost())
                return tr("MinGW Generator (%1)").arg(m_kit->displayName());
            else
                return tr("Unix Generator (%1)").arg(m_kit->displayName());
        }
    } else {
        // Non windows
        return tr("Unix Generator (%1)").arg(m_kit->displayName());
    }
    return QString();
}

QList<GeneratorInfo> GeneratorInfo::generatorInfosFor(ProjectExplorer::Kit *k, Ninja n, bool preferNinja, bool hasCodeBlocks)
{
    QList<GeneratorInfo> results;
    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(k);
    if (!tc)
        return results;
    Core::Id deviceType = ProjectExplorer::DeviceTypeKitInformation::deviceTypeId(k);
    if (deviceType !=  ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE
            && deviceType != RemoteLinux::Constants::GenericLinuxOsType)
        return results;
    ProjectExplorer::Abi targetAbi = tc->targetAbi();
    if (n != ForceNinja) {
        if (targetAbi.os() == ProjectExplorer::Abi::WindowsOS) {
            if (targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2005Flavor
                    || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2008Flavor
                    || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2010Flavor
                    || targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMsvc2012Flavor) {
                if (hasCodeBlocks)
                    results << GeneratorInfo(k);
            } else if (targetAbi.osFlavor() == ProjectExplorer::Abi::WindowsMSysFlavor) {
                results << GeneratorInfo(k);
            }
        } else {
            // Non windows
            results << GeneratorInfo(k);
        }
    }
    if (n != NoNinja) {
        if (preferNinja)
            results.prepend(GeneratorInfo(k, true));
        else
            results.append(GeneratorInfo(k, true));
    }
    return results;
}

//////////////
/// HemeraOpenProjectWizard
//////////////

HemeraOpenProjectWizard::HemeraOpenProjectWizard(HemeraProjectManager *hemeraManager, const QString &sourceDirectory, Utils::Environment env)
    : m_hemeraManager(hemeraManager),
      m_sourceDirectory(sourceDirectory),
      m_environment(env),
//      m_useNinja(false),
      m_kit(0)
{
//    if (!compatibleKitExist())
//        addPage(new NoKitPage(this));

    if (hasInSourceBuild()) {
        m_buildDirectory = m_sourceDirectory;
        addPage(new InSourceBuildPage(this));
    } else {
        m_buildDirectory = m_sourceDirectory + QLatin1String("-build");
        addPage(new ShadowBuildPage(this));
    }

//    if (!m_hemeraManager->isHemeraExecutableValid())
//        addPage(new ChooseHemeraPage(this));

//    addPage(new HemeraRunPage(this));

    init();
}

HemeraOpenProjectWizard::HemeraOpenProjectWizard(HemeraProjectManager *hemeraManager, HemeraOpenProjectWizard::Mode mode, const HemeraBuildInfo *info)
    : m_hemeraManager(hemeraManager)
    , m_sourceDirectory(info->sourceDirectory)
    , m_environment(info->environment)
//    , m_useNinja(info->useNinja)
    , m_kit(0)
{
    m_kit = ProjectExplorer::KitManager::find(info->kitId);

//    HemeraRunPage::Mode rmode;
//    if (mode == HemeraOpenProjectWizard::NeedToCreate)
//        rmode = HemeraRunPage::Recreate;
//    else if (mode == HemeraOpenProjectWizard::WantToUpdate)
//        rmode = HemeraRunPage::WantToUpdate;
//    else if (mode == HemeraOpenProjectWizard::NeedToUpdate)
//        rmode = HemeraRunPage::NeedToUpdate;
//    else
//        rmode = HemeraRunPage::ChangeDirectory;

    if (mode == HemeraOpenProjectWizard::ChangeDirectory) {
        m_buildDirectory = info->buildDirectory.toString();
        addPage(new ShadowBuildPage(this, true));
    }
//    if (!m_hemeraManager->isHemeraExecutableValid())
//        addPage(new ChooseHemeraPage(this));

//    addPage(new HemeraRunPage(this, rmode, info->buildDirectory.toString()));
    init();
}

void HemeraOpenProjectWizard::init()
{
    setWindowTitle(tr("Hemera Wizard"));
}

HemeraProjectManager *HemeraOpenProjectWizard::hemeraManager() const
{
    return m_hemeraManager;
}

bool HemeraOpenProjectWizard::hasInSourceBuild() const
{
    QFileInfo fi(m_sourceDirectory + QLatin1String("/HemeraCache.txt"));
    if (fi.exists())
        return true;
    return false;
}

bool HemeraOpenProjectWizard::compatibleKitExist() const
{
    return true;

//    bool hasCodeBlocksGenerator = m_hemeraManager->hasCodeBlocksMsvcGenerator();
//    bool hasNinjaGenerator = m_hemeraManager->hasCodeBlocksNinjaGenerator();
//    bool preferNinja = m_hemeraManager->preferNinja();

    // TODO: continue removing ninja references from here
    bool hasCodeBlocksGenerator = false;
    bool hasNinjaGenerator = false;
    bool preferNinja = false;

    QList<ProjectExplorer::Kit *> kitList = ProjectExplorer::KitManager::kits();

    foreach (ProjectExplorer::Kit *k, kitList) {
        // OfferNinja and ForceNinja differ in what they return
        // but not whether the list is empty or not, which is what we
        // are interested in here
        QList<GeneratorInfo> infos = GeneratorInfo::generatorInfosFor(k,
                                                                      hasNinjaGenerator ? GeneratorInfo::OfferNinja : GeneratorInfo::NoNinja,
                                                                      preferNinja,
                                                                      hasCodeBlocksGenerator);
        if (!infos.isEmpty())
            return true;
    }
    return false;
}

bool HemeraOpenProjectWizard::existsUpToDateXmlFile() const
{
    return false;
}

QString HemeraOpenProjectWizard::buildDirectory() const
{
    return m_buildDirectory;
}

QString HemeraOpenProjectWizard::sourceDirectory() const
{
    return m_sourceDirectory;
}

void HemeraOpenProjectWizard::setBuildDirectory(const QString &directory)
{
    m_buildDirectory = directory;
}

//bool HemeraOpenProjectWizard::useNinja() const
//{
//    return m_useNinja;
//}

//void HemeraOpenProjectWizard::setUseNinja(bool b)
//{
//    m_useNinja = b;
//}

QString HemeraOpenProjectWizard::arguments() const
{
    return m_arguments;
}

void HemeraOpenProjectWizard::setArguments(const QString &args)
{
    m_arguments = args;
}

Utils::Environment HemeraOpenProjectWizard::environment() const
{
    return m_environment;
}

ProjectExplorer::Kit *HemeraOpenProjectWizard::kit() const
{
    return m_kit;
}

void HemeraOpenProjectWizard::setKit(ProjectExplorer::Kit *kit)
{
    m_kit = kit;
}

//////
// NoKitPage
/////

NoKitPage::NoKitPage(HemeraOpenProjectWizard *hemeraWizard)
    : QWizardPage(hemeraWizard), m_hemeraWizard(hemeraWizard)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setWordWrap(true);
    layout->addWidget(m_descriptionLabel);

    m_optionsButton = new QPushButton;
    m_optionsButton->setText(tr("Show Options"));

    connect(m_optionsButton, SIGNAL(clicked()),
            this, SLOT(showOptions()));

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(m_optionsButton);
    hbox->addStretch();

    layout->addLayout(hbox);

    setTitle(tr("Check Kits"));

    connect(ProjectExplorer::KitManager::instance(), SIGNAL(kitsChanged()),
            this, SLOT(kitsChanged()));

    kitsChanged();
}

void NoKitPage::kitsChanged()
{
    if (isComplete()) {
        m_descriptionLabel->setText(tr("There are compatible kits."));
        m_optionsButton->setVisible(false);
    } else {
        m_descriptionLabel->setText(tr("Qt Creator has no kits that are suitable for Hemera projects. Please configure a kit."));
        m_optionsButton->setVisible(true);
    }
    emit completeChanged();
}

bool NoKitPage::isComplete() const
{
    return m_hemeraWizard->compatibleKitExist();
}

void NoKitPage::showOptions()
{
    Core::ICore::showOptionsDialog(Core::Id(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY),
                                   Core::Id(ProjectExplorer::Constants::KITS_SETTINGS_PAGE_ID), this);
}

InSourceBuildPage::InSourceBuildPage(HemeraOpenProjectWizard *hemeraWizard)
    : QWizardPage(hemeraWizard), m_hemeraWizard(hemeraWizard)
{
    setLayout(new QVBoxLayout);
    QLabel *label = new QLabel(this);
    label->setWordWrap(true);
    label->setText(tr("Qt Creator has detected an <b>in-source-build in %1</b> "
                   "which prevents shadow builds. Qt Creator will not allow you to change the build directory. "
                   "If you want a shadow build, clean your source directory and re-open the project.")
                   .arg(m_hemeraWizard->buildDirectory()));
    layout()->addWidget(label);
    setTitle(tr("Build Location"));
}

ShadowBuildPage::ShadowBuildPage(HemeraOpenProjectWizard *hemeraWizard, bool change)
    : QWizardPage(hemeraWizard), m_hemeraWizard(hemeraWizard)
{
    QFormLayout *fl = new QFormLayout;
    this->setLayout(fl);

    QLabel *label = new QLabel(this);
    label->setWordWrap(true);
    if (change)
        label->setText(tr("Please enter the directory in which you want to build your project.") + QLatin1Char(' '));
    else
        label->setText(tr("Please enter the directory in which you want to build your project. "
                          "Qt Creator recommends to not use the source directory for building. "
                          "This ensures that the source directory remains clean and enables multiple builds "
                          "with different settings."));
    fl->addRow(label);
    m_pc = new Utils::PathChooser(this);
    m_pc->setBaseDirectory(m_hemeraWizard->sourceDirectory());
    m_pc->setPath(m_hemeraWizard->buildDirectory());
    m_pc->setExpectedKind(Utils::PathChooser::Directory);
    connect(m_pc, SIGNAL(changed(QString)), this, SLOT(buildDirectoryChanged()));
    fl->addRow(tr("Build directory:"), m_pc);
    setTitle(tr("Build Location"));
}

void ShadowBuildPage::buildDirectoryChanged()
{
    m_hemeraWizard->setBuildDirectory(m_pc->path());
}

ChooseHemeraPage::ChooseHemeraPage(HemeraOpenProjectWizard *hemeraWizard)
    : QWizardPage(hemeraWizard), m_hemeraWizard(hemeraWizard)
{
    QFormLayout *fl = new QFormLayout;
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    setLayout(fl);

    m_hemeraLabel = new QLabel;
    m_hemeraLabel->setWordWrap(true);
    fl->addRow(m_hemeraLabel);
    // Show a field for the user to enter
    m_hemeraExecutable = new Utils::PathChooser(this);
    m_hemeraExecutable->setExpectedKind(Utils::PathChooser::ExistingCommand);
    fl->addRow(tr("Hemera Executable:"), m_hemeraExecutable);

    connect(m_hemeraExecutable, SIGNAL(editingFinished()),
            this, SLOT(hemeraExecutableChanged()));
    connect(m_hemeraExecutable, SIGNAL(browsingFinished()),
            this, SLOT(hemeraExecutableChanged()));

    setTitle(tr("Choose Hemera Executable"));
}

void ChooseHemeraPage::updateErrorText()
{
    QString hemeraExecutable = m_hemeraWizard->hemeraManager()->hsdkExecutable();
    if (m_hemeraWizard->hemeraManager()->isHsdkExecutableValid()) {
        m_hemeraLabel->setText(tr("The Hemera executable is valid."));
    } else {
        QString text = tr("Specify the path to the Hemera executable. No Hemera executable was found in the path.");
        if (!hemeraExecutable.isEmpty()) {
            text += QLatin1Char(' ');
            QFileInfo fi(hemeraExecutable);
            if (!fi.exists())
                text += tr("The Hemera executable (%1) does not exist.").arg(hemeraExecutable);
            else if (!fi.isExecutable())
                text += tr("The path %1 is not an executable.").arg(hemeraExecutable);
            else
                text += tr("The path %1 is not a valid Hemera executable.").arg(hemeraExecutable);
        }
        m_hemeraLabel->setText(text);
    }
}

void ChooseHemeraPage::hemeraExecutableChanged()
{
    m_hemeraWizard->hemeraManager()->setHsdkExecutable(m_hemeraExecutable->path());
    updateErrorText();
    emit completeChanged();
}

bool ChooseHemeraPage::isComplete() const
{
    return m_hemeraWizard->hemeraManager()->isHsdkExecutableValid();
}

HemeraRunPage::HemeraRunPage(HemeraOpenProjectWizard *hemeraWizard, Mode mode, const QString &buildDirectory)
    : QWizardPage(hemeraWizard),
      m_hemeraWizard(hemeraWizard),
      m_haveCbpFile(false),
      m_mode(mode),
      m_buildDirectory(buildDirectory)
{
    initWidgets();
}

void HemeraRunPage::initWidgets()
{
    QFormLayout *fl = new QFormLayout;
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    setLayout(fl);
    // Description Label
    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setWordWrap(true);

    fl->addRow(m_descriptionLabel);

    // Run Hemera Line (with arguments)
    m_argumentsLineEdit = new Utils::FancyLineEdit(this);
    m_argumentsLineEdit->setHistoryCompleter(QLatin1String("HemeraArgumentsLineEdit"));
    m_argumentsLineEdit->selectAll();

    connect(m_argumentsLineEdit,SIGNAL(returnPressed()), this, SLOT(runHemera()));
    fl->addRow(tr("Arguments:"), m_argumentsLineEdit);

    m_generatorComboBox = new QComboBox(this);
    fl->addRow(tr("Generator:"), m_generatorComboBox);

    m_runHemera = new QPushButton(this);
    m_runHemera->setText(tr("Run Hemera"));
    connect(m_runHemera, SIGNAL(clicked()), this, SLOT(runHemera()));

    QHBoxLayout *hbox2 = new QHBoxLayout;
    hbox2->addStretch(10);
    hbox2->addWidget(m_runHemera);
    fl->addRow(hbox2);

    // Bottom output window
    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    // set smaller minimum size to avoid vanishing descriptions if all of the
    // above is shown and the dialog not vertically resizing to fit stuff in (Mac)
    m_output->setMinimumHeight(15);
    QFont f(TextEditor::FontSettings::defaultFixedFontFamily());
    f.setStyleHint(QFont::TypeWriter);
    m_output->setFont(f);
    QSizePolicy pl = m_output->sizePolicy();
    pl.setVerticalStretch(1);
    m_output->setSizePolicy(pl);
    fl->addRow(m_output);

    m_exitCodeLabel = new QLabel(this);
    m_exitCodeLabel->setVisible(false);
    fl->addRow(m_exitCodeLabel);

    setTitle(tr("Run Hemera"));
    setMinimumSize(600, 400);
}

QByteArray HemeraRunPage::cachedGeneratorFromFile(const QString &cache)
{
    QFile fi(cache);
    if (fi.exists()) {
        // Cache exists, then read it...
        if (fi.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!fi.atEnd()) {
                QByteArray line = fi.readLine();
                if (line.startsWith("HEMERA_GENERATOR:INTERNAL=")) {
                    int splitpos = line.indexOf('=');
                    if (splitpos != -1) {
                        QByteArray cachedGenerator = line.mid(splitpos + 1).trimmed();
                        if (!cachedGenerator.isEmpty())
                            return cachedGenerator;
                    }
                }
            }
        }
    }
    return QByteArray();
}

void HemeraRunPage::initializePage()
{
    if (m_mode == Initial) {
        bool upToDateXmlFile = m_hemeraWizard->existsUpToDateXmlFile();
        m_buildDirectory = m_hemeraWizard->buildDirectory();

        if (upToDateXmlFile) {
            m_descriptionLabel->setText(
                    tr("The directory %1 already contains a cbp file, which is recent enough. "
                       "You can pass special arguments and rerun Hemera. "
                       "Or simply finish the wizard directly.").arg(m_buildDirectory));
            m_haveCbpFile = true;
        } else {
            m_descriptionLabel->setText(
                    tr("The directory %1 does not contain a cbp file. Qt Creator needs to create this file by running Hemera. "
                       "Some projects require command line arguments to the initial Hemera call.").arg(m_buildDirectory));
        }
    } else if (m_mode == HemeraRunPage::NeedToUpdate) {
        m_descriptionLabel->setText(tr("The directory %1 contains an outdated .cbp file. Qt "
                                       "Creator needs to update this file by running Hemera. "
                                       "If you want to add additional command line arguments, "
                                       "add them below. Note that Hemera remembers command "
                                       "line arguments from the previous runs.").arg(m_buildDirectory));
    } else if (m_mode == HemeraRunPage::Recreate) {
        m_descriptionLabel->setText(tr("The directory %1 specified in a build-configuration, "
                                       "does not contain a cbp file. Qt Creator needs to "
                                       "recreate this file, by running Hemera. "
                                       "Some projects require command line arguments to "
                                       "the initial Hemera call. Note that Hemera remembers command "
                                       "line arguments from the previous runs.").arg(m_buildDirectory));
    } else if (m_mode == HemeraRunPage::ChangeDirectory) {
        m_buildDirectory = m_hemeraWizard->buildDirectory();
        m_descriptionLabel->setText(tr("Qt Creator needs to run Hemera in the new build directory. "
                                       "Some projects require command line arguments to the "
                                       "initial Hemera call."));
    } else if (m_mode == HemeraRunPage::WantToUpdate) {
        m_descriptionLabel->setText(tr("Refreshing cbp file in %1.").arg(m_buildDirectory));
    }

    // Build the list of generators/toolchains we want to offer
    m_generatorComboBox->clear();

//    bool hasCodeBlocksGenerator = m_hemeraWizard->hemeraManager()->hasCodeBlocksMsvcGenerator();
//    bool hasNinjaGenerator = m_hemeraWizard->hemeraManager()->hasCodeBlocksNinjaGenerator();
//    bool preferNinja = m_hemeraWizard->hemeraManager()->preferNinja();
    // TODO: continue removing ninja references from here
    bool hasCodeBlocksGenerator = false;
    bool hasNinjaGenerator = false;
    bool preferNinja = false;

    if (m_mode == Initial) {
        // Try figuring out generator and toolchain from HemeraCache.txt
        QByteArray cachedGenerator = cachedGeneratorFromFile(m_buildDirectory + QLatin1String("/HemeraCache.txt"));

        m_generatorComboBox->show();
        QList<ProjectExplorer::Kit *> kitList = ProjectExplorer::KitManager::kits();
        int defaultIndex = 0;

        foreach (ProjectExplorer::Kit *k, kitList) {
            QList<GeneratorInfo> infos = GeneratorInfo::generatorInfosFor(k,
                                                                          hasNinjaGenerator ? GeneratorInfo::OfferNinja : GeneratorInfo::NoNinja,
                                                                          preferNinja,
                                                                          hasCodeBlocksGenerator);

            if (k == ProjectExplorer::KitManager::defaultKit())
                defaultIndex = m_generatorComboBox->count();

            foreach (const GeneratorInfo &info, infos)
                if (cachedGenerator.isEmpty() || info.generator() == cachedGenerator)
                    m_generatorComboBox->addItem(info.displayName(), qVariantFromValue(info));
        }

        m_generatorComboBox->setCurrentIndex(defaultIndex);
    } else {
        // Note: We don't compare the actually cached generator to what is set in the buildconfiguration
        // We assume that the buildconfiguration is correct
        GeneratorInfo::Ninja ninja;
        if (m_mode == HemeraRunPage::NeedToUpdate || m_mode == HemeraRunPage::WantToUpdate) {
//            ninja = m_hemeraWizard->useNinja() ? GeneratorInfo::ForceNinja : GeneratorInfo::NoNinja;
        } else { // Recreate, ChangeDirectory
            // Note: ReCreate is technically just a removed .cbp file, we assume the cache
            // got removed too. If the cache still exists the error message from hemera should
            // be a good hint to change the generator
            ninja = hasNinjaGenerator ? GeneratorInfo::OfferNinja : GeneratorInfo::NoNinja;
        }

        QList<GeneratorInfo> infos = GeneratorInfo::generatorInfosFor(m_hemeraWizard->kit(),
                                                                      ninja,
                                                                      preferNinja,
                                                                      true);
        foreach (const GeneratorInfo &info, infos)
            m_generatorComboBox->addItem(info.displayName(), qVariantFromValue(info));
    }
}

bool HemeraRunPage::validatePage()
{
    int index = m_generatorComboBox->currentIndex();
    if (index == -1)
        return false;
    GeneratorInfo generatorInfo = m_generatorComboBox->itemData(index).value<GeneratorInfo>();
    m_hemeraWizard->setKit(generatorInfo.kit());
    return QWizardPage::validatePage();
}

void HemeraRunPage::runHemera()
{
    m_haveCbpFile = false;

    Utils::Environment env = m_hemeraWizard->environment();
    int index = m_generatorComboBox->currentIndex();

    if (index == -1) {
        m_output->appendPlainText(tr("No generator selected."));
        return;
    }
    GeneratorInfo generatorInfo = m_generatorComboBox->itemData(index).value<GeneratorInfo>();
    m_hemeraWizard->setKit(generatorInfo.kit());

    // If mode is initial the user chooses the kit, otherwise it's already choosen
    // and the environment already contains the kit
    if (m_mode == Initial)
        generatorInfo.kit()->addToEnvironment(env);

    m_runHemera->setEnabled(false);
    m_argumentsLineEdit->setEnabled(false);
    m_generatorComboBox->setEnabled(false);

    m_output->clear();

    HemeraProjectManager *hemeraManager = m_hemeraWizard->hemeraManager();
    if (m_hemeraWizard->hemeraManager()->isHsdkExecutableValid()) {
        m_hemeraProcess = new Utils::QtcProcess();
        connect(m_hemeraProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(hemeraReadyReadStandardOutput()));
        connect(m_hemeraProcess, SIGNAL(readyReadStandardError()), this, SLOT(hemeraReadyReadStandardError()));
        connect(m_hemeraProcess, SIGNAL(finished(int)), this, SLOT(hemeraFinished()));
    } else {
        m_runHemera->setEnabled(true);
        m_argumentsLineEdit->setEnabled(true);
        m_generatorComboBox->setEnabled(true);
        m_output->appendPlainText(tr("No valid Hemera executable specified."));
    }
}

static QColor mix_colors(QColor a, QColor b)
{
    return QColor((a.red() + 2 * b.red()) / 3, (a.green() + 2 * b.green()) / 3,
                  (a.blue() + 2* b.blue()) / 3, (a.alpha() + 2 * b.alpha()) / 3);
}

void HemeraRunPage::hemeraReadyReadStandardOutput()
{
    QTextCursor cursor(m_output->document());
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat tf;

    QFont font = m_output->font();
    tf.setFont(font);
    tf.setForeground(m_output->palette().color(QPalette::Text));

    cursor.insertText(QString::fromLocal8Bit(m_hemeraProcess->readAllStandardOutput()), tf);
}

void HemeraRunPage::hemeraReadyReadStandardError()
{
    QTextCursor cursor(m_output->document());
    QTextCharFormat tf;

    QFont font = m_output->font();
    QFont boldFont = font;
    boldFont.setBold(true);
    tf.setFont(boldFont);
    tf.setForeground(mix_colors(m_output->palette().color(QPalette::Text), QColor(Qt::red)));

    cursor.insertText(QString::fromLocal8Bit(m_hemeraProcess->readAllStandardError()), tf);
}

void HemeraRunPage::hemeraFinished()
{
    m_runHemera->setEnabled(true);
    m_argumentsLineEdit->setEnabled(true);
    m_generatorComboBox->setEnabled(true);

    if (m_hemeraProcess->exitCode() != 0) {
        m_exitCodeLabel->setVisible(true);
        m_exitCodeLabel->setText(tr("Hemera exited with errors. Please check Hemera output."));
        static_cast<Utils::HistoryCompleter *>(m_argumentsLineEdit->completer())->removeHistoryItem(0);
        m_haveCbpFile = false;
    } else {
        m_exitCodeLabel->setVisible(false);
        m_haveCbpFile = true;
    }
    m_hemeraProcess->deleteLater();
    m_hemeraProcess = 0;
    m_hemeraWizard->setArguments(m_argumentsLineEdit->text());
    emit completeChanged();
}

void HemeraRunPage::cleanupPage()
{
    m_output->clear();
    m_haveCbpFile = false;
    m_exitCodeLabel->setVisible(false);
    emit completeChanged();
}

bool HemeraRunPage::isComplete() const
{
    int index = m_generatorComboBox->currentIndex();
    return index != -1 && m_haveCbpFile;
}

} // namespace Internal
} // namespace Hemera

Q_DECLARE_METATYPE(Hemera::Internal::GeneratorInfo)
