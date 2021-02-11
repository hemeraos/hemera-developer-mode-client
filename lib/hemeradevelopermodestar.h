#ifndef HEMERA_DEVELOPERMODE_STAR_H
#define HEMERA_DEVELOPERMODE_STAR_H

#include <QtCore/QObject>

#include "hemeradevelopermodeexport.h"

#include "hemeradevelopermodetarget.h"

namespace Hemera {
namespace DeveloperMode {

class Controller;

class Target;

class HemeraDeveloperModeClient_EXPORT Star : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Star)

    Q_PRIVATE_SLOT(d, void refreshInfo())
    Q_PRIVATE_SLOT(d, void onOnlineChanged(bool))

    Q_PROPERTY(bool             valid               READ isValid                  NOTIFY validityChanged       STORED true)

    Q_PROPERTY(QString          name                READ name                                                  STORED true)
    Q_PROPERTY(Hemera::DeveloperMode::Star::DisplayType display READ display                           STORED true)
    Q_PROPERTY(QString          activeOrbit         READ activeOrbit              NOTIFY activeOrbitChanged    STORED true)
    Q_PROPERTY(QString          residentOrbit       READ residentOrbit                                         STORED true)
    Q_PROPERTY(Hemera::DeveloperMode::Star::Phase phase READ phase                NOTIFY phaseChanged          STORED true)

    Q_PROPERTY(bool             inhibitionActive    READ isInhibitionActive       NOTIFY inhibitionChanged     STORED true)
    Q_PROPERTY(QVariantMap      inhibitionReasons   READ inhibitionReasons        NOTIFY inhibitionChanged     STORED true)
    Q_PROPERTY(QString          properties          READ properties                                            STORED true)

public:
    enum class Phase {
        Unknown = 0,
        Nebula = 1,
        MainSequence = 2,
        Injected = 3,
        Collapse = 99
    };
    Q_ENUMS(Phase)
    enum class DisplayType {
        Unknown = 0,
        X11 = 1,
        EGLFS = 2,
        Wayland = 3,
        LinuxFB = 4,
        DirectFB = 5,
        Headless = 99
    };
    Q_ENUMS(DisplayType)

    virtual ~Star();

    bool isValid();
    Target::Ptr target();

    QString name() const;
    DisplayType display() const;
    QString activeOrbit() const;
    QString residentOrbit() const;
    Phase phase() const;

    bool isInhibitionActive() const;
    QVariantMap inhibitionReasons() const;
    QString properties() const;

    bool waitForValid(int timeout = 5000);

Q_SIGNALS:
    void validityChanged(bool valid);
    void phaseChanged(Hemera::DeveloperMode::Star::Phase status);
    void activeOrbitChanged(const QString &activeOrbit);
    void inhibitionChanged();

private:
    explicit Star(const Target::Ptr &parent, const QString &star);

    class Private;
    Private * const d;

    friend class Target;
};
}
}

Q_DECLARE_METATYPE(Hemera::DeveloperMode::Star::Phase)
Q_DECLARE_METATYPE(Hemera::DeveloperMode::Star::DisplayType)

#endif // HEMERA_DEVELOPERMODE_STAR_H
