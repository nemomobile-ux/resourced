/*
 * Copyright (C) 2026 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CLIENTADAPTOR_H
#define CLIENTADAPTOR_H

#include "core/resourcemanager.h"
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QObject>
#include <QStringList>
#include <qdbusvirtualobject.h>

class ClientAdaptor : public QDBusVirtualObject {
    Q_OBJECT
public:
    explicit ClientAdaptor(ResourceClient* parent);
    ~ClientAdaptor() override;
    QString introspect(const QString& path) const override;
    bool handleMessage(const QDBusMessage& message, const QDBusConnection& connection) override;

private:
    void printDebug(const QDBusMessage& message);
};

#endif // CLIENTADAPTOR_H
