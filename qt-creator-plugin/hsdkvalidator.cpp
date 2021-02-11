#include "hsdkvalidator.h"

#include <QProcess>
#include <QFileInfo>
#include <QTextDocument>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Hemera {
namespace Internal {

///////////////////////////
// HemeraValidator
///////////////////////////
HsdkValidator::HsdkValidator()
    : m_state(Invalid)
    , m_process(0)
{
}

HsdkValidator::~HsdkValidator()
{
    cancel();
}

void HsdkValidator::cancel()
{
    if (m_process) {
        disconnect(m_process, SIGNAL(finished(int)));
        m_process->waitForFinished();

        delete m_process;
        m_process = 0;
    }
}

void HsdkValidator::finished(int exitCode)
{
    if (exitCode) {
        m_state = HsdkValidator::Invalid;
        return;
    }

    if (m_state == HsdkValidator::Running) {
        QJsonParseError error;
        QJsonObject info = QJsonDocument::fromJson(m_process->readAll(), &error).object();

        if (error.error != QJsonParseError::NoError) {
            m_state = HsdkValidator::Invalid;
            return;
        }

        m_supportedHemeraSDKs = [info] { QStringList r; for (const QVariant &v : info.value(QStringLiteral("supportedHemeraSDKs")).toArray().toVariantList()) r.append(v.toString()); return r; }();
        m_version = info.value(QStringLiteral("version")).toString();
        m_executablePath = info.value(QStringLiteral("executablePath")).toString();

        if (m_supportedHemeraSDKs.isEmpty() || m_version.isEmpty() || m_executablePath.isEmpty()) {
            m_state = HsdkValidator::Invalid;
        } else {
            m_state = HsdkValidator::Done;
        }
    }
}

QStringList HsdkValidator::supportedHemeraSDKs() const
{
    return m_supportedHemeraSDKs;
}

QString HsdkValidator::hsdkVersion() const
{
    return m_version;
}

QString HsdkValidator::hsdkExecutable() const
{
    return m_executable;
}

bool HsdkValidator::isValid() const
{
    if (m_state == HsdkValidator::Invalid) {
        return false;
    }

    if (m_state == HsdkValidator::Running) {
        m_process->waitForFinished();
    }

    return (m_state != HsdkValidator::Invalid);
}

void HsdkValidator::setHsdkExecutable(const QString &executable)
{
    cancel();
    m_process = new QProcess();
    connect(m_process, SIGNAL(finished(int)),
            this, SLOT(finished(int)));

    m_executable = executable;
    QFileInfo fi(m_executable);
    if (fi.exists() && fi.isExecutable()) {
        // Run it to find out more
        m_state = Running;
        if (!startProcess(QStringList(QLatin1String("--dump"))))
            m_state = Invalid;
    } else {
        m_state = Invalid;
    }
}

bool HsdkValidator::startProcess(const QStringList &args)
{
    m_process->start(m_executable, args);
    return m_process->waitForStarted(2000);
}

} // namespace Internal
} // namespace Hemera
