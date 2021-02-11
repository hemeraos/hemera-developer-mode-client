#include "hemeradevelopermodehamanager.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QPointer>
#include <QCoreApplication>

#include <QtQml/QQmlComponent>
#include <QtQml/QQmlEngine>

#include <hemeradevelopermodedevice.h>

// Relevant QML classes
#include <3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlapplication.h>
#include <3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsimplecppapplication.h>
#include <3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsimpleqmlapplication.h>
#include <3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlsimpleqtquick1application.h>
#include <3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlpythonapplication.h>
#include <3rdparty/hemera-qt5-sdk/plugins/qml/settings/hemeraqmlproxiedapplication.h>

namespace Hemera {
namespace DeveloperMode {

class HaManager::Private
{
public:
    Private(HaManager *q) : q(q), valid(false), ready(false), type(Type::Unknown) {}

    HaManager * const q;

    bool valid;
    bool ready;

    QString haFile;
    QQmlEngine *engine;

    QFileSystemWatcher *watcher;

    // Properties
    QString applicationId;
    QString applicationName;
    QString applicationVersion;
    Type type;
    QStringList sourceFiles;
    QStringList resourceFiles;
    QStringList formFiles;
    QStringList qrcFiles;

    void setReady(bool newready);
    void setValid(bool newvalid);

    void reloadHaFile();

    void parseHaFile(QObject *haObject);

    QString normalizeFilePath(const QString &file);
    QStringList normalizeFilesPath(const QStringList &files);
};

void HaManager::Private::setReady(bool newready)
{
    if (newready == ready) {
        return;
    }

    ready = newready;
    Q_EMIT q->readyChanged(ready);

    // If we ain't ready, clean the cache.
    if (!ready) {
        resourceFiles.clear();
        sourceFiles.clear();
        type = Type::Unknown;
    }
}

void HaManager::Private::setValid(bool newvalid)
{
    if (newvalid == valid) {
        return;
    }

    valid = newvalid;
    Q_EMIT q->validityChanged(valid);

    if (!valid) {
        // It's definitely not ready anymore!
        setReady(false);
    }
}

void HaManager::Private::reloadHaFile()
{
    setReady(false);

    // We need to clear the engine's internal cache before we proceed.
    engine->clearComponentCache();

    QQmlComponent *haComponent = new QQmlComponent(engine, QUrl::fromLocalFile(haFile), QQmlComponent::Asynchronous, q);

    auto onComponentStatusChanged = [this, haComponent] (QQmlComponent::Status status) {
        if (status == QQmlComponent::Error) {
            Q_EMIT q->qmlErrors(haComponent->errors());
            haComponent->deleteLater();

            // Invalid then
            setValid(false);
        } else if (status == QQmlComponent::Ready) {
            QObject *haObject = haComponent->create();

            if (!haObject) {
                // TODO: Highly unlikely, but just in case - how do we handle this error?
                haComponent->deleteLater();
                setValid(false);
                return;
            }

            // Yeah!
            setValid(true);

            // Parse it now
            parseHaFile(haObject);

            // Good. Clear!
            haComponent->deleteLater();
        }
    };

    if (haComponent->isLoading()) {
        QObject::connect(haComponent, &QQmlComponent::statusChanged, onComponentStatusChanged);
    } else {
        onComponentStatusChanged(haComponent->status());
    }
}

void HaManager::Private::parseHaFile(QObject *haObject)
{
    // The easy stuff.
    applicationId = haObject->property("applicationId").toString();
    applicationName = haObject->property("name").toString();
    applicationVersion = haObject->property("version").toString();
    sourceFiles = haObject->property("sourceFiles").toStringList();
    resourceFiles = haObject->property("resourceFiles").toStringList();

    const char *className = haObject->metaObject()->className();

    // Detect the type now.
    if (!qstrcmp(className, Hemera::Qml::Settings::SimpleCppApplication::staticMetaObject.className())) {
        type = Type::CxxApplication;
    } else if (!qstrcmp(className, Hemera::Qml::Settings::SimpleQmlApplication::staticMetaObject.className()) ||
               !qstrcmp(className, Hemera::Qml::Settings::SimpleQtQuick1Application::staticMetaObject.className())) {
        type = Type::QmlApplication;
    } else if (!qstrcmp(className, Hemera::Qml::Settings::PythonApplication::staticMetaObject.className())) {
        type = Type::PythonApplication;
    } else if (!qstrcmp(className, Hemera::Qml::Settings::ProxiedApplication::staticMetaObject.className())) {
        type = Type::ProxiedApplication;
    } else if (!qstrcmp(className, Hemera::Qml::Settings::Application::staticMetaObject.className())) {
        type = Type::NoBuildsystem;
    } else {
        type = Type::Unknown;
    }

    setReady(true);
}

QString HaManager::variableNameForCategory(HaManager::FileCategory category)
{
    switch (category) {
        case FileCategory::SourceFiles:
            return QStringLiteral("sourceFiles");
        case FileCategory::ResourceFiles:
            return QStringLiteral("resourceFiles");
        case FileCategory::FormFiles:
            return QStringLiteral("formFiles");
        case FileCategory::QrcFiles:
            return QStringLiteral("qrcFiles");
    }

    return QString();
}

QString HaManager::Private::normalizeFilePath(const QString& file)
{
    QDir projectDirectory = QFileInfo(haFile).dir();

    // Return relative path
    return projectDirectory.relativeFilePath(file);
}

QStringList HaManager::Private::normalizeFilesPath(const QStringList& files)
{
    QStringList normalized;
    for (const QString &file : files) {
        normalized << normalizeFilePath(file);
    }
    return normalized;
}



HaManager::HaManager(const QString &pathToHa, QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    // Good. First things first, let's find out about our file.
    if (!QFile::exists(pathToHa)) {
        // Bye.
        return;
    }

    d->haFile = pathToHa;

    // Declarative powers
    d->engine = new QQmlEngine(this);
    QList< QQmlError > errors;
    d->engine->setOutputWarningsToStandardError(false);
    QDir preparePathToPlugin(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN)
    QString pathToPlugin = QDir::cleanPath(preparePathToPlugin.absoluteFilePath(QDir::toNativeSeparators(QStringLiteral("../lib/qtcreator/hemera/qml/com/ispirata/Hemera/Settings/libHemeraQmlSettings.dll"))));
    d->engine->importPlugin(pathToPlugin, QStringLiteral("com.ispirata.Hemera.Settings"), NULL);
#else
    QString pathToPlugin = QDir::cleanPath(preparePathToPlugin.absoluteFilePath(QDir::toNativeSeparators(QStringLiteral("../lib/qtcreator/hemera/qml"))));
    d->engine->addImportPath(pathToPlugin);
#endif

    // Forward signal
    QObject::connect(d->engine, &QQmlEngine::warnings, this, &HaManager::qmlWarnings);

    // Monitor
    d->watcher = new QFileSystemWatcher(QStringList() << d->haFile, this);
    connect(d->watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        // Does the file still exist?
        Q_EMIT fileChanged();
        if (!QFile::exists(d->haFile)) {
            // Ouch. Looks like it has been deleted...
            d->setValid(false);
        } else {
            // It has been modified. Reload!
            d->setReady(false);
            d->reloadHaFile();
        }
    }, Qt::QueuedConnection);

    // Very good! Now we're ready.
    d->reloadHaFile();
}

HaManager::~HaManager()
{
    delete d;
}

QStringList HaManager::fileList(HaManager::FileCategory category)
{
    switch (category) {
        case FileCategory::SourceFiles:
            return d->sourceFiles;
        case FileCategory::ResourceFiles:
            return d->resourceFiles;
        case FileCategory::FormFiles:
            return d->formFiles;
        case FileCategory::QrcFiles:
            return d->qrcFiles;
        default:
            return QStringList();
    }
    return QStringList();
}

bool HaManager::addFile(const QString& sourceFile, HaManager::FileCategory category)
{
    return addFiles(QStringList() << sourceFile, category);
}

bool HaManager::addFiles(const QStringList& sourceFiles, HaManager::FileCategory category)
{
    QStringList files = fileList(category);
    files << d->normalizeFilesPath(sourceFiles);
    return setFiles(files, category);
}

bool HaManager::removeFile(const QString& sourceFile, HaManager::FileCategory category)
{
    return removeFiles(QStringList() << sourceFile, category);
}

bool HaManager::removeFiles(const QStringList& sourceFiles, HaManager::FileCategory category)
{
    QStringList files = fileList(category);
    QStringList toRemove = d->normalizeFilesPath(sourceFiles);

    for (const QString &tbr : toRemove) {
        if (!files.contains(tbr)) {
            return false;
        }
        files.removeOne(tbr);
    }

    return setFiles(files, category);
}

bool HaManager::renameFile(const QString& oldFile, const QString& newFile, HaManager::FileCategory category)
{
    QStringList files = fileList(category);
    QString oldFileNormalized = d->normalizeFilePath(oldFile);
    QString newFileNormalized = d->normalizeFilePath(newFile);
    if (!files.contains(oldFileNormalized)) {
        return false;
    }
    files.replace(files.indexOf(oldFileNormalized), newFileNormalized);

    return setFiles(files, category);
}

bool HaManager::setFiles(const QStringList& files, HaManager::FileCategory category)
{
    QStringList normalizedFiles = d->normalizeFilesPath(files);

    // Ok, we need to get our favorite regexp now.
    QString variable = variableNameForCategory(category);
    QString prefix;
    QRegExp matchRegexpSpace = QRegExp(QStringLiteral(" %1\\s*\\:\\s*\\[[^\\]]*\\]").arg(variable));
    QRegExp matchRegexpNewline = QRegExp(QStringLiteral("\n%1\\s*\\:\\s*\\[[^\\]]*\\]").arg(variable));
    QRegExp replacementRegexp;
    QString content;

    // Do we have a match?
    {
        QFile haFile(d->haFile);
        if (!haFile.open(QFile::ReadOnly | QFile::Text)) {
            return false;
        }
        content = QString::fromLatin1(haFile.readAll());
        if (matchRegexpSpace.indexIn(content) <= 0) {
            if (matchRegexpNewline.indexIn(content) <= 0) {
                qDebug() << "Could not match";
                return false;
            }
            prefix = QStringLiteral("\n    ");
            replacementRegexp = matchRegexpNewline;
        } else {
            prefix = QStringLiteral(" ");
            replacementRegexp = matchRegexpSpace;
        }

        QString newList;
        if (!files.isEmpty()) {
            // Now, recreate the list.
            newList = QStringLiteral("%1%2: [\n").arg(prefix, variable);
            for (const QString &file : files) {
                newList += QStringLiteral("        \"%1\",\n").arg(file);
            }

            // Remove last 2 chars (,\n)
            newList.chop(2);

            // Close the list
            newList += QLatin1Char(']');
        }

        // Replace then.
        content.replace(replacementRegexp, newList);
        qDebug() << "Match," << content;

        haFile.close();
    }

    // Good. let's write!
    QFile haFile(d->haFile);
    if (!haFile.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        return false;
    }

    if (haFile.write(content.toLatin1()) <= 0) {
        return false;
    }

    // Yeah!
    haFile.flush();
    haFile.close();

    // Done (no refresh, QFileSystemWatcher takes care of this for us)
    return true;
}


QString HaManager::filePath() const
{
    return d->haFile;
}

bool HaManager::isValid() const
{
    return d->valid;
}

bool HaManager::isReady() const
{
    return d->ready;
}

bool HaManager::isBuildsystem() const
{
    return d->type != Type::NoBuildsystem && d->type != Type::ProxiedApplication && d->type != Type::Unknown;
}

bool HaManager::isCompiledApplication() const
{
    return d->type == Type::CxxApplication;
}

HaManager::Type HaManager::type() const
{
    return d->type;
}

QString HaManager::applicationId() const
{
    return d->applicationId;
}

QString HaManager::applicationName() const
{
    return d->applicationName;
}

QString HaManager::applicationVersion() const
{
    return d->applicationVersion;
}

QString HaManager::packageNameFor(const Target::Ptr& target) const
{
    // Package name has this format:
    // ha-<id_with_no_dots>-<version>-1.i586.rpm
    QString idWithNoDots = d->applicationId;
    idWithNoDots.remove('.');
    QString architecture = QStringLiteral("i586");
    Device::Ptr device = target.objectCast<Device>();
    if (!device.isNull()) {
        architecture = device->architecture();
    }

    return QStringLiteral("ha-%1-%2-1.%3.rpm").arg(idWithNoDots, d->applicationVersion, architecture);
}

QString HaManager::typeToString(HaManager::Type type)
{
    switch (type) {
        case Type::CxxApplication:
            return tr("C++ Application");
        case Type::NoBuildsystem:
            return tr("Plain ha, no buildsystem");
        case Type::ProxiedApplication:
            return tr("Proxied Application");
        case Type::PythonApplication:
            return tr("Python Application");
        case Type::QmlApplication:
            return tr("Qml Application");
        default:
            return tr("Unknown ha type");
    }

    // unreachable
    return QString();
}

}
}
