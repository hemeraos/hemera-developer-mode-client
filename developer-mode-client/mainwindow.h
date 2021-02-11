#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QPointer>

#include <QtWidgets/QMainWindow>

#include <hemeradevelopermodecontroller.h>

// Useful macros...
#define HEMERA_DEV_MODE_URL "/com.ispirata.Hemera.DeveloperMode/"
#define TARGETS_MANAGER Hemera::DeveloperMode::TargetManager *tm = Hemera::DeveloperMode::TargetManager::instance();

#define CRAFT_DEVMODE_REQUEST(THEURL) if (m_currentTarget.isNull()) {\
    return;\
}\
QNetworkRequest request(m_currentTarget->urlForRelativeTarget(HEMERA_DEV_MODE_URL + THEURL));\
request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");\
QVariantMap data

#define PARSE_JSON_OBJECT(THEDATA) QJsonParseError error;\
QJsonDocument document = QJsonDocument::fromJson(THEDATA, &error);\
if (error.error != QJsonParseError::NoError) {\
    qDebug() << "error parse1";\
    return;\
}\
if (!document.isObject()) {\
    qDebug() << "error parse2";\
    return;\
}\
QJsonObject jsonObject = document.object()

// class ConfigureDevices;
// class QTimer;
class StarInfoWidget;
class QNetworkAccessManager;
class QStandardItemModel;

namespace Hemera {
namespace DeveloperMode {
class Controller;
class Target;
};
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void on_aboutAction_triggered();
    void on_configureTargetsButton_clicked();
    void on_targetsComboBox_currentIndexChanged(const QString &text);
    void onTargetChanged();
    void onAdvancedModeToggled(bool checked);
    void onStarBoxIndexChanged();
    void updateApplicationComboBoxList();
    void aboutQt();
    void quit();

private:
    void updateTargetDetails();
    void updateTargetsList();
    void updateDeveloperModeDetails();
    void developerModeStatusChanged(const QString &handler, Hemera::DeveloperMode::Controller::Status status);
    void saveSettings();

    Ui::MainWindow *ui;
    Hemera::DeveloperMode::Target::Ptr m_currentTarget;
    QStandardItemModel *m_applicationsModel;
    QStandardItemModel *m_featuresModel;

    QHash< QString, StarInfoWidget* > m_starInfoWidgets;
    QStringList m_activeOrbits;

    QList< QMetaObject::Connection > m_starConnections;
};

#endif // MAINWINDOW_H
