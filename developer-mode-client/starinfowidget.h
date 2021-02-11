#ifndef STARINFOWIDGET_H
#define STARINFOWIDGET_H

#include <QtWidgets/QGroupBox>

#include <QtCore/QUrl>

namespace Hemera {
namespace DeveloperMode {
class Star;
}
}
namespace Ui {
class StarInfoWidget;
}

class QNetworkAccessManager;

class StarInfoWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit StarInfoWidget(Hemera::DeveloperMode::Star *handler, QWidget *parent = 0);
    virtual ~StarInfoWidget();

private Q_SLOTS:
    void populateData();

private:
    Ui::StarInfoWidget *ui;

    Hemera::DeveloperMode::Star *m_star;
};

#endif // STARINFOWIDGET_H
