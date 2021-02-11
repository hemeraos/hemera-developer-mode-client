#ifndef HEMERA_DEVELOPERMODE_HAMANAGER_H
#define HEMERA_DEVELOPERMODE_HAMANAGER_H

#include <QtCore/QObject>

#include <QtQml/QQmlError>

#include <hemeradevelopermodetarget.h>

#include "hemeradevelopermodeexport.h"

namespace Hemera {
namespace DeveloperMode {

class HemeraDeveloperModeClient_EXPORT HaManager : public QObject
{
    Q_OBJECT

public:
    enum class Type {
        Unknown,
        CxxApplication,
        PythonApplication,
        QmlApplication,
        ProxiedApplication,
        NoBuildsystem
    };

    enum class FileCategory {
        Unknown = 0,
        SourceFiles,
        ResourceFiles,
        FormFiles,
        QrcFiles
    };

    explicit HaManager(const QString &pathToHa, QObject *parent = nullptr);
    virtual ~HaManager();

    QString filePath() const;

    bool isValid() const;
    bool isReady() const;

    bool isBuildsystem() const;
    bool isCompiledApplication() const;

    Type type() const;

    QString applicationId() const;
    QString applicationName() const;
    QString applicationVersion() const;

    static QString typeToString(Type type);

    QString packageNameFor(const Target::Ptr &target) const;

    QStringList fileList(FileCategory category);

    bool addFile(const QString &sourceFile, FileCategory category);
    bool addFiles(const QStringList &sourceFiles, FileCategory category);
    bool renameFile(const QString &oldFile, const QString &newFile, FileCategory category);
    bool removeFile(const QString &sourceFile, FileCategory category);
    bool removeFiles(const QStringList &sourceFiles, FileCategory category);
    bool setFiles(const QStringList &files, FileCategory category);

    static QString variableNameForCategory(HaManager::FileCategory category);

Q_SIGNALS:
    void fileChanged();

    void qmlWarnings(const QList<QQmlError> &warnings);
    void qmlErrors(const QList<QQmlError> &warnings);

    void validityChanged(bool valid);
    void readyChanged(bool ready);

private:
    class Private;
    Private * const d;
};
}
}

#endif // HEMERA_DEVELOPERMODE_HAMANAGER_H
