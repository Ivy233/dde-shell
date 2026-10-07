// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifysetting.h"

#include <DConfig>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QLoggingCategory>

namespace notification {

Q_DECLARE_LOGGING_CATEGORY(notifyLog)

NotifySetting * NotifySetting::instance()
{
    static NotifySetting instance;
    return &instance;
}

int NotifySetting::bubbleCount() const
{
    return m_bubbleCount;
}

int NotifySetting::contentRowCount() const
{
    return m_contentRowCount;
}

NotifySetting::NotifySetting(QObject *parent)
    : QObject(parent)
{
    m_setting = Dtk::Core::DConfig::create("org.deepin.dde.shell", "org.deepin.dde.shell.notification");
    connect(m_setting, &Dtk::Core::DConfig::valueChanged, this, [this](const QString &key) {
        if (key == "bubbleCount") {
            updateMaxBubbleCount();
        } else if (key == "bubbleContentRowCount") {
            updateBubbleContentRowCount();
        }
    });

    // dde-dconfig-daemon may not be ready when the notification server loads
    // (startup race); refetch the settings once it registers.
    auto configWatcher = new QDBusServiceWatcher(QStringLiteral("org.desktopspec.ConfigManager"),
                                                 QDBusConnection::systemBus(),
                                                 QDBusServiceWatcher::WatchForRegistration, this);
    connect(configWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString &) {
        qCDebug(notifyLog) << "ConfigManager registered, refetching bubble settings";
        updateMaxBubbleCount();
        updateBubbleContentRowCount();
    });

    updateMaxBubbleCount();
    updateBubbleContentRowCount();
}

NotifySetting::~NotifySetting()
{
    if (m_setting)
        m_setting->deleteLater();
}

void NotifySetting::updateMaxBubbleCount()
{
    const auto value = m_setting->value("bubbleCount");
    if (!value.isValid()) {
        // Backend not ready (dde-dconfig-daemon startup race) or key unknown;
        // the service watcher above refetches once the daemon registers.
        return;
    }

    auto count = value.toInt();
    if (count < 1) {
        qWarning() << "Invalid settings of bubbleCount:" << count << ", it should be greater than 0";
        return;
    }

    if (m_bubbleCount != count) {
        m_bubbleCount = count;
        Q_EMIT bubbleCountChanged(count);
    }
}

void NotifySetting::updateBubbleContentRowCount()
{
    const auto value = m_setting->value("bubbleContentRowCount");
    if (!value.isValid()) {
        return;
    }

    auto count = value.toInt();
    if (count < 1) {
        qWarning() << "Invalid settings of bubbleContentRowCount:" << count << ", it should be greater than 0";
        return;
    }

    if (m_contentRowCount != count) {
        m_contentRowCount = count;
        Q_EMIT contentRowCountChanged(count);
    }
}
}
