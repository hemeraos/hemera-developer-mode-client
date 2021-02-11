#ifndef HYPERSPACEPROCESS_H
#define HYPERSPACEPROCESS_H

#include <projectexplorer/devicesupport/deviceprocess.h>

namespace Hemera {
namespace Internal {

class HyperspaceProcess : public ProjectExplorer::DeviceProcess
{
    Q_OBJECT
public:
    explicit HyperspaceProcess(const QSharedPointer<const ProjectExplorer::IDevice> &device, QObject *parent = nullptr);
    virtual ~HyperspaceProcess();

    virtual void start(const QString &executable, const QStringList &arguments = QStringList()) override;
    virtual void interrupt() override;
    virtual void terminate() override;
    virtual void kill() override;

    virtual QProcess::ProcessState state() const override;
    virtual QProcess::ExitStatus exitStatus() const override;
    virtual int exitCode() const override;
    virtual QString errorString() const override;

    virtual Utils::Environment environment() const override;
    virtual void setEnvironment(const Utils::Environment &env) override;

    virtual void setWorkingDirectory(const QString &workingDirectory) override;

    virtual QByteArray readAllStandardOutput() override;
    virtual QByteArray readAllStandardError() override;

    virtual qint64 write(const QByteArray &data) override;

private Q_SLOTS:
    void handleProcessStateChanged(QProcess::ProcessState newState);
    void handleProcessError(QProcess::ProcessError error);
    void handleStdout(const QByteArray &message);
    void handleStderr(const QByteArray &message);

private:
    class Private;
    Private * const d;
};

}
}

#endif // HYPERSPACEPROCESS_H
