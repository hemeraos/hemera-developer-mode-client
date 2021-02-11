#ifndef HSDKVALIDATOR_H
#define HSDKVALIDATOR_H

#include <QtCore/QObject>

#include <texteditor/codeassist/keywordscompletionassist.h>

QT_FORWARD_DECLARE_CLASS(QProcess)

namespace Hemera {
namespace Internal {

class HsdkValidator : public QObject
{
    Q_OBJECT

public:
    HsdkValidator();
    virtual ~HsdkValidator();

    void cancel();
    bool isValid() const;

    void setHsdkExecutable(const QString &executable);
    QString hsdkExecutable() const;

    QString hsdkVersion() const;
    QStringList supportedHemeraSDKs() const;

private:
    enum State {
        Invalid,
        Running,
        Done
    };

    bool startProcess(const QStringList &args);

    State m_state;
    QProcess *m_process;
    QString m_executable;

    QStringList m_supportedHemeraSDKs;
    QString m_version;
    QString m_executablePath;

private slots:
    void finished(int exitCode);
};

} // namespace Internal
} // namespace Hemera

#endif // HSDKVALIDATOR_H
