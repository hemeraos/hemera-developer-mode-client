#ifndef HEMERAOPENPROJECTWIZARD_H
#define HEMERAOPENPROJECTWIZARD_H

#include "hemerabuildconfiguration.h"
#include "hemerabuildinfo.h"

#include <utils/environment.h>
#include <utils/wizard.h>
#include <utils/qtcprocess.h>
#include <projectexplorer/target.h>
#include <projectexplorer/project.h>

#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPlainTextEdit>

namespace Utils {
class FancyLineEdit;
class PathChooser;
}

namespace ProjectExplorer {
class Kit;
}

namespace Hemera {
namespace Internal {

class HemeraProjectManager;

class HemeraOpenProjectWizard : public Utils::Wizard
{
    Q_OBJECT
public:
    enum Mode {
        Nothing,
        NeedToCreate,
        NeedToUpdate,
        WantToUpdate,
        ChangeDirectory
    };

    /// used at importing a project without a .user file
    HemeraOpenProjectWizard(HemeraProjectManager *hemeraManager, const QString &sourceDirectory, Utils::Environment env);

    /// used to update if we have already a .user file
    /// recreates or updates the cbp file
    /// Also used to change the build directory of one buildconfiguration or create a new buildconfiguration
    HemeraOpenProjectWizard(HemeraProjectManager *hemeraManager, Mode mode, const HemeraBuildInfo *info);

    QString buildDirectory() const;
    QString sourceDirectory() const;
    void setBuildDirectory(const QString &directory);
    HemeraProjectManager *hemeraManager() const;
    QString arguments() const;
    void setArguments(const QString &args);
    Utils::Environment environment() const;
    ProjectExplorer::Kit *kit() const;
    void setKit(ProjectExplorer::Kit *kit);
    bool existsUpToDateXmlFile() const;
    bool compatibleKitExist() const;

private:
    void init();
    bool hasInSourceBuild() const;
    HemeraProjectManager *m_hemeraManager;
    QString m_buildDirectory;
    QString m_sourceDirectory;
    QString m_arguments;
    Utils::Environment m_environment;
    ProjectExplorer::Kit *m_kit;
};

class NoKitPage : public QWizardPage
{
    Q_OBJECT
public:
    NoKitPage(HemeraOpenProjectWizard *hemeraWizard);
    bool isComplete() const;
private slots:
    void kitsChanged();
    void showOptions();
private:
    QLabel *m_descriptionLabel;
    QPushButton *m_optionsButton;
    HemeraOpenProjectWizard *m_hemeraWizard;
};

class InSourceBuildPage : public QWizardPage
{
    Q_OBJECT
public:
    InSourceBuildPage(HemeraOpenProjectWizard *hemeraWizard);
private:
    HemeraOpenProjectWizard *m_hemeraWizard;
};

class ShadowBuildPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ShadowBuildPage(HemeraOpenProjectWizard *hemeraWizard, bool change = false);
private slots:
    void buildDirectoryChanged();
private:
    HemeraOpenProjectWizard *m_hemeraWizard;
    Utils::PathChooser *m_pc;
};

class ChooseHemeraPage : public QWizardPage
{
    Q_OBJECT
public:
    ChooseHemeraPage(HemeraOpenProjectWizard *hemeraWizard);

    virtual bool isComplete() const;
public slots:
    void hemeraExecutableChanged();
private:
    void updateErrorText();
    QLabel *m_hemeraLabel;
    HemeraOpenProjectWizard *m_hemeraWizard;
    Utils::PathChooser *m_hemeraExecutable;
};

class HemeraRunPage : public QWizardPage
{
    Q_OBJECT
public:
    enum Mode { Initial, NeedToUpdate, Recreate, ChangeDirectory, WantToUpdate };
    explicit HemeraRunPage(HemeraOpenProjectWizard *hemeraWizard, Mode mode = Initial, const QString &buildDirectory = QString());

    virtual void initializePage();
    virtual bool validatePage();
    virtual void cleanupPage();
    virtual bool isComplete() const;
private slots:
    void runHemera();
    void hemeraFinished();
    void hemeraReadyReadStandardOutput();
    void hemeraReadyReadStandardError();
private:
    void initWidgets();
    QByteArray cachedGeneratorFromFile(const QString &cache);
    HemeraOpenProjectWizard *m_hemeraWizard;
    QPlainTextEdit *m_output;
    QPushButton *m_runHemera;
    Utils::QtcProcess *m_hemeraProcess;
    Utils::FancyLineEdit *m_argumentsLineEdit;
    QComboBox *m_generatorComboBox;
    QLabel *m_descriptionLabel;
    QLabel *m_exitCodeLabel;
    bool m_haveCbpFile;
    Mode m_mode;
    QString m_buildDirectory;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERAOPENPROJECTWIZARD_H
