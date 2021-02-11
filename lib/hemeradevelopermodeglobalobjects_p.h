#ifndef HEMERA_DEVELOPERMODE_GLOBALOBJECTS_H
#define HEMERA_DEVELOPERMODE_GLOBALOBJECTS_H

#include <QtCore/QObject>

class QNetworkAccessManager;
namespace Hemera {
namespace DeveloperMode {

class GlobalObjects : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalObjects)

public:
    static GlobalObjects * const instance();

    virtual ~GlobalObjects();

    QNetworkAccessManager *networkAccessManager();

private:
    GlobalObjects();

    class Private;
    Private * const d;
};

}
}

#endif // HEMERA_DEVELOPERMODE_GLOBALOBJECTS_H
