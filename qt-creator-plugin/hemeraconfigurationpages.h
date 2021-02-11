#ifndef HEMERA_INTERNAL_HEMERACONFIGURATIONPAGES_H
#define HEMERA_INTERNAL_HEMERACONFIGURATIONPAGES_H

#include <QtWidgets/QWizardPage>
#include <QtWidgets/QStyledItemDelegate>

#include <QtCore/QItemSelectionModel>

#include <hemeradevelopermodedevice.h>

class QStandardItemModel;

namespace Hemera {
namespace DeveloperMode {
class HyperDiscoveryClient;
}
namespace Internal {

namespace Ui {
class HemeraDeviceAssociateWidget;
class HemeraDeviceSetupWidget;
class HemeraEmulatorSetupWidget;
class GenericProgressWidget;
}

enum DeviceItemData {
    HardwareIdRole = Qt::UserRole + 3,
    GenericNameRole = Qt::UserRole + 4,
    DeviceTypeRole = Qt::UserRole + 5,
    DeviceIconRole = Qt::UserRole + 6,
    IsDeviceDiscoveredRole = Qt::UserRole + 7,
    DeviceURL = Qt::UserRole + 8
};

enum EmulatorInstallMode {
    EmulatorInstallUnknownMode = 0,
    EmulatorInstallFromLocalVDI = 1,
    EmulatorInstallFromStart,
    EmulatorInstallConvertExistingVM
};

class HemeraDeviceConfigureWizardPage : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QItemSelectionModel* selectionModel READ selectionModel)

public:
    explicit HemeraDeviceConfigureWizardPage(QWidget *parent = 0);
    virtual ~HemeraDeviceConfigureWizardPage();

    virtual void initializePage() override final;
    virtual void cleanupPage() override final;
    virtual bool validatePage() override final;
    virtual bool isComplete() const override final;

    QItemSelectionModel *selectionModel() const;

private:
    void addDeviceById(const QString &id, const QString &name, DeveloperMode::Device::DeviceType deviceType, const QUrl &url, bool discovered);
    void removeDeviceById(const QString &id);

    Ui::HemeraDeviceSetupWidget *ui;

    Hemera::DeveloperMode::HyperDiscoveryClient *m_discoveryClient;

    QStandardItemModel *m_model;
};

class HemeraDeviceAssociateWizardPage : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QItemSelectionModel* selectionModel READ selectionModel)

public:
    explicit HemeraDeviceAssociateWizardPage(QWidget *parent = 0);
    virtual ~HemeraDeviceAssociateWizardPage();

    virtual void initializePage() override final;
    virtual void cleanupPage() override final;
    virtual bool isComplete() const override final;

    QItemSelectionModel *selectionModel() const;

private Q_SLOTS:
    void populateEmulatorModel();

private:
    Ui::HemeraDeviceAssociateWidget *ui;

    QStandardItemModel *m_model;
};

class HemeraDeviceCreateWizardPage : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)

public:
    explicit HemeraDeviceCreateWizardPage(const QString &hsdk, QWidget *parent = 0);
    virtual ~HemeraDeviceCreateWizardPage();

    virtual void initializePage() override final;
    virtual bool isComplete() const override final;

    QString deviceName() const;

protected:
    void setDeviceName(const QString &deviceName);

Q_SIGNALS:
    void deviceNameChanged();

private Q_SLOTS:
    void startProcessing();

private:
    void finished();
    void finishedWithError(const QString &error);

    Ui::GenericProgressWidget *ui;
    QString m_hsdk;

    QString m_deviceName;
    bool m_complete;
};

class HemeraEmulatorConfigureWizardPage : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(uint chosenMode MEMBER m_chosenMode)
    Q_PROPERTY(QString selectedId MEMBER m_selectedId)

public:
    explicit HemeraEmulatorConfigureWizardPage(QWidget *parent = 0);
    virtual ~HemeraEmulatorConfigureWizardPage();

    virtual void initializePage() override final;
    virtual void cleanupPage() override final;
    virtual bool validatePage() override final;
    virtual bool isComplete() const override final;

    QItemSelectionModel *existingVMSelectionModel() const;

private:
    void addExistingVM(const QString &name, const QString &id);

    Ui::HemeraEmulatorSetupWidget *ui;

    QStandardItemModel *m_existingVMModel;

    uint m_chosenMode;
    QString m_selectedId;
};

class HemeraEmulatorCreateWizardPage : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QString emulatorName READ emulatorName WRITE setEmulatorName NOTIFY emulatorNameChanged)

public:
    explicit HemeraEmulatorCreateWizardPage(const QString &hsdk, QWidget *parent = 0);
    virtual ~HemeraEmulatorCreateWizardPage();

    virtual void initializePage() override final;
    virtual bool isComplete() const override final;

    QString emulatorName() const;

protected:
    void setEmulatorName(const QString &emulatorName);

Q_SIGNALS:
    void emulatorNameChanged();

private Q_SLOTS:
    void startProcessing();

private:
    void finished();
    void finishedWithError(const QString &error);

    Ui::GenericProgressWidget *ui;
    QString m_hsdk;

    QString m_emulatorName;
    bool m_complete;
};

class DeviceItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DeviceItemDelegate(QObject *parent = 0);
    virtual ~DeviceItemDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override final;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override final;

public Q_SLOTS:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override final;

Q_SIGNALS:
    void repaintItem(QModelIndex);
};

} // namespace Internal
} // namespace Hemera

Q_DECLARE_METATYPE(QItemSelectionModel*)

#endif // HEMERA_INTERNAL_HEMERACONFIGURATIONPAGES_H
