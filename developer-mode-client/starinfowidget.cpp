#include "starinfowidget.h"

#include "ui_starinfowidget.h"

#include <hemeradevelopermodestar.h>

#include "mainwindow.h"

StarInfoWidget::StarInfoWidget(Hemera::DeveloperMode::Star *handler, QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::StarInfoWidget)
    , m_star(handler)
{
    ui->setupUi(this);

    // Connect to our object
    connect(handler, &Hemera::DeveloperMode::Star::phaseChanged, this, &StarInfoWidget::populateData);
    connect(handler, &Hemera::DeveloperMode::Star::activeOrbitChanged, this, &StarInfoWidget::populateData);
    connect(handler, &Hemera::DeveloperMode::Star::validityChanged, this, &StarInfoWidget::populateData);
}

StarInfoWidget::~StarInfoWidget()
{

}

void StarInfoWidget::populateData()
{
    switch (m_star->display()) {
        case Hemera::DeveloperMode::Star::DisplayType::X11:
            ui->typeLabel->setText(tr("X11 (X.org) Display Server"));
            break;
        case Hemera::DeveloperMode::Star::DisplayType::EGLFS:
            ui->typeLabel->setText(tr("EGL Full Screen (framebuffer)"));
            break;
        case Hemera::DeveloperMode::Star::DisplayType::Wayland:
            ui->typeLabel->setText(tr("Wayland Compositor"));
            break;
        case Hemera::DeveloperMode::Star::DisplayType::LinuxFB:
            ui->typeLabel->setText(tr("Linux direct framebuffer"));
            break;
        case Hemera::DeveloperMode::Star::DisplayType::DirectFB:
            ui->typeLabel->setText(tr("DirectFB framebuffer"));
            break;
        case Hemera::DeveloperMode::Star::DisplayType::Headless:
            ui->typeLabel->setText(tr("Headless"));
            break;
        default:
            break;
    }
    switch (m_star->phase()) {
        case Hemera::DeveloperMode::Star::Phase::Nebula:
            ui->statusLabel->setText(tr("Nebula"));
            break;
        case Hemera::DeveloperMode::Star::Phase::MainSequence:
            ui->statusLabel->setText(tr("Main Sequence"));
            break;
        case Hemera::DeveloperMode::Star::Phase::Injected:
            ui->statusLabel->setText(tr("Orbit Injected (Developer mode likely active)"));
            break;
        case Hemera::DeveloperMode::Star::Phase::Collapse:
            ui->statusLabel->setText(tr("Collapsed"));
            break;
        case Hemera::DeveloperMode::Star::Phase::Unknown:
        default:
            ui->statusLabel->setText(tr("Unknown status"));
            break;
    }
    if (!m_star->isValid()) {
        ui->statusLabel->setText(tr("Offline or not reachable"));
    }
    setTitle(m_star->name());
    ui->activeOrbitLabel->setText(m_star->activeOrbit());
    if (m_star->residentOrbit().isEmpty()) {
        ui->residentOrbitLabel->setText(tr("Not available"));
    } else {
        ui->residentOrbitLabel->setText(m_star->residentOrbit());
    }
    if (m_star->isInhibitionActive()) {
        ui->inhibitionLabel->setText(tr("Active, triggered by applications: %1").arg(
                                        QStringList(m_star->inhibitionReasons().keys()).join(QLatin1Char(','))));
    } else {
        ui->inhibitionLabel->setText(tr("Not active"));
    }
    ui->parametersLabel->setText(m_star->properties());
}
