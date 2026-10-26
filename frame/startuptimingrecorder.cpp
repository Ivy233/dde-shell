// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "startuptimingrecorder.h"

#include <dobject_p.h>
#include <QApplicationStatic>

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QMutexLocker>

DS_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(dsBenchmarkLog, "org.deepin.dde.shell.benchmark")

Q_APPLICATION_STATIC(StartupTimingRecorder, g_instance)

StartupTimingRecorder::StartupTimingRecorder()
    : QObject()
{
}

StartupTimingRecorder::~StartupTimingRecorder() = default;

StartupTimingRecorder *StartupTimingRecorder::instance()
{
    return g_instance;
}

void StartupTimingRecorder::record(const QString &phase, qint64 elapsedMs,
                                   const QString &pluginId, const QVariantMap &metadata)
{
    QMutexLocker locker(&m_mutex);
    m_samples.append({phase, pluginId, elapsedMs, metadata});
}

QJsonArray StartupTimingRecorder::toJson() const
{
    QMutexLocker locker(&m_mutex);
    QJsonArray array;
    for (const auto &sample : m_samples) {
        QJsonObject obj;
        obj["phase"] = sample.phase;
        if (!sample.pluginId.isEmpty())
            obj["plugin_id"] = sample.pluginId;
        obj["elapsed_ms"] = sample.elapsedMs;
        if (!sample.metadata.isEmpty())
            obj["metadata"] = QJsonObject::fromVariantMap(sample.metadata);
        array.append(obj);
    }
    return array;
}

bool StartupTimingRecorder::isEnabled() const
{
    return qgetenv("DDE_SHELL_BENCHMARK") == "1";
}

void StartupTimingRecorder::dumpToFile(const QString &path) const
{
    QJsonObject root;
    root["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    root["version"] = "1.0";
    root["pid"] = QCoreApplication::applicationPid();
    root["samples"] = toJson();

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
    } else {
        qCWarning(dsBenchmarkLog) << "Cannot open benchmark output:" << path;
    }
}

StartupTimingScope::StartupTimingScope(const QString &phase, const QString &pluginId,
                                       const QVariantMap &metadata, int timeoutMs)
    : m_phase(phase)
    , m_pluginId(pluginId)
    , m_metadata(metadata)
    , m_timeoutMs(timeoutMs)
{
    if (StartupTimingRecorder::instance()->isEnabled()) {
        m_enabled = true;
        m_timer.start();
    }
}

StartupTimingScope::~StartupTimingScope()
{
    if (!m_enabled)
        return;

    const qint64 elapsed = m_timer.elapsed();
    StartupTimingRecorder::instance()->record(m_phase, elapsed, m_pluginId, m_metadata);

    if (m_timeoutMs > 0 && elapsed >= m_timeoutMs) {
        if (m_pluginId.isEmpty())
            qCWarning(dsBenchmarkLog) << m_phase << ": elapsed time [" << elapsed << "ms ].";
        else
            qCWarning(dsBenchmarkLog) << m_phase << ":" << m_pluginId << ": elapsed time [" << elapsed << "ms ].";
    }
}

DS_END_NAMESPACE
