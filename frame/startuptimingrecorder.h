// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QObject>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QMutex>
#include <QVariantMap>
#include <QVector>
#include <QString>

DS_BEGIN_NAMESPACE

struct TimingSample
{
    QString phase;
    QString pluginId;
    qint64 elapsedMs = 0;
    QVariantMap metadata;
};

class DS_SHARE StartupTimingRecorder : public QObject
{
    Q_OBJECT
public:
    StartupTimingRecorder();
    ~StartupTimingRecorder() override;
    static StartupTimingRecorder *instance();

    void record(const QString &phase, qint64 elapsedMs,
                const QString &pluginId = {}, const QVariantMap &metadata = {});
    QJsonArray toJson() const;
    bool isEnabled() const;
    void dumpToFile(const QString &path) const;

private:
    Q_DISABLE_COPY(StartupTimingRecorder)

    QVector<TimingSample> m_samples;
    mutable QMutex m_mutex;
};

class StartupTimingScope
{
public:
    explicit StartupTimingScope(const QString &phase, const QString &pluginId = {},
                                const QVariantMap &metadata = {}, int timeoutMs = 0);
    ~StartupTimingScope();

private:
    QString m_phase;
    QString m_pluginId;
    QVariantMap m_metadata;
    int m_timeoutMs = 0;
    bool m_enabled = false;
    QElapsedTimer m_timer;
};

DS_END_NAMESPACE
