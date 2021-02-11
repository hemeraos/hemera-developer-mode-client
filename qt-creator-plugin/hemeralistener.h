#ifndef HEMERA_INTERNAL_HEMERALISTENER_H
#define HEMERA_INTERNAL_HEMERALISTENER_H

#include <QtCore/QObject>

namespace Hemera {
namespace Internal {

class HemeraListener : public QObject
{
    Q_OBJECT
public:
    explicit HemeraListener(QObject *parent = 0);
    virtual ~HemeraListener();

signals:

public slots:
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERA_INTERNAL_HEMERALISTENER_H
