// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearanceapplet.h"

#include "pluginfactory.h"

#include <QDBusError>
#include <QDebug>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>

DCORE_USE_NAMESPACE
DS_BEGIN_NAMESPACE
namespace dde {

AppearanceApplet::AppearanceApplet(QObject *parent)
    : DApplet(parent)
{
    auto watcher = new QDBusServiceWatcher(this);
    watcher->addWatchedService("org.deepin.dde.Appearance1");
    watcher->setConnection(QDBusConnection::sessionBus());
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, [this] (const QString & service) {
        Q_UNUSED(service)
        initDBusProxy();
    });
}

AppearanceApplet::~AppearanceApplet()
{

}

bool AppearanceApplet::load()
{
    initDBusProxy();
    return DApplet::load();
}

qreal AppearanceApplet::opacity() const
{
    return m_opacity;
}

void AppearanceApplet::initDBusProxy()
{
    // Skip while the appearance service has not registered yet; the service
    // watcher above retries via initDBusProxy() once it appears, so the
    // proxy is only created when the call can succeed (an invalid proxy
    // here would just be the startup race and is retried anyway).
    // sessionBus().interface() is null when the session bus itself is
    // unavailable; defer in that case too instead of dereferencing null.
    auto *busInterface = QDBusConnection::sessionBus().interface();
    if (!busInterface || !busInterface->isServiceRegistered(QStringLiteral("org.deepin.dde.Appearance1"))) {
        qDebug() << "Appearance service not registered yet, deferring dbus proxy.";
        return;
    }

    qDebug() << "Init appearance dbus proxy.";
    m_interface.reset(new org::deepin::dde::Appearance1("org.deepin.dde.Appearance1",
                                                        "/org/deepin/dde/Appearance1",
                                                        QDBusConnection::sessionBus(),
                                                        this));
    if (!m_interface->isValid()) {
        qWarning() << "Failed to proxy Appearance, error:" << m_interface->lastError();
        m_interface.reset();
        updateOpacity(-1);
        return;
    }

    QObject::connect(m_interface.data(), &org::deepin::dde::Appearance1::OpacityChanged,
                     this, &AppearanceApplet::updateOpacity);
    updateOpacity(m_interface->opacity());
}

void AppearanceApplet::updateOpacity(qreal opacity)
{
    if (opacity >= 0) {
        // The minimum opacity is 0.2
        opacity = std::max(0.2, opacity);
    }
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    Q_EMIT opacityChanged();
}

D_APPLET_CLASS(AppearanceApplet)
}
DS_END_NAMESPACE

#include "appearanceapplet.moc"
